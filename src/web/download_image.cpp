#include "download_image.h"

#if USE_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLFW/emscripten_glfw3.h>

void js_download_image(const char* url) {
    char *path = "/download.temp";
    printf("Downloading...\n");
    emscripten_wget(url, path);
    printf("Download completed\n");

    FILE *img;
    img = fopen("download.temp","r");
    if(img == NULL) {
        printf("ERROR: Can't open file!\n");
    }
}


#else
void js_download_image(const char* url) {
    throw "Tried to load image on native platform";
};
#endif

