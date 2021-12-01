#pragma once
#include "Window.hpp"
#include "sdwInterfaces.hpp"
#include <algorithm>
#include "Texture.hpp"


int getLargestDistance(const vf& l, const vf& r){
    float xDif = std::abs(l[0] - r[0]) + 1;
    float yDif = std::abs(l[1] - r[1]) + 1;

    return int(std::max(xDif, yDif));
}

//Points should be of the form {x, y}
void drawMattLine(const vf& l, const vf& r, const Colour& col, Window& window){
    auto positions = interpolate(l, r, getLargestDistance(l, r) + 10);
    xAdjustLine(positions);
    for(size_t i = 0; i < positions.size(); i++)
        window.setPixel(col, positions[i][0], l[1], positions[i][4]);
}


//Points should be of the form {x, y, textureX, textureY}
void drawTexturedLine(const vf& l, const vf& r, const TextureMap* tex, Window& window){
    auto positions = interpolate(l, r, getLargestDistance(l, r));
    xAdjustLine(positions);
    size_t i = 0;
    for(; i < positions.size()-1; i++){
        window.setPixel(getColourFromTP(tex, positions[i][2], positions[i][3]), positions[i][0], l[1], positions[i][4]);
        window.setPixel(getColourFromTP(tex, positions[i][2], positions[i][3]), positions[i][0]+1, l[1], positions[i][4]);
    }
    window.setPixel(getColourFromTP(tex, positions[i][2], positions[i][3]), positions[i][0], l[1], positions[i][4]);
}

void drawPointcloudTriangle(const my::CanvasTriangle& tri, Window& window){
    for(size_t i = 0; i < 3; i++)
        window.setPixel(tri.colour, tri.data[i].data[0], tri.data[i].data[1], 100.0);
}

void drawWireframeTriangle(const my::CanvasTriangle& tri, Window& window){
    drawMattLine(tri.data[0].data, tri.data[1].data, tri.colour, window);
    drawMattLine(tri.data[2].data, tri.data[1].data, tri.colour, window);
    drawMattLine(tri.data[0].data, tri.data[2].data, tri.colour, window);
}


void drawTriangle(const my::CanvasTriangle& tri, Window& window){

    unsigned topHalfLength = (tri.data[1].sdw.y) - (tri.data[0].sdw.y);
    unsigned bottomHalfLength = (tri.data[2].sdw.y) - (tri.data[1].sdw.y);
    unsigned longLength = topHalfLength + bottomHalfLength;

    auto smallSide = interpolate(tri.data[0].data, tri.data[1].data, topHalfLength + 5);
    auto smallSide2 = interpolate(tri.data[1].data, tri.data[2].data, bottomHalfLength + 5);
    auto longSide = interpolate(tri.data[0].data, tri.data[2].data, longLength + 10);


    if(bottomHalfLength != 0)
        smallSide.insert(smallSide.end(), smallSide2.begin(), smallSide2.end());
    
    if(tri.sdw.texture == nullptr)
        for(size_t i = 0; i < smallSide.size(); i++)
            drawMattLine(smallSide[i], longSide[i], tri.colour, window);
    else
        for(size_t i = 0; i < smallSide.size(); i++)
            drawTexturedLine(smallSide[i], longSide[i], tri.sdw.texture, window);
}