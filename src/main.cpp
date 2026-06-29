#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <thread>
#include <queue>
#include <mutex>
#include <deque>
#include <functional>
#include <helpers/thread_safe_queue.hpp>
#include <helpers/video_metadata.hpp>
#include <chrono>

using tsq = mtq::ThreadSafeQueue<cv::Mat>;

void capture_video_file(cv::VideoCapture& cap, tsq& frame_queue, helpers::metadata& metadata){
    // maybe we throw here instead?
    if ( !cap.isOpened() ) return;
    
    double fps = cap.get(cv::CAP_PROP_FPS);
    metadata.set_fps(fps);
        
    std::size_t frame_count = cap.get(cv::CAP_PROP_FRAME_COUNT);
    cv::Mat end_of_stream {};
    for (std::size_t frame_no {0}; frame_no < frame_count; ++frame_no){
        cv::Mat frame {};
        bool video_finished = !cap.read(frame);
        if (video_finished) {
            break;
        }
        
        while (!frame_queue.push_front(frame)) {
            std::this_thread::sleep_for(std::chrono::milliseconds{static_cast<int>(1000/fps)});
        }
    }
    
    while (!frame_queue.push_front(end_of_stream)) {
            std::this_thread::sleep_for(std::chrono::milliseconds{static_cast<int>(1000/fps)});
    }
    
    std::cout << "eof: " << end_of_stream.total();
}

void display_video(tsq& frame_queue, helpers::metadata& metadata){
    
    cv::Mat frame;
    double fps {metadata.get_fps()};
    while (true){
        if (!frame_queue.empty()){
            frame_queue.pop_back(std::ref(frame));
            
            // close if we got end_of_stream frame. we can switch to a signal later.
            if (frame.total() == 0) return;

            cv::imshow("Gameplay",frame);
        }
        if (cv::waitKey(1000/fps) == 27) {
            cv::destroyWindow("Gameplay");
            return;
        };
    }
}

int main(int argc, char** argv )
{
    std::string path;
    if (argc > 1){
        path = argv[1];
    }
    else{
        path = "./gameplay.mp4";
    }

    tsq frame_queue{};
    helpers::metadata metadata{};
    cv::VideoCapture cap(path);
    
    std::jthread display_thread(display_video, std::ref(frame_queue), std::ref(metadata));
    std::jthread capture_thread(capture_video_file, std::ref(cap), std::ref(frame_queue), std::ref(metadata));
    
    return 0;
}