#include <memory>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <thread>
#include <functional>
#include <chrono>
#include <algorithm>

#include <helpers/frame_queue.hpp>
#include <helpers/video.hpp>
#include <helpers/timers.hpp>

#if defined(__APPLE__)
    #define VIDEO_CAP_BACKEND cv::CAP_AVFOUNDATION
#elif defined(_WIN32)
    #include <windows.h>
    #define VIDEO_CAP_BACKEND cv::CAP_MSMF
#else
    #define VIDEO_CAP_BACKEND cv::CAP_V4L2
#endif


using fq = mtq::frame_queue<cv::Mat>;

helpers::metadata set_metadata(cv::VideoCapture& cap){
    helpers::metadata metadata;
    metadata.fps = cap.get(cv::CAP_PROP_FPS);
    metadata.height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    metadata.width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    metadata.frame_count = cap.get(cv::CAP_PROP_FRAME_COUNT);
    return metadata;
}

void fill_queue(){
    
}

void capture_video_file(cv::VideoCapture& cap, std::vector<std::unique_ptr<fq>>& queues, helpers::metadata& metadata){
    cv::Mat frame {};
    cv::Mat frame_clone{};

    while (cap.read(frame)){
        for (auto& q : queues){
            frame_clone = frame.clone();
            if (!(*q).push(std::move(frame_clone))) return;
        }
    }
    for (auto& q: queues){
        (*q).close();
    }
}


void display_video(fq& frame_queue, helpers::metadata& metadata, helpers::DisplayTypes window_type){
    cv::Mat frame;
    std::shared_ptr<cv::Mat> frame_ptr {nullptr};
    helpers::Timer timer{};
    auto frame_time = std::chrono::microseconds(static_cast<int>(1000000/metadata.fps));
    
    // lambda used to sleep in between displaying frames
    auto wait_time = [frame_time, &timer]() {
        auto total_time = timer.elapsed_time();
        if (total_time < frame_time){
            std::this_thread::sleep_for((frame_time-total_time));
            //while (timer.elapsed_time() < frame_time) {}; 
        }
    };

    // display using opencv backend
    if (window_type == helpers::OPENCV) {
        cv::namedWindow("Gameplay", cv::WINDOW_NORMAL);
        while (true){
            timer.start();
            
            if (!frame_queue.pop(frame)) {
                return;
            };
            cv::imshow("Gameplay",frame);        

            if (cv::pollKey() == 27) {
                cv::destroyWindow("Gameplay");
                // not sure why but window won't close unless you trigger the gui using pollkey again here
                cv::pollKey();
                frame_queue.close();
                return;
            };
            
            wait_time();
        }
    }

    //display using sfml backend
    else if (window_type == helpers::SFML){
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
                    frame_queue.close();
                    return;
                }
            }
            
            if (!frame_queue.pop(frame)){
                window.close();
                return;
            }
            
            cv::Mat dst_frame {};
            cv::cvtColor(frame, dst_frame, cv::COLOR_BGR2RGBA);
            texture.update(dst_frame.ptr());
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

void inventory_detect(fq& frame_queue){
    
}

int main(int argc, char** argv )
{
    #ifdef _WIN32
        timeBeginPeriod(1);
    #endif

    std::string path;
    if (argc > 1){
        path = argv[1];
    }
    else{
        path = "./gameplay.mp4";
    }

    cv::VideoCapture cap(path, VIDEO_CAP_BACKEND);
    if ( !cap.isOpened() ) return 1;

    // set metadata before we run threads that rely on it
    helpers::metadata metadata = set_metadata(cap);
    
    // initialise frame queues
    std::unique_ptr<fq> display_queue = std::make_unique<fq>();
    std::unique_ptr<fq> process_queue = std::make_unique<fq>();
    
    std::vector<std::unique_ptr<fq>> queues_ {};
    queues_.push_back(std::move(display_queue));
    //queues_.push_back(std::move(process_queue));
    
    
    // spawn threads
    std::jthread capture_thread(capture_video_file, std::ref(cap), std::ref(queues_), std::ref(metadata));
    display_video((*queues_[0]), metadata, helpers::SFML); // display needs to be in main thread for macos support
    
    #ifdef _WIN32
        timeEndPeriod(1);
    #endif

    return 0;
}