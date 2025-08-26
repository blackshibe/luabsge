#include "tbo.h"

// GL_RGBA32F
TextureBufferObject setup_tbo(int packing_size, uint max_size, uint obj_size) {
    TextureBufferObject obj;
    obj.item_size = obj_size;

#if USE_EMSCRIPTEN
    // // WebGL doesn't support texture buffer objects
    // // Use array buffer as fallback
    // glGenBuffers(1, &obj.tboBuffer);
    // glBindBuffer(GL_ARRAY_BUFFER, obj.tboBuffer);
    // glBufferData(GL_ARRAY_BUFFER, max_size * obj_size, NULL, GL_DYNAMIC_DRAW);
    
    // glGenTextures(1, &obj.tboTexture);
    // glBindTexture(GL_TEXTURE_2D, obj.tboTexture);
#else
    glGenBuffers(1, &obj.tboBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, obj.tboBuffer);
    glBufferData(GL_TEXTURE_BUFFER, max_size * obj_size, NULL, GL_DYNAMIC_DRAW);

    glGenTextures(1, &obj.tboTexture);
    glBindTexture(GL_TEXTURE_BUFFER, obj.tboTexture);
    glTexBuffer(GL_TEXTURE_BUFFER, packing_size, obj.tboBuffer);
#endif

    return obj;
}

void upload_tbo_element(TextureBufferObject obj, uint index, const void* item) {
#if USE_EMSCRIPTEN
    // glBindBuffer(GL_ARRAY_BUFFER, obj.tboBuffer);
    // glBufferSubData(GL_ARRAY_BUFFER,
    //                index * obj.item_size,
    //                obj.item_size,
    //                item);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
    glBindBuffer(GL_TEXTURE_BUFFER, obj.tboBuffer);
    glBufferSubData(GL_TEXTURE_BUFFER,
                   index * obj.item_size,
                   obj.item_size,
                   item);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
#endif
}