#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <thread>
#include <queue>
#include <mutex>
#include <deque>
#include <functional>
#include <helpers/thread_safe_queue.hpp>

void capture_video(cv::VideoCapture cap){
    // need to figure out something better to do here
    if ( !cap.isOpened() ) return;

    double fps = cap.get(cv::CAP_PROP_FPS);
    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);    
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);    
    int frame_count = cap.get(cv::CAP_PROP_FRAME_COUNT);

    std::cout << "\nfps: " << fps << "\nwidth: " << width;
    std::cout << "\nheight: " << height << "\nframe count: " << frame_count << std::endl;

    cv::Mat frame;
    
    while (cap.read(frame)){
        imshow("Video", frame);
        if (cv::waitKey(1000/fps) == 27) break;
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

    cv::VideoCapture cap(path);
    std::jthread display_thread(capture_video, cap);
    
    display_thread.join();
    return 0;
}