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
            struct Members{
                bool closed {false};
                size_t max_size {10};
                mutable std::mutex mq {};
                mutable std::condition_variable not_empty;
                mutable std::condition_variable not_full;
                std::deque<T> queue {};
            };
            std::shared_ptr<Members> m {};

        public:
            frame_queue() {
                m = std::make_shared<Members>();
            };

            frame_queue(size_t size_){
                m = std::make_shared<Members>();
                m->max_size = size_;
            }

            frame_queue(size_t size_, std::deque<T> queue_){
                m = std::make_shared<Members>();
                m->max_size = size_;
                m->queue = std::move(queue_);
            };

            size_t size() const {
                std::lock_guard<std::mutex> lock(m->mq);
                return m->queue.size();
            }

            void close(){
                std::unique_lock<std::mutex> lock (m->mq);
                m->closed = true;
                lock.unlock();
                m->not_empty.notify_all();
                m->not_full.notify_all();
            }
            void resume(){
                std::lock_guard<std::mutex> lock (m->mq);
                m->closed = false;
            }
            void clear(){
                std::unique_lock<std::mutex> lock (m->mq);
                m->queue.clear();
                lock.unlock();
                m->not_full.notify_all();
            }

            [[nodiscard]] bool pop(T& value){   
                std::unique_lock<std::mutex> lock(m->mq);
                
                m->not_empty.wait(lock,[this](){return m->queue.size() > 0 || m->closed;});
                if (m->closed && m->queue.size() < 1) return false;
                
                value = std::move(m->queue.back());
                m->queue.pop_back();

                lock.unlock();
                m->not_full.notify_one();

                return true;
            }
            
            [[nodiscard]] std::shared_ptr<T> pop(){
                std::unique_lock<std::mutex> lock(m->mq);
                
                m->not_empty.wait(lock,[this](){return m->queue.size() > 0 || m->closed;});
                if (m->closed && m->queue.size() < 1) return nullptr;
                
                std::shared_ptr<T> value = std::make_shared<T>(std::move(m->queue.back()));
                m->queue.pop_back();

                lock.unlock();
                m->not_full.notify_one();

                return value;
            }
            
            [[nodiscard]] bool push(T value){   
                std::unique_lock<std::mutex> lock(m->mq);
                
                m->not_full.wait(lock,[this](){return m->queue.size() < m->max_size || m->closed;});
                if (m->closed) return false;
                
                m->queue.push_front(std::move(value));

                lock.unlock();
                m->not_empty.notify_one();

                return true;
            }
    };

};
