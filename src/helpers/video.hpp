#pragma once


namespace helpers{
    enum DisplayTypes{
        OPENCV,
        SFML,
    };
    
    class metadata{
    public:
        double fps {60};

        unsigned int width {1920};
        unsigned int height {1080};
        int frame_count {0};
        
        metadata() = default;

        metadata(double fps_, unsigned int height_, unsigned int width_, int frame_count_)
        : fps{fps_}, height{height_}, width{width_} , frame_count{frame_count_} {};
    };
}