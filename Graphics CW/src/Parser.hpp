#pragma once

#include <vector>
#include <string>
#include <fstream>
#include "sdwInterfaces.hpp"
#include "../sdw/ModelTriangle.h"
#include <unordered_map>

std::vector<std::string> splitStringOn(const std::string& in, char splitOn){
    std::vector<std::string> splits;
    splits.reserve(8);

    size_t start = 0;
    for(size_t i = 0; i < in.size(); i++){
        if(in[i] == splitOn){
            splits.emplace_back(in.substr(start, i - start));
            start = i + 1;
        }
    }

    if(start != in.size())
        splits.emplace_back(in.substr(start));

    return splits;
}

struct Material{
    bool isColour;
    Colour colour;
    TextureMap* texture = nullptr;
};

struct Model{
    std::vector<ModelTriangle> triangles;
    std::vector<TextureMap*> textures;
};

void loadMaterials(Model& model, std::unordered_map<std::string, Material>& mats, std::string name){
    std::string mtlName = "";
    std::string line;
    Material currentMat;
    std::ifstream file(name, std::ios::in);
 
    while(std::getline(file, line)){
        if(line == "")continue;
        auto splitLine = splitStringOn(line, ' ');
        if(splitLine[0] == "newmtl"){
            if(mtlName != ""){
                mats.emplace(std::move(mtlName), std::move(currentMat));
            }
            currentMat = Material();
            mtlName = splitLine[1];
        }
        else if(splitLine[0] == "Kd"){
            currentMat.isColour = true;
            currentMat.colour = Colour(splitLine[1], splitLine[2], splitLine[3]);
            printf("Generating colour wih %d %d %d\n", currentMat.colour.r, currentMat.colour.g, currentMat.colour.b);
        }
        else if(splitLine[0] == "map_Kd"){
            TextureMap* tex = new TextureMap(splitLine[1]);
            model.textures.emplace_back(tex);
            currentMat.isColour = false;
            currentMat.texture = tex;
        }
    }
    mats.emplace(std::move(mtlName), std::move(currentMat));

    file.close();
}

glm::vec3 arrToVec3(const std::array<float, 3>& arr){
    return glm::vec3(arr[0], arr[1], arr[2]);
}

Model getObjFromFile(const std::string& filename){
    std::ifstream file(filename, std::ios::in);
    std::string line;
    Model thisModel;
    std::string currentMaterial = "";
    std::unordered_map<std::string, Material> materials;
    std::vector<std::array<float, 3>> vertices;
    std::vector<TexturePoint> texturePoints;

    while(std::getline(file, line)){
        if(line == "")continue;

        auto splitLine = splitStringOn(line, ' ');
        if(splitLine[0] == "o")continue;
        else if(splitLine[0] == "mtllib"){
            loadMaterials(thisModel, materials, splitLine[1]);
        }
        else if(splitLine[0] == "usemtl"){
            currentMaterial = splitLine[1];
        }
        else if(splitLine[0] == "v"){
            std::array<float, 3> vertex;
            for(size_t i = 1; i < 4; i++)
                vertex[i-1] = std::atof(splitLine[i].c_str());
            vertices.emplace_back(vertex);
        }
        else if(splitLine[0] == "vt"){
            texturePoints.emplace_back(std::atof(splitLine[1].c_str()), std::atof(splitLine[2].c_str()));
        }
        else if(splitLine[0] == "f"){
            ModelTriangle tri;
            for(size_t i = 1; i < 4; i++){
                auto vpSplit = splitStringOn(splitLine[i], '/');
                tri.vertices[i-1] = arrToVec3(vertices[std::atoi(vpSplit[0].c_str())]);
                if(vpSplit.size() > 1)
                    tri.texturePoints[i-1] = texturePoints[std::atoi(vpSplit[1].c_str())];
            }
            Material& m = materials[currentMaterial];
            if(m.isColour){
                tri.colour = m.colour;
                tri.texture = nullptr;
            }
            else{
                tri.texture = m.texture;
                tri.colour = Colour(255, 255, 255);
            }
            thisModel.triangles.emplace_back(tri);
        }
    }

    file.close();
    return thisModel;
}






