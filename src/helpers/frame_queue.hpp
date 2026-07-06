#pragma once

#include <cstddef>
#include <deque>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace mtq{

    template <typename T>
    class frame_queue{
        private:
            bool closed {false};
            size_t max_size {10};
            mutable std::mutex mq {};
            mutable std::condition_variable not_empty;
            mutable std::condition_variable not_full;
            std::deque<T> queue {};

        public:
            frame_queue() = default;
            frame_queue(const frame_queue& b) = delete;
            frame_queue& operator=(const frame_queue&) = delete;
            
            frame_queue& operator=(frame_queue&& other){
                
                if (this == &other) {
                    return *this;
                }

                {
                    std::scoped_lock<std::mutex,std::mutex> lock(mq,other.mq);
                    queue = std::move(other.queue);
                    max_size = other.max_size;
                    closed = other.closed;

                    other.closed = true;
                }
                
                not_empty.notify_all();
                not_full.notify_all();
                other.not_empty.notify_all();
                other.not_full.notify_all();
                return *this;
            };
            
            frame_queue(frame_queue&& other){
                {
                    std::scoped_lock<std::mutex,std::mutex> lock(mq,other.mq);
                    queue = std::move(other.queue);
                    max_size = other.max_size;
                    closed = other.closed;
                    other.closed = true;
                }
                other.not_empty.notify_all();
                other.not_full.notify_all();
            };
            ~frame_queue() = default;

            frame_queue(size_t size_, std::deque<T> queue_)
            : max_size{size_}, queue{std::move(queue_)} {};

            size_t size() const {
                std::lock_guard<std::mutex> lock(mq);
                return queue.size();
            }

            void close(){
                std::unique_lock<std::mutex> lock (mq);
                closed = true;
                lock.unlock();
                not_empty.notify_all();
                not_full.notify_all();
            }
            void resume(){
                std::lock_guard<std::mutex> lock (mq);
                closed = false;
            }
            void clear(){
                std::unique_lock<std::mutex> lock (mq);
                queue.clear();
                lock.unlock();
                not_full.notify_all();
            }

            [[nodiscard]] bool pop(T& value){   
                std::unique_lock<std::mutex> lock(mq);
                
                not_empty.wait(lock,[this](){return queue.size() > 0 || closed;});
                if (closed && queue.size() < 1) return false;
                
                value = std::move(queue.back());
                queue.pop_back();

                lock.unlock();
                not_full.notify_one();

                return true;
            }
            
            [[nodiscard]] std::shared_ptr<T> pop(){
                std::unique_lock<std::mutex> lock(mq);
                
                not_empty.wait(lock,[this](){return queue.size() > 0 || closed;});
                if (closed && queue.size() < 1) return nullptr;
                
                std::shared_ptr<T> value = std::make_shared<T>(std::move(queue.back()));
                queue.pop_back();

                lock.unlock();
                not_full.notify_one();

                return value;
            }
            
            [[nodiscard]] bool push(T value){   
                std::unique_lock<std::mutex> lock(mq);
                
                not_full.wait(lock,[this](){return queue.size() < max_size || closed;});
                if (closed) return false;
                
                queue.push_front(std::move(value));

                lock.unlock();
                not_empty.notify_one();

                return true;
            }
    };

};
