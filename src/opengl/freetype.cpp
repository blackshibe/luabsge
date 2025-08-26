#include "freetype.h"

#include "../include/colors.h"
#include "../lua/lua.h"
#include "../lua/luax.h"
#include "../opengl/shader.h"

GLuint freetype_text_shader;
GLuint freetype_text_shader_nosampling;
GLuint freetype_vao, freetype_vbo;
FT_Library ft;

struct TextRenderData {
	float w;
	float h;
	float x;
	float y;
};

void freetype_render(
	struct Textlabel label,
	GLuint shader) {

	const char *text = label.text;
	Font font = label.font;

	float total_y = FREETYPE_BASE_FONT_HEIGHT;
	float total_x = 0;

	glUseProgram(shader);
	glUniform3f(glGetUniformLocation(shader, "textColor"), label.color.x, label.color.y, label.color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(freetype_vao);

	// count text size data
	int text_length = strlen(text);
	TextRenderData data[text_length];

	for (int i = 0; i < strlen(text); i++) {
		struct Character ch = font.data[(int)text[i]];

		// newline
		if (text[i] == 10) {
			total_y += FREETYPE_BASE_FONT_HEIGHT;
			total_x = label.position.x;
			continue;
		}

		float w = ch.size.x;
		float h = ch.size.y;

		float xpos = total_x + (ch.bearing.x);
		float ypos = total_y + (ch.size.y - ch.bearing.y);

		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		total_x += (ch.advance >> 6); // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))

		data[i] = {w, h, xpos, ypos};
	}

	total_x *= label.scale;
	total_y *= label.scale;

	for (int i = 0; i < strlen(text); i++) {
		struct Character ch = font.data[(int)text[i]];
		TextRenderData entry = data[i];

		// process newline
		if (text[i] == 10) {
			continue;
		}

		float w = entry.w * label.scale;
		float h = entry.h * label.scale;
		float xpos = entry.x * label.scale;
		float ypos = entry.y * label.scale;

		//  use anchor
		xpos = (xpos - total_x * label.anchor.x);
		ypos = (ypos - total_y * label.anchor.y);

		xpos += label.position.x;
		ypos += label.position.y;

		// // update VBO for each character
		float vertices[6][4] = {
			{xpos, ypos - h, 0.0f, 0.0f},
			{xpos, ypos, 0.0f, 1.0f},
			{xpos + w, ypos, 1.0f, 1.0f},

			{xpos, ypos - h, 0.0f, 0.0f},
			{xpos + w, ypos, 1.0f, 1.0f},
			{xpos + w, ypos - h, 1.0f, 0.0f}

		};

		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.texture_id);
		glEnable(GL_DEPTH_TEST);

		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, freetype_vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glUseProgram(0);
}

void freetype_init(lua_State *L) {
	printf("[freetype.h] init\n");

	lua_pushnumber(L, FREETYPE_BASE_FONT_HEIGHT);
	lua_setglobal(L, "BASE_FONT_HEIGHT");

	bool compiled = bsge_compile_shader(&freetype_text_shader, "shader/font/text_vert.glsl", "shader/font/text_frag.glsl");
	bool compiled_nosampling = bsge_compile_shader(&freetype_text_shader_nosampling, "shader/font/text_vert.glsl", "shader/font/text_nosampling_frag.glsl");

	if (!compiled) {
		printf("%s[freetype.h] couldn't compile text shader%s\n", ANSI_RED, ANSI_NC);
		return;
	}
	if (!compiled_nosampling) {
		printf("%s[freetype.h] couldn't compile nosampling shader%s\n", ANSI_RED, ANSI_NC);
	}

	// configure VAO/VBO for texture quads
	// -----------------------------------
	glGenVertexArrays(1, &freetype_vao);
	glGenBuffers(1, &freetype_vbo);
	glBindVertexArray(freetype_vao);
	glBindBuffer(GL_ARRAY_BUFFER, freetype_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	printf("[freetype.h] finished\n");
}

void freetype_resize_window(float width, float height) {

	float offset_x = 0.0f;
	float offset_y = 0.0f;
	float x = 0.f;
	float y = 0.f;

	if (width > height) {
		offset_y = (height - width) / 2;
	} else {
		offset_x = (width - height) / 2;
	}

	// don't use anything but floats here otherwise bad things happen
	glm::mat4 proj = glm::ortho(offset_x, width - offset_x, height - offset_y, offset_y);

	glUseProgram(freetype_text_shader);
	glUniformMatrix4fv(glGetUniformLocation(freetype_text_shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

	glUseProgram(freetype_text_shader_nosampling);
	glUniformMatrix4fv(glGetUniformLocation(freetype_text_shader_nosampling, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
}

bool freetype_load_font(struct Font *font, const char *font_directory) {
	if (FT_Init_FreeType(&ft)) {
		printf("[freetype.h] couldn't initialize\n");
		return false;
	}

	FT_Face face;
	if (FT_New_Face(ft, font_directory, 0, &face)) {
		printf("[freetype.h] couldn't load font\n");
		return false;
	}

	else {
		// set size to load glyphs as
		FT_Set_Pixel_Sizes(face, 0, FREETYPE_BASE_FONT_HEIGHT);

		// disable byte-alignment restriction
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// load first 256 characters of ASCII set
		for (unsigned char c = 0; c < FREETYPE_CHARSET_SIZE - 1; c++) {

			// Load character glyph
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
				continue;

			// generate texture
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
#if USE_EMSCRIPTEN
				GL_LUMINANCE,
#else
				GL_RED,
#endif
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
#if USE_EMSCRIPTEN
				GL_LUMINANCE,
#else
				GL_RED,
#endif
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer);

			// set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// now store character for later use
			struct Character character = {
				texture,
				(unsigned int)face->glyph->advance.x,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top)};

			font->data[c] = character;
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// destroy FreeType once we're finished
	FT_Done_Face(face);

	return true;
}

void freetype_quit() {
	FT_Done_FreeType(ft);
}
