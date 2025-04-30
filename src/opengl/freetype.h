#pragma once

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
	unsigned int advance;	 // Horizontal offset to advance to next glyph

	glm::vec2 size;	   // Size of glyph
	glm::vec2 bearing; // Offset from baseline to left/top of glyph
};

struct Font {
	struct Character data[FREETYPE_CHARSET_SIZE];
	const char *path;
};

struct Textlabel {
	Textlabel() {
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		position = glm::vec2(10.0f, 10.0f);
		anchor = glm::vec2(0.0f, 0.0f);
		scale = 1.0f;
		text = "Hello world";
	};

	const char *get_text() {
		return text;
	}

	void set_text(const char *new_text) {
		text = strdup(new_text);
	}

	glm::vec3 color; // vertex color
	glm::vec2 position;
	glm::vec2 anchor;
	float scale; // width of the font is FREETYPE_BASE_FONT_HEIGHT * 96
	struct Font font;

	const char *text;
};

extern GLuint freetype_text_shader;
extern GLuint freetype_text_shader_nosampling;
extern GLuint freetype_vao, freetype_vbo;

void freetype_render(
	struct Textlabel font,
	GLuint shader);

void freetype_init(lua_State *L);
void freetype_resize_window(float width, float height);
bool freetype_load_font(struct Font *font, const char *font_directory);
void freetype_quit();
