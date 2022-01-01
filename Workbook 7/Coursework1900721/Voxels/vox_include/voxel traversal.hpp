#pragma once

#include "voxel_data.hpp"
#include "libs/glm-0.9.7.2/glm/glm.hpp"
#include<vector>

struct vec2i{
    int x, y;

    vec2i() = default;

    vec2i(float x, float y):
        x(x),
        y(y)
    {}
};

struct vec3i{
    int x, y, z;

    vec3i() = default;

    vec3i(float x, float y, float z):
        x(x),
        y(y),
        z(z)
    {}
};

//Implimentation of the DDA algorithm for raycasting on tilemaps
//Code modified slightly from https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Videos/OneLoneCoder_PGE_RayCastDDA.cpp
    // DDA Algorithm ==============================================
    // https://lodev.org/cgtutor/raycasting.html
std::vector<glm::vec2> getDDACoords2D(const glm::vec2& start, const glm::vec2& end, int tmapwidth, int tmapheight){
    //Code modified slightly from https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Videos/OneLoneCoder_PGE_RayCastDDA.cpp
    // DDA Algorithm ==============================================
    // https://lodev.org/cgtutor/raycasting.html

    std::vector<glm::vec2> tiles;

    // Form ray cast from player into scene
    glm::vec2 vRayStart = start;
    glm::vec2 vRayDir = end - start;
    glm::normalize(vRayDir);
            
    // Lodev.org also explains this additional optimistaion (but it's beyond scope of video)
    glm::vec2 vRayUnitStepSize = { abs(1.0f / vRayDir.x), abs(1.0f / vRayDir.y) };

    //sf::Vector2f vRayUnitStepSize = { sqrt(1 + (vRayDir.y / vRayDir.x) * (vRayDir.y / vRayDir.x)), sqrt(1 + (vRayDir.x / vRayDir.y) * (vRayDir.x / vRayDir.y)) };
    vec2i vMapCheck(vRayStart.x, vRayStart.y);
    glm::vec2 vRayLength1D;
    vec2i vStep;

    // Establish Starting Conditions
    if (vRayDir.x < 0)
    {
        vStep.x = -1;
        vRayLength1D.x = (vRayStart.x - float(vMapCheck.x)) * vRayUnitStepSize.x;
    }
    else
    {
        vStep.x = 1;
        vRayLength1D.x = (float(vMapCheck.x + 1) - vRayStart.x) * vRayUnitStepSize.x;
    }

    if (vRayDir.y < 0)
    {
        vStep.y = -1;
        vRayLength1D.y = (vRayStart.y - float(vMapCheck.y)) * vRayUnitStepSize.y;
    }
    else
    {
        vStep.y = 1;
        vRayLength1D.y = (float(vMapCheck.y + 1) - vRayStart.y) * vRayUnitStepSize.y;
    }

    bool inBounds = true;
    bool notYetFound = true;
    
    while (notYetFound && inBounds)
    {
        // Walk along shortest path
        if (vRayLength1D.x < vRayLength1D.y)
        {
            vMapCheck.x += vStep.x;
            vRayLength1D.x += vRayUnitStepSize.x;
        }
        else
        {
            vMapCheck.y += vStep.y;
            vRayLength1D.y += vRayUnitStepSize.y;
        }

        // Test tile at new test point
        if (vMapCheck.x >= 0 && vMapCheck.x < tmapwidth && vMapCheck.y >= 0 && vMapCheck.y < tmapheight)
        {
            /*if (vecMap[vMapCheck.y * vMapSize.x + vMapCheck.x] == 1)
            {
                bTileFound = true;
            }*/
            tiles.emplace_back(vMapCheck.x, vMapCheck.y);

            if(vMapCheck.x == end.x && vMapCheck.y == end.y)
                notYetFound = false;
        }
        else
        {
            inBounds = false;
        }
    }

    return tiles;
}


