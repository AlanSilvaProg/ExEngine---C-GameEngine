#include "Color.h"

std::shared_ptr<ColorChannel> Color::WHITE = std::make_shared<ColorChannel>(255,255,255,255);
std::shared_ptr<ColorChannel> Color::RED = std::make_shared<ColorChannel>(255,0,0,255);
std::shared_ptr<ColorChannel> Color::GREEN = std::make_shared<ColorChannel>(0,255,0,255);
std::shared_ptr<ColorChannel> Color::BLUE = std::make_shared<ColorChannel>(0,0,255,255);
std::shared_ptr<ColorChannel> Color::BLACK = std::make_shared<ColorChannel>(0,0,0,255);
std::shared_ptr<ColorChannel> Color::GREY   = std::make_shared<ColorChannel>(40,40,40,255);
std::shared_ptr<ColorChannel> Color::YELLOW = std::make_shared<ColorChannel>(255,255,0,255);

Color::Color(const unsigned int r, const unsigned int g, const unsigned int b, const unsigned int a){
    if(colorChannel == nullptr)
    {
        colorChannel = std::make_shared<ColorChannel>(r,g,b,a);
        return;
    }

    colorChannel->r = r;
    colorChannel->r = g;
    colorChannel->r = b;
    colorChannel->r = a;
};

Color::Color(std::shared_ptr<ColorChannel> channel){
    colorChannel = channel;
};

std::shared_ptr<ColorChannel> Color::GetColorChannel(){
    return colorChannel;
};