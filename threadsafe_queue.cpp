#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
template <typename T>
class threadsafe_queue {
 public:
  threadsafe_queue() {}
  threadsafe_queue(const threadsafe_queue& other) {
    std::lock_guard lk(other.mut);
    data_queue = other.data_queue;
  }
  void push(T new_value) {
    std::lock_guard lk(mut);
    data_queue.push(new_value);
    data_cond.notify_one();
  }
  void wait_and_pop(T& value) {
    std::unique_lock lk(mut);
    data_cond.wait(lk, [this] { return !data_queue.empty(); });
    value = data_queue.front();
    data_queue.pop();
  }
  std::shared_ptr<T> wait_and_pop() {
    std::unique_lock lk(mut);
    data_cond.wait(lk, [this] { return !data_queue.empty(); });
    std::shared_ptr res(std::make_shared(data_queue.front()));
    data_queue.pop();
    return res;
  }
  bool try_pop(T& value) {
    std::lock_guard lk(mut);
    if (data_queue.empty())
      return false;
    value = data_queue.front();
    data_queue.pop();
    return true;
  }
  std::shared_ptr<T> try_pop() {
    std::lock_guard lk(mut);
    if (data_queue.empty())
      return std::shared_ptr<T>();
    std::shared_ptr res(std::make_shared(data_queue.front()));
    data_queue.pop();
    return res;
  }
  bool empty() {
    std::lock_guard lk(mut);
    return data_queue.empty();
  }

 private:
  mutable std::mutex mut;
  std::queue<T> data_queue;
  std::condition_variable data_cond;
};