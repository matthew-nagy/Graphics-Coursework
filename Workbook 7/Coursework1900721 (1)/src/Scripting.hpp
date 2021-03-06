#pragma once
#include "SemiMain.hpp"

typedef float(*interpFunction)(float);

template<class Type, size_t length>
std::array<Type, length> operator+(const std::array<Type, length>& l, const std::array<Type, length>& r){
    std::array<Type, length> res;
    for(size_t i = 0; i < length; i++)
        res[i] = l[i] + r[i];
    return res;
}
template<class Type, size_t length>
std::array<Type, length> operator-(const std::array<Type, length>& l, const std::array<Type, length>& r){
    std::array<Type, length> res;
    for(size_t i = 0; i < length; i++)
        res[i] = l[i] - r[i];
    return res;
}

template<class Type, size_t length>
std::array<Type, length> operator*(const std::array<Type, length>& l, float r){
    std::array<Type, length> res;
    for(size_t i = 0; i < length; i++)
        res[i] = l[i] * r;
    return res;
}

template<class Type, size_t length>
array_vector<Type, length> interpolateWeighted(const std::array<Type, length>& from, const std::array<Type, length>& to, size_t count, interpFunction weightFunc){
    std::array<Type, length> totalDifference = to - from;

    array_vector<Type, length> answer;
    answer.reserve(count);

    for(size_t i = 0; i < count; i++){
        answer.emplace_back( from + (totalDifference * weightFunc(float(i+1) / float(count))) );
    }

    return answer;
}


float sineasoidal(float prop){
    return sin(prop * 3.141592 / 2.0);
}

float easeUp(float prop){
    return pow(prop, 2);
}

float easeDown(float prop){
    return sqrt(prop);
}

float linear(float prop){
    return prop;
}

float instant(float prop){
    return 1.0;
}


enum PosType{Positional, Orbital};

struct CameraPosFrame{
    glm::vec3 cameraPos;
    glm::vec3 rayPos;

    float isX, degree;
    glm::vec3 rotationCenter;
    PosType positioningType;

    CameraPosFrame() = default;

    CameraPosFrame(Camera& camera){
        cameraPos = camera.pos;
        rayPos = camera.rayPos;
        positioningType = Positional;
    }

    void setAsTruth(Camera& camera){
        if(positioningType == Positional){
            camera.pos = cameraPos;
            camera.rayPos = rayPos;
        }
        else{

        }
    }

    void printOut(){
        printf("Position %f %f %f %f %f %f\n", cameraPos.x, cameraPos.y, cameraPos.z, rayPos.x, rayPos.y, rayPos.z);
        printf("\n");
    }

    std::array<float, 6> asArray(){
        return std::array<float, 6>{cameraPos.x, cameraPos.y, cameraPos.z, rayPos.x, rayPos.y, rayPos.z};
    }
    CameraPosFrame(std::array<float, 6>& arr){
        cameraPos = glm::vec3(arr[0], arr[1], arr[2]);
        rayPos = glm::vec3(arr[3], arr[4], arr[5]);
        positioningType = Positional;
    }

    CameraPosFrame(std::array<float, 5>& arr){
        rotationCenter = glm::vec3(arr[0], arr[1], arr[2]);
        isX = arr[3] == 1;
        degree = arr[4];
        positioningType = Orbital;
    }
};
enum RotationType{Hard, Lookat};
struct CameraRotFrame{
    float x,y, z;
    RotationType rType;

    CameraRotFrame() = default;
    CameraRotFrame(Camera& camera){
        x = camera.xLook;
        y = camera.yLook;
    }
    CameraRotFrame(std::array<float, 2>& v){
        x = v[0];
        y = v[1];
        rType = Hard;
    }
    CameraRotFrame(std::array<float, 3>& v){
        x = v[0];
        y = v[1];
        z = v[2];
        rType = Lookat;
    }

    void printout(){
        printf("Rotation %f %f\n\n", x, y);
    }

    void setAsTruth(Camera& camera){
        if(rType == Hard)
            camera.setXY(x,y);
        else
            camera.lookAt(glm::vec3(x, y, z));
    }

    std::array<float, 2> asArray(){
        std::array<float, 2> arr;
        arr[0] = x;
        arr[1] = y;
        return arr;
    }
};
struct FlagPack{
	bool shadows;
	bool reflections;
	bool shadings;
	bool quality;
	bool cellShading;
	bool superAntiAliasing;
	bool fastAproxAntiAliasing;
    std::vector<bool*> bools;

    FlagPack() = default;

    FlagPack(const FlagPack& copy){
        setupBools();
        for(size_t i = 0; i < copy.bools.size(); i++)
            *(bools[i]) = *(copy.bools[i]);
    }

