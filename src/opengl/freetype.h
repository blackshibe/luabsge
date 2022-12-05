#pragma once

// move /usr/include/freetype2/* to /usr/lib/ to get it to work
#include <ft2build.h>
#include FT_FREETYPE_H

#include "../glad/glad.h"
#include <GLFW/glfw3.h>

#include <lua.hpp>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define FREETYPE_BASE_FONT_HEIGHT 96
#define FREETYPE_CHARSET_SIZE 256

struct Character {
	unsigned int texture_id; // ID handle of the glyph texture
	unsigned int advance;    // Horizontal offset to advance to next glyph

	glm::vec2 size;           // Size of glyph
	glm::vec2 bearing;        // Offset from baseline to left/top of glyph
};

struct Font {
	struct Character data[FREETYPE_CHARSET_SIZE];
	const char* path;
};

// storing pointers ended up extremely broken
struct Textlabel {
	struct Font font;
	const char* text;

	glm::vec3 color; // vertex color
	glm::vec2 position; // currently uses a fixed top-left anchor
	float scale; // width of the font is FREETYPE_BASE_FONT_HEIGHT * 96

};

extern GLuint freetype_text_shader;
extern GLuint freetype_text_shader_nosampling;
extern GLuint freetype_vao, freetype_vbo;

void freetype_render(
	struct Textlabel font,
	GLuint shader
);

void freetype_init(lua_State* L);
void freetype_resize_window(float width, float height);
bool freetype_load_font(struct Font* font, const char* font_directory);
void freetype_quit();
