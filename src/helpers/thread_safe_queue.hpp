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


        // operator overloads -----------------------------------------------------
        /**
        * @brief Returns the value at index i.
        * @warning throws if i is out of range
        */
        T operator()(std::size_t i){
            std::lock_guard<std::mutex> lock(mq_);
            if (i >= queue_.size()) {
                throw std::invalid_argument("ERROR: invalid index!");
            }
            return queue_[i];
        }
              

        // read-only methods -----------------------------------------------------
        /**
        * @brief Returns true if the queue has no elements.
        */
        bool empty() const {
            std::lock_guard<std::mutex> lock(mq_);
            return queue_.empty();
        }

        /**
        * @brief Returns a read-only pointer to the back (oldest) element.
        * @warning throws if queue is empty
        */
        std::shared_ptr<const T> back() const{
            // maybe we could make these shared lock?
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.size() < 1){
                throw EmptyQueue();
            }
            return std::make_shared(queue_.back());
        }

        /**
        * @brief Returns a read-only pointer to the front (newest) element.
        * @warning throws if queue is empty
        */
        std::shared_ptr<const T> front() const{
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.size() < 1){
                throw EmptyQueue();
            }
            return std::make_shared(queue_.front());
        }

        /**
        * @brief Returns the current number of elements in the queue.
        */
        std::size_t size() const{
            std::lock_guard<std::mutex> lock(mq_);
            return queue_.size();
        }

        /**
        * @brief Returns a copy of the entire underlying deque.
        */
        std::deque<T> copy_queue() const{
            std::lock_guard<std::mutex> lock(mq_);
            return queue_;
        }       

        // push, pop methods -----------------------------------------------------
        /**
        * @brief Pushes a value onto the back if there is space.
                 returns false without pushing if the queue is full
        */
        bool try_push_back(T& value){
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.size() < max_queue_size_) {
                queue_.push_back(std::move(value));
                return true;
            }
            return false;
        }

        /**
        * @brief Pushes a value onto the front if there is space. 
        *        returns false without pushing if the queue is full
        */
        bool try_push_front(T& value){
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.size() < max_queue_size_){
                queue_.push_front(std::move(value));
                return true;
            }
            return false;
        }

        /**
        * @brief Pushes a value onto the back of the queue, evicting an element if full.
        * @warning if queue size > max queue size, will remove elements according to FIFO
        */
        void push_back(T& value){
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.size() >= max_queue_size_) {
                queue_.pop_back();
            }
            queue_.push_back(std::move(value));
        }
        
        /**
        * @brief Pushes a value onto the front of the queue, evicting an element if full.
        * @warning if queue size > max queue size, will remove elements according to FIFO
        */
        void push_front(T& value){
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.size() >= max_queue_size_){
                queue_.pop_back();
            }
            queue_.push_front(std::move(value));
        }
        
        /**
        * @brief Pops the back (oldest) element into value.
        * @warning throws EmptyQueue if the queue is empty
        */
        void pop_back(T& value){
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.empty()) throw EmptyQueue();
            value = queue_.back();
            queue_.pop_back();
        }

        /**
        * @brief Pops the back (oldest) element and returns it.
        * @warning throws EmptyQueue if the queue is empty
        */
        std::shared_ptr<T> pop_back(){
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.empty()) throw EmptyQueue();
            std::shared_ptr<T> const res = std::make_shared<T>(queue_.back());
            queue_.pop_back();
            return res;
        }

        /**
        * @brief Pops the front (newest) element into value.
        * @warning throws EmptyQueue if the queue is empty
        */
        void pop_front(T& value){
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.empty()) throw EmptyQueue();
            value = queue_.front();
            queue_.pop_front();
        }
        
        /**
        * @brief Pops the front (newest) element and returns it.
        * @warning throws EmptyQueue if the queue is empty
        */
        std::shared_ptr<T> pop_front(){
            std::lock_guard<std::mutex> lock(mq_);
            if (queue_.empty()) throw EmptyQueue();

            std::shared_ptr<T> const res = std::make_shared<T>(queue_.front());
            queue_.pop_front();
            return res;
        }

    };
};