//As above
vx::material_index getDDACoords3D(const glm::vec3& start, const glm::vec3& end, int tmapwidth, int tmapheight, int tmapdepth, vx::Voxel_Chunk& vc){
    std::vector<glm::vec3> tiles;

    // Form ray cast from player into scene
    glm::vec3 vRayStart = start;
    glm::vec3 vRayDir = end - start;
    glm::normalize(vRayDir);
            
    // Lodev.org also explains this additional optimistaion (but it's beyond scope of video)
    glm::vec3 vRayUnitStepSize = { abs(1.0f / vRayDir.x), abs(1.0f / vRayDir.y), abs(1.0f / vRayDir.z) };

    //sf::Vector2f vRayUnitStepSize = { sqrt(1 + (vRayDir.y / vRayDir.x) * (vRayDir.y / vRayDir.x)), sqrt(1 + (vRayDir.x / vRayDir.y) * (vRayDir.x / vRayDir.y)) };
    vec3i vMapCheck(vRayStart.x, vRayStart.y, vRayStart.z);
    glm::vec3 vRayLength1D;
    vec3i vStep;

    // Establish Starting Conditions
    if (vRayDir.x < 0)
    {
        vStep.x = -1;
        vRayLength1D.x = (vRayStart.x - float(vMapCheck.x)) * vRayUnitStepSize.x;
    }
    else
    {
        vStep.x = 1;
        vRayLength1D.x = (float(vMapCheck.x + 1) - vRayStart.x) * vRayUnitStepSize.x;
    }

    if (vRayDir.y < 0)
    {
        vStep.y = -1;
        vRayLength1D.y = (vRayStart.y - float(vMapCheck.y)) * vRayUnitStepSize.y;
    }
    else
    {
        vStep.y = 1;
        vRayLength1D.y = (float(vMapCheck.y + 1) - vRayStart.y) * vRayUnitStepSize.y;
    }

    if (vRayDir.z < 0)
    {
        vStep.z = -1;
        vRayLength1D.z = (vRayStart.z - float(vMapCheck.z)) * vRayUnitStepSize.z;
    }
    else
    {
        vStep.z = 1;
        vRayLength1D.z = (float(vMapCheck.z + 1) - vRayStart.z) * vRayUnitStepSize.z;
    }

    bool inBounds = true;
    bool notYetFound = true;
    
    while (notYetFound && inBounds)
    {
        // Walk along shortest path
        if (vRayLength1D.x < vRayLength1D.y)
        {
            if(vRayLength1D.x < vRayLength1D.z){
                vMapCheck.x += vStep.x;
                vRayLength1D.x += vRayUnitStepSize.x;
            }
            else{
                vMapCheck.z += vStep.z;
                vRayLength1D.z += vRayUnitStepSize.z;
            }
        }
        else
        {
            if(vRayLength1D.y < vRayLength1D.z){
                vMapCheck.y += vStep.y;
                vRayLength1D.y += vRayUnitStepSize.y;
            }
            else{
                vMapCheck.z += vStep.z;
                vRayLength1D.z += vRayUnitStepSize.z;
            }
        }

        // Test tile at new test point
        if (vMapCheck.x >= 0 && vMapCheck.x < tmapwidth && vMapCheck.y >= 0 && vMapCheck.y < tmapheight && vMapCheck.z >= 0 && vMapCheck.z < tmapdepth)
        {
            /*if (vecMap[vMapCheck.y * vMapSize.x + vMapCheck.x] == 1)
            {
                bTileFound = true;
            }*/
            tiles.emplace_back(vMapCheck.x, vMapCheck.y, vMapCheck.z);

            if(vMapCheck.x == end.x && vMapCheck.y == end.y && vMapCheck.z == end.z)
                notYetFound = false;
            
            auto mi = vc.getIndex(vMapCheck.x, vMapCheck.y, vMapCheck.z);
            if(vc.getIndex(vMapCheck.x, vMapCheck.y, vMapCheck.z) != vx::Voxel_Chunk::emptyIndex)
                return mi;
        }
        else
        {
            inBounds = false;
        }
    }

    return vx::Voxel_Chunk::emptyIndex;
}


