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

#include "asylo/platform/posix/threading/thread_manager.h"

#include <algorithm>
#include <cstdlib>
#include <memory>

#include "asylo/platform/arch/include/trusted/host_calls.h"
#include "asylo/platform/core/trusted_global_state.h"
#include "asylo/platform/posix/pthread_impl.h"

namespace asylo {

using pthread_impl::PthreadMutexLock;

ThreadManager::Thread::Thread(std::function<void *()> start_routine)
    : start_routine_(std::move(start_routine)) {}

void ThreadManager::Thread::Run() {
  UpdateThreadState(ThreadState::RUNNING);
  ret_ = start_routine_();
  UpdateThreadState(ThreadState::DONE);
}

void *ThreadManager::Thread::GetReturnValue() { return ret_; }

void ThreadManager::Thread::UpdateThreadId(const pthread_t thread_id) {
  PthreadMutexLock lock(&lock_);
  thread_id_ = thread_id;
}

pthread_t ThreadManager::Thread::GetThreadId() {
  PthreadMutexLock lock(&lock_);
  return thread_id_;
}

void ThreadManager::Thread::UpdateThreadState(const ThreadState &new_state) {
  PthreadMutexLock lock(&lock_);
  this->state_ = new_state;
  int ret = pthread_cond_broadcast(&state_change_cond_);
  CHECK_EQ(ret, 0);
}

void ThreadManager::Thread::WaitForThreadToEnterState(
    const ThreadState &desired_state) {
  PthreadMutexLock lock(&lock_);
  while (state_ != desired_state) {
    int ret = pthread_cond_wait(&state_change_cond_, &lock_);
    CHECK_EQ(ret, 0);
  }
}

void ThreadManager::Thread::WaitForThreadToExitState(
    const ThreadState &undesired_state) {
  PthreadMutexLock lock(&lock_);
  while (state_ == undesired_state) {
    int ret = pthread_cond_wait(&state_change_cond_, &lock_);
    CHECK_EQ(ret, 0);
  }
}

ThreadManager *ThreadManager::GetInstance() {
  static ThreadManager *instance = new ThreadManager();
  return instance;
}

int ThreadManager::CreateThread(const std::function<void *()> &start_routine,
                                pthread_t *const thread_id_out) {
  std::shared_ptr<Thread> thread;

  // Add thread entry point to queue of threads waiting to run.
  {
    PthreadMutexLock lock(&queued_threads_lock_);
    queued_threads_.emplace(std::make_shared<Thread>(start_routine));
    thread = queued_threads_.back();
  }

  if (thread == nullptr) {
    return ENOMEM;
  }

  // Exit and create a thread to enter with EnterAndDonateThread().
  if (enc_untrusted_create_thread(GetEnclaveName().c_str())) {
    return ECHILD;
  }

  // Wait until a thread enters and executes the job.
  thread->WaitForThreadToExitState(Thread::ThreadState::QUEUED);

  if (thread_id_out != nullptr) {
    *thread_id_out = thread->GetThreadId();
  }

  return 0;
}

// StartThread is called from trusted_application.cc as the start routine when
// a new thread is donated to the Enclave.
int ThreadManager::StartThread() {
  std::shared_ptr<Thread> thread;

  {
    PthreadMutexLock lock(&queued_threads_lock_);
    // There should be a one-to-one mapping of threads donated to the enclave
    // and threads created from above at the pthread API layer waiting to run.
    // If a thread gets donated and there's no thread waiting to run, something
    // has gone very wrong.
    CHECK(!queued_threads_.empty());

    thread = queued_threads_.front();
    queued_threads_.pop();
  }

  // Bind the Thread we just took off the queue to the thread id of the donated
  // enclave thread we're running under.
  const pthread_t thread_id = pthread_self();
  thread->UpdateThreadId(thread_id);

  // Add the thread to our map so we can find it later.
  {
    PthreadMutexLock lock(&threads_lock_);
    threads_[thread_id] = thread;
  }

  // Run the start_routine.
  thread->Run();

  // Wait for the caller to join before releasing the thread.
  thread->WaitForThreadToEnterState(Thread::ThreadState::JOINED);

  return 0;
}

int ThreadManager::JoinThread(const pthread_t thread_id,
                              void **return_value_out) {
  std::shared_ptr<Thread> thread = GetThread(thread_id);

  if (thread == nullptr) {
    return -1;
  }

  // Wait until the job is finished executing.
  thread->WaitForThreadToEnterState(Thread::ThreadState::DONE);

  if (return_value_out != nullptr) {
    *return_value_out = thread->GetReturnValue();
  }

  thread->UpdateThreadState(Thread::ThreadState::JOINED);

  return 0;
}

std::shared_ptr<ThreadManager::Thread> ThreadManager::GetThread(
    pthread_t thread_id) {
  PthreadMutexLock lock(&threads_lock_);
  auto it = threads_.find(thread_id);
  if (it == threads_.end()) {
    return nullptr;
  }
  return it->second;
}

}  // namespace asylo
