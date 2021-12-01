#pragma once
#include "DrawingLines.hpp"
#include "Window.hpp"
#include "Parser.hpp"

void drawScenePointcloud(const Model& model, Camera& camera, Window& window){
    for(auto& tri : model.triangles){
        my::CanvasTriangle myCTri = camera.getCanvasTriangle(tri);
        drawPointcloudTriangle(myCTri, window);
    }
}

void drawSceneWireframe(const Model& model, Camera& camera, Window& window){
    for(auto& tri : model.triangles){
        my::CanvasTriangle myCTri = camera.getCanvasTriangle(tri);
        drawWireframeTriangle(myCTri, window);
    }
}

void drawSceneRaster(const Model& model, Camera& camera, Window& window){
    for(auto& tri : model.triangles){
        my::CanvasTriangle myCTri = camera.getCanvasTriangle(tri);
        drawTriangle(myCTri, window);
    }
}