    FlagPack(bool hehehaha){
        setupBools();
        shadows = mode::shadows;
        reflections = mode::reflections;
        shadings = mode::shadings;
        quality = mode::quality;
        cellShading = mode::cellShading;
        superAntiAliasing = mode::superAntiAliasing;
        fastAproxAntiAliasing = mode::fastAproxAntiAliasing;
    }

    FlagPack(std::vector<std::string>& setupCommand){
        setupBools();
        for(size_t i = 0; i < bools.size(); i++)
            *(bools[i]) = setupCommand[i+1] == "O" ? true : false;
    }

    void printOut(){
        printf("Flags ");
        for(size_t i = 0; i < bools.size(); i++)
            if(*(bools[i]))
                printf("O ");
            else
                printf("X ");
        printf("\n\n");
    }

    void setAsTruth(){
        mode::shadows = shadows;
        mode::reflections = reflections;
        mode::shadings = shadings;
        mode::quality = quality;
        mode::cellShading = cellShading;
        mode::superAntiAliasing = superAntiAliasing;
        mode::fastAproxAntiAliasing = fastAproxAntiAliasing;
    }

private:
    void setupBools(){
        bools.push_back(&shadows);
        bools.push_back(&reflections);
        bools.push_back(&shadings);
        bools.push_back(&quality);
        bools.push_back(&cellShading);
        bools.push_back(&superAntiAliasing);
        bools.push_back(&fastAproxAntiAliasing);
    }
};


//TO DO, actually get the frames with interpolation, read in from a file, and the in main make sure you can play these back and print values out
//you know. do everything.


float orbitSpeed = 0.1;

std::array<std::array<uint32_t, WIDTH>, HEIGHT>* raytraceInfo = new std::array<std::array<uint32_t, WIDTH>, HEIGHT>;
std::array<std::array<uint32_t, WIDTH*mode::superScalerQuality>, HEIGHT*mode::superScalerQuality>* superScalerRaytraceInfo = new std::array<std::array<uint32_t, WIDTH*mode::superScalerQuality>, HEIGHT*mode::superScalerQuality>;

#include <iostream>

class ScriptedAnimation{
public:

    ScriptedAnimation(Camera& camera, Model& model, Depth_Window* dpWindow, DrawingWindow* slockWindow, std::string sceneName):
        camera(&camera),
        model(&model),
        window(dpWindow),
        slockWindow(slockWindow),
        chunkName(sceneName)
    {}

    void loadProgram(const std::string& programName){
        std::ifstream file(programName, std::ios::in);
        std::string line;
        std::vector<std::string> currentCommandSet;
        while(std::getline(file, line)){
            if(line == "")continue;
            else if(line == "Length"){
                std::getline(file, line);
                length = std::atoi(line.c_str());
                createVectors(length);
            }
            else if(line == "Skip"){
                std::getline(file, line);
                skipTo = std::atoi(line.c_str());
            }
            else if(line == "End"){
                parseCommands(currentCommandSet);
                currentCommandSet.clear();
            }
            else
                currentCommandSet.emplace_back(line);
        }
        printf("Loaded program\n");
    }

    void render(){
        orbitSpeed = orbitSpeed * 20;
        for(size_t i = skipTo; i < length; i++){
            drawFrame(i);
            if(i % 4 == 0)
                printf("%s:\t Rendered %zu\t/%zu\n", chunkName.c_str(), i+1, length);
        }
        printf("What filename for the mp4?\n");
        std::string name;
        std::cin>>name;
        system(std::string("ffmpeg -framerate 20 -start_number 0 -i Frames/" + chunkName + "-%04d.ppm " + name + ".mp4").c_str());
    }

private:

    std::vector<CameraPosFrame> cameraPosFrames;
    std::vector<CameraRotFrame> cameraRotFrames;
    std::vector<FlagPack> flags;
    std::vector<DrawMode> drawModes;
    std::vector<bool> orbiting;

    size_t length;

    Camera* camera;

    size_t skipTo = 0;

    Model* model;

    std::string chunkName;
    Depth_Window* window;
    DrawingWindow* slockWindow;

    void createVectors(size_t size){
        cameraPosFrames.reserve(size);
        cameraRotFrames.reserve(size);
        flags.reserve(size);
        drawModes.reserve(size);
        orbiting.reserve(size);

        cameraPosFrames.emplace_back(*camera);
        cameraRotFrames.emplace_back(*camera);
        flags.emplace_back(true);
        drawModes.emplace_back(Wireframe);
        orbiting.emplace_back(false);

        for(size_t i = 1; i < size; i++){
            cameraPosFrames.emplace_back();
            cameraRotFrames.emplace_back();
            flags.emplace_back();
            orbiting.emplace_back(false);
            drawModes.emplace_back();
        }
    }

