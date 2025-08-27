#include "vfx.h"
#include "image.h"
#include "../module/lua_window.h"
#include <glm/gtc/type_ptr.hpp>

struct framebuffer {
    GLuint textureId;
    GLuint FBO;
    GLuint RBO;

    int width;
    int height;

    framebuffer(float w, float h) : width((float)w), height((float)h) {
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);

        glGenRenderbuffers(1, &RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
};

void lua_bsge_init_framebuffer(sol::state &lua) {
    lua.new_usertype<framebuffer>("Framebuffer",
        sol::constructors<framebuffer(float, float)>(),

        "bind_texture", [](framebuffer& effect, int texture_slot) {
            glActiveTexture(texture_slot);
            glBindTexture(GL_TEXTURE_2D, effect.textureId);
        },

        "bind", [](framebuffer& effect) {
            glBindFramebuffer(GL_FRAMEBUFFER, effect.FBO);
            glViewport(0, 0, effect.width, effect.height);
        },

        "unbind", [](framebuffer& effect) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glm::vec2 window_dims = get_window_dimensions();
            glViewport(0, 0, (int)window_dims.x, (int)window_dims.y);
        },

        "clear", [](framebuffer& effect) {
            glBindFramebuffer(GL_FRAMEBUFFER, effect.FBO);
            glViewport(0, 0, effect.width, effect.height);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glm::vec2 window_dims = get_window_dimensions();
            glViewport(0, 0, (int)window_dims.x, (int)window_dims.y);
        },

        "texture_id", &framebuffer::textureId
    );
}