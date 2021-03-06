/*
 *
 * Copyright 2019 Asylo authors
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

#include "asylo/platform/core/generic_enclave_client.h"

#include <cstddef>
#include <memory>

#include "asylo/enclave.pb.h"  // IWYU pragma: export
#include "asylo/platform/core/entry_selectors.h"
#include "asylo/platform/primitives/extent.h"
#include "asylo/platform/primitives/parameter_stack.h"
#include "asylo/platform/primitives/primitives.h"
#include "asylo/util/posix_error_space.h"
#include "asylo/util/status.h"
#include "asylo/util/status_macros.h"

namespace asylo {

Status GenericEnclaveClient::Initialize(const char *name, size_t name_len,
                                        const char *input, size_t input_len,
                                        std::unique_ptr<char[]> *output,
                                        size_t *output_len) {
  primitives::NativeParameterStack params;
  params.PushByCopy(primitives::Extent{name, name_len});
  params.PushByCopy(primitives::Extent{input, input_len});
  ASYLO_RETURN_IF_ERROR(
      primitive_client_->EnclaveCall(kSelectorAsyloInit, &params));

  if (params.empty()) {
    return {error::GoogleError::INVALID_ARGUMENT,
            "Parameter stack empty but expected to contain output extent."};
  }
  primitives::NativeParameterStack::ExtentPtr output_extent = params.Pop();
  *output_len = output_extent->size();
  output->reset(new char[*output_len]);
  memcpy(output->get(), output_extent->As<char>(), *output_len);
  return Status::OkStatus();
}

Status GenericEnclaveClient::Run(const char *input, size_t input_len,
                                 std::unique_ptr<char[]> *output,
                                 size_t *output_len) {
  primitives::NativeParameterStack params;
  params.PushByCopy(primitives::Extent{input, input_len});
  ASYLO_RETURN_IF_ERROR(
      primitive_client_->EnclaveCall(kSelectorAsyloRun, &params));

  if (params.empty()) {
    return {error::GoogleError::INVALID_ARGUMENT,
            "Parameter stack empty but expected to contain output extent."};
  }
  primitives::NativeParameterStack::ExtentPtr output_extent = params.Pop();
  *output_len = output_extent->size();
  output->reset(new char[*output_len]);
  memcpy(output->get(), output_extent->As<char>(), *output_len);
  return Status::OkStatus();
}

Status GenericEnclaveClient::Finalize(const char *input, size_t input_len,
                                      std::unique_ptr<char[]> *output,
                                      size_t *output_len) {
  primitives::NativeParameterStack params;
  params.PushByCopy(primitives::Extent{input, input_len});
  ASYLO_RETURN_IF_ERROR(
      primitive_client_->EnclaveCall(kSelectorAsyloFini, &params));

  if (params.empty()) {
    return {error::GoogleError::INVALID_ARGUMENT,
            "Parameter stack empty but expected to contain output extent."};
  }
  primitives::NativeParameterStack::ExtentPtr output_extent = params.Pop();
  *output_len = output_extent->size();
  output->reset(new char[*output_len]);
  memcpy(output->get(), output_extent->As<char>(), *output_len);
  return Status::OkStatus();
}

Status GenericEnclaveClient::EnterAndInitialize(const EnclaveConfig &config) {
  std::string buf;
  if (!config.SerializeToString(&buf)) {
    return Status(error::GoogleError::INVALID_ARGUMENT,
                  "Failed to serialize EnclaveConfig");
  }

  std::unique_ptr<char[]> output;
  size_t output_len = 0;
  std::string enclave_name = get_name();
  ASYLO_RETURN_IF_ERROR(Initialize(enclave_name.c_str(),
                                   enclave_name.size() + 1, buf.data(),
                                   buf.size(), &output, &output_len));

  // Enclave entry-point was successfully invoked. |output| is guaranteed to
  // have a value.
  StatusProto status_proto;
  if (!status_proto.ParseFromArray(output.get(), output_len)) {
    return Status(error::GoogleError::INTERNAL,
                  "Failed to deserialize StatusProto");
  }
  Status status;
  status.RestoreFrom(status_proto);
  return status;
}

Status GenericEnclaveClient::EnterAndRun(const EnclaveInput &input,
                                         EnclaveOutput *output) {
  std::string buf;
  if (!input.SerializeToString(&buf)) {
    return Status(error::GoogleError::INVALID_ARGUMENT,
                  "Failed to serialize EnclaveInput");
  }

  std::unique_ptr<char[]> output_buf;
  size_t output_len = 0;
  ASYLO_RETURN_IF_ERROR(Run(buf.data(), buf.size(), &output_buf, &output_len));

  // Enclave entry-point was successfully invoked. |output_buf| is guaranteed to
  // have a value.
  EnclaveOutput local_output;
  local_output.ParseFromArray(output_buf.get(), output_len);
  Status status;
  status.RestoreFrom(local_output.status());

  // Set the output parameter if necessary.
  if (output) {
    *output = local_output;
  }

  return status;
}

Status GenericEnclaveClient::EnterAndFinalize(const EnclaveFinal &final_input) {
  std::string buf;
  if (!final_input.SerializeToString(&buf)) {
    return Status(error::GoogleError::INVALID_ARGUMENT,
                  "Failed to serialize EnclaveFinal");
  }

  std::unique_ptr<char[]> output;
  size_t output_len = 0;

  ASYLO_RETURN_IF_ERROR(Finalize(buf.data(), buf.size(), &output, &output_len));

  // Enclave entry-point was successfully invoked. |output| is guaranteed to
  // have a value.
  StatusProto status_proto;
  status_proto.ParseFromArray(output.get(), output_len);
  Status status;
  status.RestoreFrom(status_proto);
  return status;
}

Status GenericEnclaveClient::EnterAndDonateThread() {
  primitives::NativeParameterStack params;
  Status status =
      primitive_client_->EnclaveCall(kSelectorAsyloDonateThread, &params);
  if (!status.ok()) {
    LOG(ERROR) << "EnterAndDonateThread failed " << status;
  }
  return status;
}

Status GenericEnclaveClient::DestroyEnclave() {
  return primitive_client_->Destroy();
}

}  // namespace asylo
