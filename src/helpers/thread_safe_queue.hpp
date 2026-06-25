#pragma once

#include <deque>
#include <mutex>

namespace helpers{
    template <typename T>
    class ThreadSafeQueue{
    private:
        std::mutex lock_{};
        std::deque<T> queue_{};

    public:
    
    using const_iterator = std::deque<T>::const_iterator;    
    using iterator = std::deque<T>::iterator;
    
    iterator begin() {return queue_.begin();}
        const_iterator begin() const {return queue_.begin();}

        iterator end() {return queue_.end();}
        const_iterator end() const {return queue_.end();}    
        
        void push_back(T value){
            queue_.push_back(value);
        }
        void push_front(T value){
            queue_.push_front(value);
        }
        void pop_back(T value){
            queue_.pop_back(value);
        }
        void pop_front(T value){
            queue_.pop_front(value);
        }
    };
};