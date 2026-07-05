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
            frame_queue(const frame_queue& b){
                std::lock_guard<std::mutex> lock(b.mq);
                queue = b.queue;
            };

            frame_queue& operator=(const frame_queue&) = delete;
            frame_queue& operator=(frame_queue&&) = delete;
            frame_queue(frame_queue&&) = delete;

            frame_queue(size_t size_, std::deque<T> queue_)
            : max_size{size_}, queue{queue_} {};

            size_t size(){
                std::lock_guard<std::mutex> lock(mq);
                return queue.size();
            }

            bool pop(T& value){   
                std::unique_lock<std::mutex> lock(mq);
                
                not_empty.wait(lock,[this](){return queue.size() > 0 || closed;});
                if (closed && queue.size() < 1) return false;
                
                value = queue.back();
                queue.pop_back();

                lock.unlock();
                not_full.notify_one();

                return true;
            }
            
            std::shared_ptr<T> pop(){
                std::unique_lock<std::mutex> lock(mq);
                
                not_empty.wait(lock,[this](){return queue.size() > 0 || closed;});
                if (closed && queue.size() < 1) return nullptr;
                
                std::shared_ptr<T> value = std::make_shared<T>(queue.back());
                queue.pop_back();

                lock.unlock();
                not_full.notify_one();

                return value;
            }

            bool push(T& value){   
                std::unique_lock<std::mutex> lock(mq);
                
                not_full.wait(lock,[this](){return queue.size() < max_size || closed;});
                if (closed) return false;
                
                queue.push_front(value);

                lock.unlock();
                not_empty.notify_one();

                return true;
            }

            void close(){
                std::lock_guard<std::mutex> lock (mq);
                closed = true;
                not_empty.notify_all();
                not_full.notify_all();
            }
            void open(){
                std::lock_guard<std::mutex> lock (mq);
                closed = false;
            }

    };

};