glm::vec3 operator*(const glm::vec3& left, float right){
    return glm::vec3(left.x * right, left.y * right, left.z * right);
}

void vd(Camera& c, vx::Voxel_Chunk& mvc, Window& window, std::vector<vx::Material>& materials){
    int yDir = window.getHeight() * -0.5;
    for(size_t y = 0; y < window.getHeight(); y+=2){
        int xDir = window.getWidth() * -0.5;
        for(size_t x = 0; x < window.getWidth(); x+=2){
            glm::vec3 direction = glm::vec3(xDir, yDir, focal);
            direction = direction * c.orientation;
            auto mi = getDDACoords3D(c.pos, c.pos + (direction * 40), CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH, mvc);
            if(mi != vx::Voxel_Chunk::emptyIndex){
                window.setPixelColour(x, y, materials[mi].getColourData());
                window.setPixelColour(x+1, y, materials[mi].getColourData());
                window.setPixelColour(x, y+1, materials[mi].getColourData());
                window.setPixelColour(x+1, y+1, materials[mi].getColourData());}
            else{
                window.setPixelColour(x, y, 0xFF666666);
                window.setPixelColour(x+1, y, 0xFF666666);
                window.setPixelColour(x, y+1, 0xFF666666);
                window.setPixelColour(x+1, y+1, 0xFF666666);}

            xDir+=2;
        }
        yDir+=2;
    }
}


struct VoxelHit{
    vx::material_index hit;
    glm::vec3 position;
    glm::vec3 priorPosition;
    uint8_t colourVal;
};

void stepRay(glm::vec3& vRayLength1D, vec3i& vMapCheck, vec3i& vStep, glm::vec3& vRayUnitStepSize, glm::vec3& start){
    if (vRayLength1D.x < vRayLength1D.y)
    {
        if(vRayLength1D.x < vRayLength1D.z){
            vMapCheck.x += vStep.x;
            start.x += vStep.x;
            vRayLength1D.x += vRayUnitStepSize.x;
        }
        else{
            vMapCheck.z += vStep.z;
            start.z += vStep.z;
            vRayLength1D.z += vRayUnitStepSize.z;
        }
    }
    else
    {
        if(vRayLength1D.y < vRayLength1D.z){
            vMapCheck.y += vStep.y;
            start.y += vStep.y;
            vRayLength1D.y += vRayUnitStepSize.y;
        }
        else{
            vMapCheck.z += vStep.z;
            start.z += vStep.z;
            vRayLength1D.z += vRayUnitStepSize.z;
        }
    }
}

