
#pragma once
#include <DrawingWindow.h>
#include <Utils.h>
#include "Interpolate.hpp"
#include <array>
#include "../libs/glm-0.9.7.2/glm/glm.hpp"
#include "../sdw/Colour.h"
#include "sdwInterfaces.hpp"
#include "../libs/sdw/ModelTriangle.h"

#define WIDTH 320
#define HEIGHT 240

struct Camera{
    glm::mat3 orientation;
    glm::vec3 position;
    float focalLength;
    glm::vec2 viewDimensions;

    void rotateAroundPoint(const glm::vec3& point, const glm::mat3& transformation){
        position -= point;
        position = position * transformation;
        position += point;
    }

    CanvasPoint getCanvasIntersectionPoint(glm::vec3 vert){
        vert = vert * orientation;
        vert -= position;

        float x = focalLength * (vert.x / vert.z) + (viewDimensions.x / 2);
        float y = focalLength * (vert.y / vert.z) + (viewDimensions.y / 2);

        float depth = vert.z - position.z;
        return CanvasPoint(x, y, depth);
    }

    my::CanvasTriangle getCanvasTriangle(const ModelTriangle& tri){
        CanvasTriangle ct;
        for(size_t i = 0; i < 3; i++){
            ct.vertices[i] = getCanvasIntersectionPoint(tri.vertices[i]);
            ct.vertices[i].texturePoint = tri.texturePoints[i];
        }

        my::CanvasTriangle myCTri(ct);
        myCTri.colour = tri.colour;
        if(tri.texture != nullptr)
            myCTri.sdw.texture = tri.texture;
        else
            myCTri.sdw.texture = nullptr;
        return myCTri;
    }
};

class Window{
public:

    void clearPixels(){
        w.clearPixels();
        for(size_t y = 0; y < HEIGHT; y++)
            for(size_t x = 0; x < WIDTH; x++)
                depthBuffer[y][x] = 0.0f;
    }

    void renderFrame(){
        w.renderFrame();
    }

    void setPixel(unsigned x, unsigned y, Colour c, float depth){
        setPixel(x, y, getColData(c),depth);
    }
    void setPixel(Colour c, float x, float y, float depth){
        setPixel(getColData(c), x, y, depth);
    }

    void setPixel(unsigned x, unsigned y, uint32_t c, float depth){
        uint32_t sdlCol = c;
        x -= leftx;
        y -= lefty;
        drawScaledPixAt(x * scale, y * scale, sdlCol, depth);
    }
    void setPixel(uint32_t c, float x, float y, float depth){
        drawScaledPixAt(std::round(x * float(scale)), std::round(y * float(scale)), c, depth);
    }

    void setViewTopLeft(float x, float y){
        leftx = x;
        lefty = y;
    }

    DrawingWindow& sdlData(){
        return w;
    }

    static uint32_t getColData(const Colour& col){
        uint32_t ret = (col.a << 24) + (uint8_t(col.r) << 16) + (uint8_t(col.g) << 8) + uint8_t(col.b);
        return ret;
    }

    int width(){
        return sizex;
    }

    int height(){
        return sizey;
    }

    Window(unsigned scale):
        w(WIDTH, HEIGHT, false),
        scale(scale),
        leftx(0),
        lefty(0)
    {
        sizex = WIDTH / scale;
        sizey = HEIGHT / scale;
    }

private:
    DrawingWindow w;

    std::array<
        std::array<float, WIDTH>,
    HEIGHT> depthBuffer;

    unsigned scale;
    int leftx, lefty, sizex, sizey;

    void drawScaledPixAt(unsigned x, unsigned y ,uint32_t sdlCol, float depth){
        if(depth <= 0)return;
        float invDepth = 1.0f / depth;
        if(invDepth > depthBuffer[y][x]){
            for(size_t i = 0; i < scale; i++)
                for(size_t j = 0; j < scale; j++)
                    w.setPixelColour(x + j, y + i, sdlCol);
            depthBuffer[y][x] = invDepth;
        }
    }
};

