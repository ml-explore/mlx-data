// Copyright © 2023 Apple Inc.

#include "mlx/data/core/ThreadPool.h"

namespace mlx {
namespace data {
namespace core {

std::shared_ptr<ThreadController> ThreadPool::thread_controller = nullptr;

ThreadPool::ThreadPool(size_t thread_count) {
  if (!thread_controller) {
    thread_controller = std::make_shared<ThreadController>();
  }
  for (size_t i = 0; i < thread_count; ++i) {
    // start waiting threads. Workers listen for changes through
    //  the ThreadPool member condition_variable
    threads_.emplace_back(std::thread([&]() {
      std::unique_lock<std::mutex> queue_lock(task_mutex_, std::defer_lock);

      while (true) {
        queue_lock.lock();
        task_cv_.wait(queue_lock, [&]() -> bool {
          return !tasks_.empty() || stop_threads_;
        });

        // used by dtor to stop all threads without having to
        //  unceremoniously stop tasks. The tasks must all be
        //  finished, lest we break a promise and risk a `future`
        //  object throwing an exception.
        if (stop_threads_ && tasks_.empty())
          return;

        // to initialize temp_task, we must move the unique_ptr
        //  from the queue to the local stack. Since a unique_ptr
        //  cannot be copied (obviously), it must be explicitly
        //  moved. This transfers ownership of the pointed-to
        //  object to *this, as specified in 20.11.1.2.1
        //  [unique.ptr.single.ctor].
        auto temp_task = std::move(tasks_.front());

        tasks_.pop();
        queue_lock.unlock();

        auto thread_state = thread_controller->limit();
        (*temp_task)();
        thread_controller->restore(thread_state);
      }
    }));
  }
}

ThreadPool::~ThreadPool() {
  stop_threads_ = true;
  task_cv_.notify_all();

  for (std::thread& thread : threads_) {
    thread.join();
  }
}
} // namespace core
} // namespace data
} // namespace mlx
