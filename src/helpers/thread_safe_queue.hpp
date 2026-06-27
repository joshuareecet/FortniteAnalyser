#pragma once

#include <deque>
#include <mutex>
#include <stdexcept>
#include <memory>
#include <exception>

namespace helpers{
    
    struct EmptyQueue : std::exception{
        const char* what() const noexcept override {
            return "ERROR: Queue is empty!";
        }
    };
    
    template <typename T>   
    class ThreadSafeQueue{
    private:
    std::deque<T> queue_{};
    mutable std::mutex mutex_{};

    public:
        // Constructors
        ThreadSafeQueue() = default;

        ThreadSafeQueue(std::initializer_list<T> init_list)
            : queue_{init_list} {}
        
        ThreadSafeQueue(const ThreadSafeQueue& other){
            std::lock_guard<std::mutex> lock(other.mutex_);
            queue_ = other.queue_;
        }
        
        ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
        ThreadSafeQueue(ThreadSafeQueue&&) = delete;
        ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

        
        // copy snapshot of entire queue
        std::deque<T> copy_queue() const{
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_;
        }
        
        bool empty() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.empty();
        }

        // push, pop methods
        void push_back(T value){
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push_back(std::move(value));
        }
        
        void push_front(T value){
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push_front(std::move(value));
        }
        
        void pop_back(T& value){            
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty()) throw EmptyQueue();
            value = queue_.back();
            queue_.pop_back();
        }
        
        std::shared_ptr<T> pop_back(){
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty()) throw EmptyQueue();
            std::shared_ptr<T> const res = std::make_shared<T>(queue_.back());
            queue_.pop_back();
            return res;
        }

        void pop_front(T& value){
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty()) throw EmptyQueue();            
            value = queue_.front();
            queue_.pop_front();
        }
        std::shared_ptr<T> pop_front(){
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty()) throw EmptyQueue();
            
            std::shared_ptr<T> const res = std::make_shared<T>(queue_.front());
            queue_.pop_front();
            return res;
        }

        // operator overloads
        T operator()(std::size_t i){
            std::lock_guard<std::mutex> lock(mutex_);
            if (i >= queue_.size()) {
                throw std::invalid_argument("ERROR: invalid index!");
            }
            return queue_[i];
        }
                
        std::size_t size() const{
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.size();
        }
        
    };
};