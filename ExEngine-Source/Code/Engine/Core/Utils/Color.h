#pragma once
#include "ColorChannel.h"
#include "memory"

class Color{
private:
    std::shared_ptr<ColorChannel> colorChannel = nullptr;
public:
    static std::shared_ptr<ColorChannel> WHITE;
    static std::shared_ptr<ColorChannel> RED;
    static std::shared_ptr<ColorChannel> GREEN;
    static std::shared_ptr<ColorChannel> BLUE;
    static std::shared_ptr<ColorChannel> BLACK;
    static std::shared_ptr<ColorChannel> GREY;
    static std::shared_ptr<ColorChannel> YELLOW;

    Color(const unsigned int r, const unsigned int g, const unsigned int b, const unsigned int a);
    Color(std::shared_ptr<ColorChannel> channel);

    std::shared_ptr<ColorChannel> GetColorChannel();
};