    void drawFrame(size_t frameNumber){
		for(size_t i = 0; i < __DFX.size() ;i++)
			__DFX[i]->frameStep();
            
        std::string num = std::to_string(frameNumber);
        while(num.size() < 4)
            num = "0" + num;
        std::string filename = "Frames/" + chunkName + "-" + num;
        flags[frameNumber].setAsTruth();
        drawmode = drawModes[frameNumber];

        if(orbiting[frameNumber]){
            printf("orbiting\n");
			camera->rotatePositionAround('y', orbitSpeed*3);
			camera->lookAt(glm::vec3(0,0,0));
		}
        else{
            cameraPosFrames[frameNumber].setAsTruth(*camera);
            cameraRotFrames[frameNumber].setAsTruth(*camera);
        }
	    //SDL_Event event;
        //slockWindow->pollForInputEvents(event);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !

		// We MUST poll for events - otherwise the window will freeze !

		window->clearPixels();
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		
		switch(drawmode){
			case PointCloud:
				drawPointCloud(*camera, *window, model->triangles);
				break;
			case Wireframe:
				drawWireframe(*camera, *window, model->triangles);
				break;
			case Raster:
				drawRasterisedView(*camera, *window, model->triangles);
				break;
			case Raytrace:
				if(mode::superAntiAliasing){
					ray_raytraceInto<WIDTH*mode::superScalerQuality, HEIGHT*mode::superScalerQuality>(*superScalerRaytraceInfo, *model, *camera);
					ray_drawResult<WIDTH*mode::superScalerQuality, HEIGHT*mode::superScalerQuality>(*superScalerRaytraceInfo, *window);
				}
				else{
					ray_raytraceInto<WIDTH, HEIGHT>(*raytraceInfo, *model, *camera);
					ray_drawResult<WIDTH, HEIGHT>(*raytraceInfo, *window);
				}
				break;
		}
		slockWindow->renderFrame();
        slockWindow->savePPM(filename + ".ppm");
    }

    interpFunction getInterpolationFromCommand(const std::string& request){
        if(request == "linear"){
            return linear;
        }
        else if(request == "sine"){
            return sineasoidal;
        }
        else if(request == "easeUp"){
            return easeUp;
        }
        else if(request == "easeDown"){
            return easeDown;
        }
        else if(request == "instant"){
            return instant;
        }
        printf("%s is not a command\n", request.c_str());
        return instant;
    }

    DrawMode getDrawModeFromCommand(const std::string& drawMode){
        if(drawMode == "Wireframe")
            return Wireframe;
        else if(drawMode == "Raster")
            return Raster;
        else if(drawMode == "Raytrace")
            return Raytrace;
        printf("%s is not a drawer\n", drawMode.c_str());
        return PointCloud;
    }

    void parseCommands(std::vector<std::string>& lines){
        size_t startFrame;
        size_t endFrame;
        interpFunction iFunc;
        for(size_t i = 0; i < lines.size(); i++){
            auto splits = splitStringOn(lines[i], ' ');
            if(splits[0] == "From"){
                startFrame = std::atoi(splits[1].c_str());
                endFrame = std::atoi(splits[3].c_str());
                iFunc = getInterpolationFromCommand(splits[4]);
            }
            else if(splits[0] == "Position"){
                std::array<float, 6> requestedPos;
                for(size_t i = 0; i < 6; i++){
                    requestedPos[i] = std::atof(splits[i+1].c_str());
                }
                auto interps = interpolateWeighted<float, 6>(cameraPosFrames[startFrame-1].asArray(), requestedPos, (endFrame - startFrame) + 1, iFunc);
                for(size_t i = startFrame; i < endFrame + 1; i++)
                    cameraPosFrames[i] = CameraPosFrame(interps[i - startFrame]);
            }
            else if(splits[0] == "Orbit"){
                for(size_t i = startFrame; i <= endFrame; i++)
                    orbiting[i] = true;
            }
            else if(splits[0] == "Rotation"){
                std::array<float, 2> requestedPos;
                for(size_t i = 0; i < 2; i++){
                    requestedPos[i] = std::atof(splits[i+1].c_str());
                }
                auto interps = interpolateWeighted<float, 2>(cameraRotFrames[startFrame-1].asArray(), requestedPos, (endFrame - startFrame) + 1, iFunc);
                for(size_t i = startFrame; i < endFrame + 1; i++)
                    cameraRotFrames[i] = CameraRotFrame(interps[i - startFrame]);
            }
            else if(splits[0] == "Flags"){
                for(size_t i = startFrame; i < endFrame + 1; i++)
                    flags[i] = FlagPack(splits);
            }
            else if(splits[0] == "SetEngine"){
                DrawMode chosenDrawMode = getDrawModeFromCommand(splits[1]);
                for(size_t i = startFrame; i < endFrame + 1; i++)
                    drawModes[i] = chosenDrawMode;
            }
        }
    }

};