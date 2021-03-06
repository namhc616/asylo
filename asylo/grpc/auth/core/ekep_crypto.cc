/*
 *
 * Copyright 2017 Asylo authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "asylo/grpc/auth/core/ekep_crypto.h"

#include <openssl/curve25519.h>
#include <openssl/digest.h>
#include <openssl/hkdf.h>
#include <openssl/hmac.h>
#include <openssl/mem.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include <cstdint>
#include <memory>

#include "absl/strings/str_cat.h"
#include "asylo/crypto/util/bssl_util.h"
#include "asylo/crypto/util/byte_container_view.h"
#include "asylo/util/logging.h"
#include "asylo/grpc/auth/core/ekep_error_space.h"
#include "asylo/util/status.h"

namespace asylo {
namespace {

constexpr size_t kEkepSecretSize =
    (kEkepMasterSecretSize + kEkepAuthenticatorSecretSize);

constexpr char kEkepHkdfSalt[] = "EKEP Handshake v1";
constexpr char kEkepHkdfSaltRecordProtocol[] = "EKEP Record Protocol v1";
constexpr char kServerAuthenticatedText[] = "EKEP Handshake v1: Server Finish";
constexpr char kClientAuthenticatedText[] = "EKEP Handshake v1: Client Finish";

// Computes the HMAC of |authenticated_text| using the given |key|. The HMAC
// function is initialized using the hash function from |ciphersuite|. On
// success, writes the message authentication code to |mac|.
//
// If the ciphersuite is unsupported, returns BAD_HANDSHAKE_CIPHER.
Status Hmac(const HandshakeCipher &ciphersuite, ByteContainerView key,
            ByteContainerView authenticated_text,
            CleansingVector<uint8_t> *mac) {
  mac->clear();
  const EVP_MD *digest = nullptr;
  switch (ciphersuite) {
    case CURVE25519_SHA256:
      digest = EVP_sha256();
      mac->resize(SHA256_DIGEST_LENGTH);
      break;
    default:
      return Status(
          Abort::BAD_HANDSHAKE_CIPHER,
          "Ciphersuite not supported: " + HandshakeCipher_Name(ciphersuite));
  }

  unsigned int mac_size = mac->size();
  if (!HMAC(digest, key.data(), key.size(), authenticated_text.data(),
            authenticated_text.size(), mac->data(), &mac_size)) {
    LOG(ERROR) << "HMAC failed: " << BsslLastErrorString();
    return Status(Abort::INTERNAL_ERROR, "Internal error");
  }
  return Status::OkStatus();
}

}  // namespace

Status DeriveSecrets(const HandshakeCipher &ciphersuite,
                     ByteContainerView transcript_hash,
                     ByteContainerView peer_dh_public_key,
                     ByteContainerView self_dh_private_key,
                     CleansingVector<uint8_t> *master_secret,
                     CleansingVector<uint8_t> *authenticator_secret) {
  const EVP_MD *digest = nullptr;
  CleansingVector<uint8_t> shared_secret;

  // Generate the shared secret and initialize a hash function for HKDF based on
  // the ciphersuite.
  switch (ciphersuite) {
    case CURVE25519_SHA256:
      // Sanity check the arguments.
      if (peer_dh_public_key.size() != X25519_PUBLIC_VALUE_LEN) {
        return Status(Abort::PROTOCOL_ERROR,
                      absl::StrCat("Public parameter has incorrect size: ",
                                   peer_dh_public_key.size()));
      }
      if (self_dh_private_key.size() != X25519_PRIVATE_KEY_LEN) {
        return Status(Abort::INTERNAL_ERROR,
                      absl::StrCat("Private parameter has incorrect size: ",
                                   self_dh_private_key.size()));
      }

      // Compute the shared secret.
      shared_secret.resize(X25519_SHARED_KEY_LEN);
      if (!X25519(shared_secret.data(), self_dh_private_key.data(),
                  peer_dh_public_key.data())) {
        LOG(ERROR) << "X25519 failed: " << BsslLastErrorString();
        return Status(Abort::INTERNAL_ERROR, "Internal error");
      }

      // Initialize a SHA256-digest for HKDF.
      digest = EVP_sha256();
      break;
    default:
      return Status(
          Abort::BAD_HANDSHAKE_CIPHER,
          "Ciphersuite not supported: " + HandshakeCipher_Name(ciphersuite));
  }

  // Derive the master and authenticator secrets using HKDF.
  std::string salt(kEkepHkdfSalt);
  CleansingVector<uint8_t> output_key;
  output_key.resize(kEkepSecretSize);
  if (!HKDF(output_key.data(), kEkepSecretSize, digest, shared_secret.data(),
            shared_secret.size(),
            reinterpret_cast<const uint8_t *>(salt.data()), salt.size(),
            transcript_hash.data(), transcript_hash.size())) {
    LOG(ERROR) << "HKDF failed: " << BsslLastErrorString();
    return Status(Abort::INTERNAL_ERROR, "Internal error");
  }

  // Copy the master secret.
  std::copy(output_key.cbegin(), output_key.cbegin() + kEkepMasterSecretSize,
            std::back_inserter(*master_secret));

  // Copy the authenticator secret.
  std::copy(output_key.cbegin() + kEkepMasterSecretSize, output_key.cend(),
            std::back_inserter(*authenticator_secret));

  return Status::OkStatus();
}

Status DeriveRecordProtocolKey(const HandshakeCipher &ciphersuite,
                               const RecordProtocol &record_protocol,
                               ByteContainerView transcript_hash,
                               ByteContainerView master_secret,
                               CleansingVector<uint8_t> *record_protocol_key) {
  record_protocol_key->clear();
  const EVP_MD *digest = nullptr;
  switch (ciphersuite) {
    case CURVE25519_SHA256:
      digest = EVP_sha256();
      break;
    default:
      return Status(
          Abort::BAD_HANDSHAKE_CIPHER,
          "Ciphersuite not supported: " + HandshakeCipher_Name(ciphersuite));
  }

  // Resize the output vector to an appropriate size for the selected record
  // protocol.
  switch (record_protocol) {
    case SEAL_AES128_GCM:
      record_protocol_key->resize(kSealAes128GcmKeySize);
      // Randomize the key bytes just in case the key is mistakenly used even
      // when the key derivation fails. The byte-sequence in uninitialized
      // memory could be predictable and, as a result, an attacker may be able
      // to recover data that is encrypted by a key whose underlying bytes are
      // uninitialized. Initializing the key with a truly random value makes it
      // impossible for an attacker to recover any data that is mistakenly
      // encrypted with the key.
      RAND_bytes(record_protocol_key->data(), record_protocol_key->size());
      break;
    default:
      return Status(Abort::BAD_RECORD_PROTOCOL,
                    "Record protocol not supported " +
                        RecordProtocol_Name(record_protocol));
  }

  std::string salt(kEkepHkdfSaltRecordProtocol);
  if (!HKDF(record_protocol_key->data(), record_protocol_key->size(), digest,
            master_secret.data(), master_secret.size(),
            reinterpret_cast<const uint8_t *>(salt.data()), salt.size(),
            transcript_hash.data(), transcript_hash.size())) {
    LOG(ERROR) << "HKDF failed: " << BsslLastErrorString();
    return Status(Abort::INTERNAL_ERROR, "Internal error");
  }
  return Status::OkStatus();
}

Status ComputeClientHandshakeAuthenticator(
    const HandshakeCipher &ciphersuite, ByteContainerView authenticator_secret,
    CleansingVector<uint8_t> *authenticator) {
  std::string input(kClientAuthenticatedText);
  return Hmac(ciphersuite, authenticator_secret, input, authenticator);
}

Status ComputeServerHandshakeAuthenticator(
    const HandshakeCipher &ciphersuite, ByteContainerView authenticator_secret,
    CleansingVector<uint8_t> *authenticator) {
  std::string input(kServerAuthenticatedText);
  return Hmac(ciphersuite, authenticator_secret, input, authenticator);
}

bool CheckMacEquality(CleansingVector<uint8_t> mac1,
                      CleansingVector<uint8_t> mac2) {
  if (mac1.size() != mac2.size()) {
    return false;
  }
  return CRYPTO_memcmp(mac1.data(), mac2.data(), mac1.size()) == 0;
}

}  // namespace asylo
