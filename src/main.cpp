#include <condition_variable>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>


#include <thread>
#include <functional>
#include <chrono>
#include <algorithm>

#include <helpers/thread_safe_queue.hpp>
#include <helpers/video.hpp>
#include <helpers/timers.hpp>

using tsq = mtq::ThreadSafeQueue<cv::Mat>;

helpers::metadata set_metadata(cv::VideoCapture& cap){
    helpers::metadata metadata;
    metadata.fps = cap.get(cv::CAP_PROP_FPS);
    metadata.height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    metadata.width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    metadata.frame_count = cap.get(cv::CAP_PROP_FRAME_COUNT);
    return metadata;
}

void capture_video_file(cv::VideoCapture& cap, tsq& frame_queue, helpers::metadata& metadata, std::atomic_bool& stop_flag){

    helpers::Timer timer {};
    cv::Mat end_of_stream {};
    
    auto frame_time = std::chrono::microseconds(static_cast<int>(1000000/metadata.fps));
    cv::Mat frame {};

    while (cap.read(frame)){

        while (frame_queue.try_push_front(frame,std::chrono::microseconds(1)) == std::cv_status::timeout){
            if (stop_flag){
                frame_queue.push_front(end_of_stream);
                return;
            }
        }
    }
    frame_queue.push_front(end_of_stream);
}


void display_video(tsq& frame_queue, helpers::metadata& metadata, helpers::DisplayTypes window_type){
    cv::Mat frame;
    helpers::Timer timer{};
    auto frame_time = std::chrono::microseconds(static_cast<int>(1000000/metadata.fps));
    
    auto wait_time = [frame_time, &timer]() {
        auto total_time = timer.elapsed_time();
        if (total_time < frame_time){
            std::this_thread::sleep_for(0.8*(frame_time-total_time));
            while (timer.elapsed_time() < frame_time) {}; 
        }
    };

    if (window_type == helpers::OPENCV) {
        cv::namedWindow("Gameplay", cv::WINDOW_OPENGL);
        while (true){
            timer.start();
            frame_queue.try_pop_back(frame);

            cv::imshow("Gameplay",frame);        
            if (cv::pollKey() == 27) {
                cv::destroyWindow("Gameplay");
                cv::pollKey();
                return;
            };

            if (frame.total() == 0) return;
            wait_time();
        }
    }
    else if (window_type == helpers::SFML){
        //sf::Window window(sf::VideoMode({metadata.width,metadata.height}), "Gameplay", sf::Style::Default, sf::State::Fullscreen);
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        unsigned int win_width = std::min(metadata.width, desktop.size.x);
        unsigned int win_height = std::min(metadata.height, desktop.size.y);

        sf::RenderWindow window(sf::VideoMode({win_width, win_height}), "Gameplay");
        sf::View view(sf::FloatRect({0.f, 0.f}, {static_cast<float>(metadata.width), static_cast<float>(metadata.height)}));
        window.setView(view);

        sf::Texture texture;
        sf::Image img;
        if (!texture.resize({metadata.width,metadata.height})){
            return;
        }

        while (window.isOpen()){
            timer.start();
            
            while (const std::optional event = window.pollEvent())
            {
                // "close requested" event: we close the window
                if (event->is<sf::Event::Closed>()){
                    window.close();
                    return;
                }
            }

            frame_queue.try_pop_back(frame);
            if (frame.total() == 0) return;
            
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGBA);
            texture.update(frame.ptr());
            sf::Sprite sprite(texture);
            
            window.clear(sf::Color::Black);
            window.draw(sprite);
            window.display();
            
            wait_time();
        }
        return;
    }
    return;
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
    if ( !cap.isOpened() ) return 1;
    
    // set metadata before we run threads that rely on it
    helpers::metadata metadata = set_metadata(cap);
    std::atomic_bool stop_flag {false};
    tsq frame_queue{};
    
    std::jthread capture_thread(capture_video_file, std::ref(cap), std::ref(frame_queue), std::ref(metadata), std::ref(stop_flag));
    
    // running display in main thread to maximise portability
    display_video(frame_queue, metadata, helpers::SFML);
    stop_flag = true;

    return 0;
}