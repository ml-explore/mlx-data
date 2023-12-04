// Copyright © 2023 Apple Inc.

#pragma once

// from https://github.com/fbastos1/thread_pool_cpp17.git

#include <condition_variable> //condition_variable
#include <future> //packaged_task
#include <mutex> //unique_lock
#include <queue> //queue
#include <thread> //thread
#include <type_traits> //invoke_result, enable_if, is_invocable
#include <vector> //vector

#include "mlx/data/core/ThreadController.h"

namespace mlx {
namespace data {
namespace core {

class ThreadPool {
 public:
  ThreadPool(size_t thread_count = std::thread::hardware_concurrency());
  ~ThreadPool();

  // since std::thread objects are not copiable, it doesn't make sense for a
  //  ThreadPool to be copiable.
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  template <
      typename F,
      typename... Args,
      std::enable_if_t<std::is_invocable_v<F&&, Args&&...>, int> = 0>
  auto enqueue(F&&, Args&&...);

 private:
  // TaskContainerBase and TaskContainer exist simply as a wrapper around a
  //   MoveConstructible - but not CopyConstructible - Callable object. Since an
  //   std::function requires a given Callable to be CopyConstructible, we
  //   cannot construct one from a lambda function that captures a
  //   non-CopyConstructible object (such as the packaged_task declared in
  //   enqueue) - because a lambda capturing a non-CopyConstructible object is
  //   not CopyConstructible.

  // TaskContainerBase exists only to serve as an abstract base for
  //   TaskContainer.
  class TaskContainerBase {
   public:
    virtual ~TaskContainerBase(){};

    virtual void operator()() = 0;
  };
  using _task_ptr = std::unique_ptr<TaskContainerBase>;

  // TaskContainer takes a typename F, which must be Callable and
  //  MoveConstructible.
  //   Furthermore, F must be callable with no arguments; it can, for example,
  //   be a bind object with no placeholders. F may or may not be
  //   CopyConstructible.
  template <typename F, std::enable_if_t<std::is_invocable_v<F&&>, int> = 0>
  class TaskContainer : public TaskContainerBase {
   public:
    // here, std::forward is needed because we need the construction of _f not
    //  to bind an lvalue reference - it is not a guarantee that an object of
    //  type F is CopyConstructible, only that it is MoveConstructible.
    TaskContainer(F&& func) : f_(std::forward<F>(func)) {}

    void operator()() override {
      f_();
    }

   private:
    F f_;
  };

  // template <typename F> TaskContainer(F) -> TaskContainer<std::decay<F>>;

  std::vector<std::thread> threads_;
  std::queue<_task_ptr> tasks_;
  std::mutex task_mutex_;
  std::condition_variable task_cv_;
  bool stop_threads_ = false;
  static std::shared_ptr<ThreadController> thread_controller;
};

template <
    typename F,
    typename... Args,
    std::enable_if_t<std::is_invocable_v<F&&, Args&&...>, int>>
auto ThreadPool::enqueue(F&& function, Args&&... args) {
  std::unique_lock<std::mutex> queue_lock(task_mutex_, std::defer_lock);
  std::packaged_task<std::invoke_result_t<F, Args...>()> task_pkg(
      // in C++20, this could be:
      // [..., _fargs = std::forward<Args>(args)...]
      [f = std::move(function),
       fargs = std::make_tuple(std::forward<Args>(args)...)]() mutable {
        return std::apply(std::move(f), std::move(fargs));
      });
  std::future<std::invoke_result_t<F, Args...>> future = task_pkg.get_future();

  queue_lock.lock();
  // this lambda move-captures the packaged_task declared above. Since the
  //  packaged_task type is not CopyConstructible, the function is not
  //  CopyConstructible either - hence the need for a TaskContainer to wrap
  //  around it.
  tasks_.emplace(_task_ptr(
      new TaskContainer([task(std::move(task_pkg))]() mutable { task(); })));

  queue_lock.unlock();

  task_cv_.notify_one();

  return std::move(future);
}
} // namespace core
} // namespace data
} // namespace mlx
