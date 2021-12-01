#pragma once

#include <iostream>
#include <string>

struct Colour{
    uint8_t r, g, b, a;
    Colour() = default;
    Colour(const std::string& red, const std::string& green, const std::string& blue);
    Colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
};