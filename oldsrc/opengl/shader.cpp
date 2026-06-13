#include "shader.h"

const char* load_file(const char* filename) {
    char* buffer = NULL;
    long length;
    
    // Use binary mode to avoid line ending issues on Windows
    FILE* file = fopen(filename, "rb");  // Note: "rb" instead of "r"
    
    if (file) {
        fseek(file, 0, SEEK_END);
        length = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        // Allocate space for content + null terminator
        buffer = (char*)malloc(length + 1);
        
        if (buffer) {
            size_t bytes_read = fread(buffer, 1, length, file);
            buffer[bytes_read] = '\0';  // Always add null terminator
        }
        
        fclose(file);
    }
    
    return buffer;
}

bool compile_shader(GLuint* shader, int type, const char* src) {
	if (!src) {
		printf("%s[shader.cpp] no shader source\n%s", ANSI_RED, ANSI_NC);
		return false;
	}

	int success;
	char infolog[512];

	GLuint id = glCreateShader(type);
	glShaderSource(id, 1, &src, 0);
	glCompileShader(id);

	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(id, 512, 0, infolog);
		printf("%s[shader.cpp] failed to compile %s shader:\n%s\n%s",
			ANSI_RED,
			type == GL_VERTEX_SHADER ? "vertex" : "fragment",
			infolog,
			ANSI_NC
		);

		return false;
	}

	*shader = id;

	return true;
}



bool bsge_compile_shader(sol::state &lua, GLuint* _shader, const char* vert_path, const char* frag_path) {
	BSGEConfig config = lua_bsge_get_config(lua);
	GLuint vert_shader;
	GLuint frag_shader;

	const char* actual_vert_path = concat_c_strings(config.default_asset_directory.c_str(), vert_path);
	const char* actual_frag_path = concat_c_strings(config.default_asset_directory.c_str(), frag_path);
	if (!compile_shader(&vert_shader, GL_VERTEX_SHADER, load_file(actual_vert_path))) {
		return false;
	}

	if (!compile_shader(&frag_shader, GL_FRAGMENT_SHADER, load_file(actual_frag_path))) {
		return false;
	}

	GLuint shader = glCreateProgram();
	glAttachShader(shader, vert_shader);
	glAttachShader(shader, frag_shader);
	glLinkProgram(shader);

	int success;
	char infolog[512];
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader, 512, NULL, infolog);
		printf(infolog);
		return false;
	}

	*_shader = shader;

	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

	return true;
}
