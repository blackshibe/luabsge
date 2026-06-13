#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct EmscriptenImage {
    uint8_t* data;
    int width;
    int height;
    int channels;
};

void js_download_image(const char* url);
void js_free_image_data(EmscriptenImage* img);