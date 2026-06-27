#pragma once

#include <deque>
#include <mutex>
#include <stdexcept>
#include <memory>
#include <exception>

namespace mtq{
    
    struct EmptyQueue : std::exception{
        const char* what() const noexcept override {
            return "ERROR: Queue is empty!";
        }
    };
    
    template <typename T>   
    class ThreadSafeQueue{
    private:
    
    std::deque<T> queue_{};
    std::size_t max_queue_size_ {};
    double fps_ {60};
    
    mutable std::mutex mq_{};
    mutable std::mutex mdata_{};
    
    
    public:
        // Constructors
        ThreadSafeQueue()
        : max_queue_size_ {10} {}
        

        ThreadSafeQueue(std::initializer_list<T> init_list, std::size_t max_queue_size)
            : queue_{init_list} , max_queue_size_{max_queue_size} {}
        
        ThreadSafeQueue(const ThreadSafeQueue& other){
            std::lock_guard<std::mutex> lock(other.mq_);
            queue_ = other.queue_;
        }
        
        ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
        ThreadSafeQueue(ThreadSafeQueue&&) = delete;
        ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

        // get snapshot of entire deque
        std::deque<T> copy_queue() const{
            std::lock_guard<std::mutex> lock(mq_);
            return queue_;
        }
        
        bool empty() const {
            std::lock_guard<std::mutex> lock(mq_);
            return queue_.empty();
        }

        // push, pop methods
        bool push_back(T& value){
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.size() < max_queue_size_) {
                queue_.push_back(std::move(value));
                return true;
            }
            return false;
        }
        
        bool push_front(T& value){
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.size() < max_queue_size_){
                queue_.push_front(std::move(value));
                return true;
            }
            return false;
        }
        
        void pop_back(T& value){            
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.empty()) throw EmptyQueue();
            value = queue_.back();
            queue_.pop_back();
        }
        
        std::shared_ptr<T> pop_back(){
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.empty()) throw EmptyQueue();
            std::shared_ptr<T> const res = std::make_shared<T>(queue_.back());
            queue_.pop_back();
            return res;
        }

        void pop_front(T& value){
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.empty()) throw EmptyQueue();            
            value = queue_.front();
            queue_.pop_front();
        }
        std::shared_ptr<T> pop_front(){
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.empty()) throw EmptyQueue();
            
            std::shared_ptr<T> const res = std::make_shared<T>(queue_.front());
            queue_.pop_front();
            return res;
        }
        void set_fps(double fps){
            std::lock_guard<std::mutex> lock_fps{mdata_};
            if (fps < 0){
                throw std::invalid_argument("ERROR: invalid fps, must be at least 0");
            }
            fps_ = fps;
        }
        double get_fps(){
            std::lock_guard<std::mutex> lock_fps{mdata_};
            return fps_;
        }

        // operator overloads
        T operator()(std::size_t i){
            std::lock_guard<std::mutex> lock(mq_);
            if (i >= queue_.size()) {
                throw std::invalid_argument("ERROR: invalid index!");
            }
            return queue_[i];
        }
                
        std::size_t size() const{
            std::lock_guard<std::mutex> lock(mq_);
            return queue_.size();
        }
        
    };
};