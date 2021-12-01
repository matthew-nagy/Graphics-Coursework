#pragma once
#include "Interpolate.hpp"
#include <CanvasPoint.h>
#include<algorithm>
#include <array>
#include <CanvasTriangle.h>

namespace my{

    struct CanvasPoint{
        //This is some ugly crud right here
        ::CanvasPoint sdw;
        vf data;

        CanvasPoint() = default;

        CanvasPoint(::CanvasPoint sdw):
            sdw(sdw)
        {
            data.reserve(6);
            data.emplace_back(sdw.x);
            data.emplace_back(sdw.y);
            data.emplace_back(sdw.texturePoint.x);
            data.emplace_back(sdw.texturePoint.y);
            data.emplace_back(sdw.depth);
            data.emplace_back(sdw.brightness);
        }
    };

    struct CanvasTriangle{
        ::CanvasTriangle sdw;
        std::array<CanvasPoint, 3> data;
        Colour colour;

        CanvasPoint& operator[](size_t index){
            return data[index];
        }

        CanvasTriangle(::CanvasTriangle tri):
            sdw(tri)
        {
            std::sort(tri.vertices.begin(), tri.vertices.end(), [](const ::CanvasPoint& l, const ::CanvasPoint& r){return l.y < r.y;});

            for(size_t i = 0; i < 3; i++)
                data[i] = CanvasPoint(tri[i]);
        }
    };

}