#pragma once
#include <glm/glm.hpp>

struct ColorChannel{
public:
    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int a;
    glm::vec4 channel;

    ColorChannel(const unsigned int r, const unsigned int g, const unsigned int b, const unsigned int a) : r(r), g(g), b(b), a(a){
        channel.r = r;
        channel.g = g;
        channel.b = b;
        channel.a = a;
    };
};