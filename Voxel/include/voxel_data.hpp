#pragma once

#include <stdint.h>
#include <stdio.h>
#include <fstream>
#include <array>

#define CHUNK_WIDTH 256 // 64
#define CHUNK_HEIGHT 128 // 32
#define CHUNK_DEPTH 256 // 64
#define CHUNK_VOXELS CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH // 64 * 32 * 64

namespace vx{
    struct Material{
        //Colour of the material
        uint8_t r, g, b;
        //Visual data of the material
        uint8_t transparency, reflectivity, luminosity;
        //Special data of the material
        uint8_t effectIndex, modelReference;

        uint32_t getColourData(){
	        uint32_t ret = (255 << 24) + (r << 16) + (g << 8) + b;
	        return ret;
        }

        Material() = default;
        Material(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255):
            r(r),
            g(g),
            b(b),
            transparency(a)
        {}
    };

    typedef uint16_t material_index;

    class Voxel_Chunk{
    public:
        static material_index emptyIndex;
        static material_index debugSolidIndex;

        virtual material_index& getIndex(size_t x, size_t y, size_t z) = 0;
    };

    class Material_Voxel_Chunk : public Voxel_Chunk{
    public:
        material_index& getIndex(size_t x, size_t y, size_t z){
            return indexer[y][z][x];
        }

        void writeOut(std::ofstream& file){
            file.write((char*)data, sizeof(material_index) * CHUNK_VOXELS);
        }

        Material_Voxel_Chunk(){
            create();

            for(size_t y = 0; y < CHUNK_HEIGHT; y++)
                for(size_t z = 0; z < CHUNK_DEPTH; z++)
                    for(size_t x = 0; x < CHUNK_WIDTH; x++)
                        indexer[y][z][x] = Voxel_Chunk::emptyIndex;

            indexer[CHUNK_HEIGHT / 2][CHUNK_DEPTH / 2][CHUNK_WIDTH / 2] = Voxel_Chunk::debugSolidIndex;
            printf("Solid voxel %d %d %d\n", CHUNK_WIDTH / 2, CHUNK_HEIGHT / 2, CHUNK_DEPTH / 2);
        }

        Material_Voxel_Chunk(int numOfMaterials, int gap = 2){
            create();

            for(size_t y = 0; y < CHUNK_HEIGHT; y++)
                for(size_t z = 0; z < CHUNK_DEPTH; z++)
                    for(size_t x = 0; x < CHUNK_WIDTH; x++){
                        bool set = false;
                        if((x / gap) % gap == 0){
                            if((y / gap) % gap == 0){
                                if((z / gap) % gap == 0){
                                    indexer[y][z][x] = (rand() % (numOfMaterials + 1));
                                    set = true;
                                }
                            }
                        }
                        if(!set)
                            indexer[y][z][x] = Voxel_Chunk::emptyIndex;
                    }
        }

        Material_Voxel_Chunk(std::ifstream& file){
            create();
            
            file.read((char*)data, CHUNK_VOXELS * sizeof(material_index));
        }

        ~Material_Voxel_Chunk(){
            for(size_t y = 0; y < CHUNK_HEIGHT; y++)
                free(indexer[y]);
            free(indexer);
            free(data);
        }

    private:
        //Underlying data of the voxels
        material_index* data;
        //Indexer into data
        material_index*** indexer;

        void create(){
            data = (material_index*)malloc(sizeof(material_index) * CHUNK_VOXELS);
            indexer = (material_index***)malloc(sizeof(material_index**) * CHUNK_HEIGHT);
            for(size_t y = 0; y < CHUNK_HEIGHT; y++){
                indexer[y] = (material_index**)malloc(sizeof(material_index*) * CHUNK_DEPTH);
                for(size_t z = 0; z < CHUNK_DEPTH; z++)
                    indexer[y][z] = data + ( (z * CHUNK_WIDTH) + (y * CHUNK_DEPTH * CHUNK_WIDTH) );
            }
        }
    };

    class Debug_Voxel_Chunk : public Voxel_Chunk{
    public:

        material_index& getIndex(size_t x, size_t y, size_t z){
            if(data[y][z][x])
                return Voxel_Chunk::debugSolidIndex;
            return Voxel_Chunk::emptyIndex;
        }

        Debug_Voxel_Chunk(std::fstream& file){
            material_index fileData[CHUNK_VOXELS];
            file.read((char*)fileData, sizeof(material_index) * CHUNK_VOXELS);
            size_t index = 0;
            for(size_t y = 0; y < CHUNK_HEIGHT; y++)
                for(size_t z = 0; z < CHUNK_DEPTH; z++)
                    for(size_t x = 0; x < CHUNK_WIDTH; x++){
                        if( (fileData[index]) == emptyIndex)
                            data[y][z][x] = false;
                        else
                            data[y][z][x] = true;
                        index++;
                    }
        }
    private:
        std::array< std::array<std::array<bool, CHUNK_WIDTH> , CHUNK_DEPTH>, CHUNK_HEIGHT> data;
    };

    material_index Voxel_Chunk::emptyIndex = 0;
    material_index Voxel_Chunk::debugSolidIndex = 1;
}