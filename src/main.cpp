#include <opencv2/opencv.hpp>
#include <thread>
#include <functional>
#include <helpers/thread_safe_queue.hpp>
#include <helpers/video_metadata.hpp>
#include <helpers/timers.hpp>
#include <chrono>

using tsq = mtq::ThreadSafeQueue<cv::Mat>;

void capture_video_file(cv::VideoCapture& cap, tsq& frame_queue, helpers::metadata& metadata){
    // maybe we throw here instead?
    if ( !cap.isOpened() ) return;
    
    helpers::Timer timer {};

    double fps = cap.get(cv::CAP_PROP_FPS);
    std::cout << "FPS: " << fps << std::endl;
    metadata.set_fps(fps);
    
    std::size_t frame_count = cap.get(cv::CAP_PROP_FRAME_COUNT);
    cv::Mat end_of_stream {};
    
    auto frame_time = std::chrono::microseconds(static_cast<int>(1000000/fps));
    for (std::size_t frame_no {0}; frame_no < frame_count; ++frame_no){
        timer.start();
        
        cv::Mat frame {};
        cap.read(frame);
        auto read_frame_t = timer.elapsed_time_ms();

        frame_queue.try_push_front(frame);
        auto total = timer.elapsed_time_ms();
        //std::cout << std::format("Read frame time: {0}, Total time {1}", read_frame_t, total) << std::endl;
    }
    frame_queue.push_front(end_of_stream);
}

void display_video(tsq& frame_queue, helpers::metadata& metadata){
    cv::Mat frame;
    double fps {metadata.get_fps()};
    cv::namedWindow("Gameplay", cv::WINDOW_NORMAL);
    
    helpers::Timer timer{};
    auto frame_time = std::chrono::microseconds(static_cast<int>(1000000/fps));
    while (true){

        timer.start();
        frame_queue.try_pop_back(frame);
        auto pop_time = timer.elapsed_time_ms();
        
        // close if we got end_of_stream frame. we can switch to a signal later.
        if (frame.total() == 0) return;
        
        auto window_create_time = timer.elapsed_time_ms()-pop_time;
        cv::imshow("Gameplay",frame);
        auto imshow_time = timer.elapsed_time_ms() - window_create_time;
        auto total_time = timer.elapsed_time();
        
        if (total_time < frame_time){
            std::this_thread::sleep_for(0.8*(frame_time-total_time));
            while (timer.elapsed_time() < frame_time) {}; 
        }
        
        timer.restart();
        if (cv::pollKey() == 27) {
            cv::destroyWindow("Gameplay");
            return;
        };
        std::cout << "pollkey time: " << timer.elapsed_time_ms() << std::endl;
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

    std::jthread capture_thread(capture_video_file, std::ref(cap), std::ref(frame_queue), std::ref(metadata));

    // HighGUI (imshow/waitKey) is not thread-safe; it must run on the main thread on every platform.
    display_video(frame_queue, metadata);

    return 0;
}