std::array<uint8_t, 3> getDDAColour(glm::vec3 start, const glm::vec3& end, int tmapwidth, int tmapheight, int tmapdepth, vx::Voxel_Chunk& vc, const std::vector<vx::Material>& mats){
    // Form ray cast from player into scene
    glm::vec3 vRayStart = start;
    glm::vec3 vRayDir = end - start;
    glm::normalize(vRayDir);
            
    // Lodev.org also explains this additional optimistaion (but it's beyond scope of video)
    glm::vec3 vRayUnitStepSize = { abs(1.0f / vRayDir.x), abs(1.0f / vRayDir.y), abs(1.0f / vRayDir.z) };

    //sf::Vector2f vRayUnitStepSize = { sqrt(1 + (vRayDir.y / vRayDir.x) * (vRayDir.y / vRayDir.x)), sqrt(1 + (vRayDir.x / vRayDir.y) * (vRayDir.x / vRayDir.y)) };
    vec3i vMapCheck(vRayStart.x, vRayStart.y, vRayStart.z);
    glm::vec3 vRayLength1D;
    vec3i vStep;

    VoxelHit vHit;

    // Establish Starting Conditions
    if (vRayDir.x < 0)
    {
        vStep.x = -1;
        vRayLength1D.x = (vRayStart.x - float(vMapCheck.x)) * vRayUnitStepSize.x;
        start.x += vRayStart.x - float(vMapCheck.x);
    }
    else
    {
        vStep.x = 1;
        vRayLength1D.x = (float(vMapCheck.x + 1) - vRayStart.x) * vRayUnitStepSize.x;
        start.x += float(vMapCheck.x + 1) - vRayStart.x;
    }

    if (vRayDir.y < 0)
    {
        vStep.y = -1;
        vRayLength1D.y = (vRayStart.y - float(vMapCheck.y)) * vRayUnitStepSize.y;
        start.x += vRayStart.y - float(vMapCheck.y);
    }
    else
    {
        vStep.y = 1;
        vRayLength1D.y = (float(vMapCheck.y + 1) - vRayStart.y) * vRayUnitStepSize.y;
        start.x += float(vMapCheck.y + 1) - vRayStart.y;
    }

    if (vRayDir.z < 0)
    {
        vStep.z = -1;
        vRayLength1D.z = (vRayStart.z - float(vMapCheck.z)) * vRayUnitStepSize.z;
        start.z += vRayStart.z - float(vMapCheck.z);
    }
    else
    {
        vStep.z = 1;
        vRayLength1D.z = (float(vMapCheck.z + 1) - vRayStart.z) * vRayUnitStepSize.z;
        start.z += float(vMapCheck.z + 1) - vRayStart.z;
    }

    bool inBounds = true;
    bool notYetFound = true;
    
    while (notYetFound && inBounds){

        stepRay(vRayLength1D, vMapCheck, vStep, vRayUnitStepSize, start);

        vHit.priorPosition = vHit.position;

        // Walk along shortest path

        vHit.position = start + vRayLength1D;

        // Test tile at new test point
        if (vMapCheck.x >= 0 && vMapCheck.x < tmapwidth && vMapCheck.y >= 0 && vMapCheck.y < tmapheight && vMapCheck.z >= 0 && vMapCheck.z < tmapdepth)
        {
            
            vHit.hit = vc.getIndex(vMapCheck.x, vMapCheck.y, vMapCheck.z);
            if(vc.getIndex(vMapCheck.x, vMapCheck.y, vMapCheck.z) != vx::Voxel_Chunk::emptyIndex){
                auto mat = mats[vHit.hit];
                std::array<uint8_t, 3> myCol{mat.r, mat.g, mat.b};
                vHit.colourVal = mats[vHit.hit].transparency;
                if(vHit.colourVal != 255){
                    stepRay(vRayLength1D, vMapCheck, vStep, vRayUnitStepSize, start);
                    auto otherCol = getDDAColour(start, start + vRayDir, tmapwidth, tmapheight, tmapdepth, vc, mats);
                    
                    for(size_t i = 0; i < 3; i++)
                        myCol[i] = otherCol[i] * 0.8;

                }

                return myCol;
            }
        }
        else
        {
            inBounds = false;
        }
    }

    return std::array<uint8_t, 3>{0, 0,0};
}

void vdWithTransparency(Camera& c, vx::Voxel_Chunk& mvc, Window& window, std::vector<vx::Material>& materials){
    int yDir = window.getHeight() * -0.5;
    for(size_t y = 0; y < window.getHeight(); y+=1){
        int xDir = window.getWidth() * -0.5;
        for(size_t x = 0; x < window.getWidth(); x+=1){
            glm::vec3 direction = glm::vec3(xDir, yDir, focal);
            direction = direction * c.orientation;
            auto colour = getDDAColour(c.pos, c.pos + (direction * 40), CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH, mvc, materials);
            Colour c(colour[0], colour[1], colour[2]);
            window.setPixelColour(x, y, getColourData(c));

            xDir+=1;
        }
        yDir+=1;
    }
}


