#pragma once

#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <format>

namespace helpers{
    class metadata{
    private:
        double fps {-1};
        std::mutex mtx{};
        std::condition_variable fps_set{};
        std::condition_variable metadata_set{};

    public:
        void set_fps(double fps){
            std::lock_guard<std::mutex> lock(mtx);
            if (fps < 0){
                throw std::invalid_argument(std::format("ERROR: supplied fps value {0} must be above 0!",fps))  ;
            }
            this->fps = fps;
            fps_set.notify_one();
        }
        double get_fps(){
            std::unique_lock<std::mutex> lock(mtx);
            fps_set.wait(lock,[this](){return this->fps >= 0;});
            return fps;
        }
    };
}