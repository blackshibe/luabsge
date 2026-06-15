#version 450

#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec4 outFragColor;

struct Vertex {
	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer {
	Vertex vertices[];
};

layout( push_constant ) uniform constants {
	mat4 camera_matrix;
	mat4 object_matrix;
	VertexBuffer vertexBuffer;
} PushConstants;

void main() {
    float depth = gl_FragCoord.z;        // already [0,1] in Vulkan
    outFragColor = vec4(vec3(depth), 1.0);
}
