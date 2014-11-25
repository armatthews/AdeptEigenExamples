#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class WorkQueue {
public:
  void push(const T& item) {
    std::unique_lock<std::mutex> lock(mutex);
    queue.push(item);
    lock.unlock();
    cv.notify_one();
  }

  T wait_pop() {
    std::unique_lock<std::mutex> lock(mutex);
    while (queue.empty()) {
      cv.wait(lock);
    }
    T item = queue.front();
    queue.pop();
    lock.unlock();
    return item; 
  }

  bool pop(T& item) {
    std::unique_lock<std::mutex> lock(mutex);
    if (queue.empty()) {
      lock.unlock();
      return false;
    }
    else {
      item = queue.front();
      queue.pop();
      lock.unlock();
      return true;
    }
  }

private:
  std::queue<T> queue;
  std::mutex mutex;
  std::condition_variable cv;
};
