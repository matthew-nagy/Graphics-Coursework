#pragma once

uint32_t getColourFromTP(const TextureMap* texture, float x, float y){
    unsigned xAlong = std::round(x);
    unsigned yAlong = (std::round(y) * texture->width);
    return texture->pixels[xAlong + yAlong];
}