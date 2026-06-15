#version 450

// We need to enable the GL_EXT_buffer_reference extension so that the shader
// compiler knows how to handle these buffer references.
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;

// Then we have the vertex struct, which is the exact same one as the one we have
// on CPU.
struct Vertex {

	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
};

// After that, we declare the VertexBuffer, which is a readonly buffer that has an
// array (unsized) of Vertex structures. by having the buffer_reference in the
// layout, that tells the shader that this object is used from buffer adress.
// std430 is the alignement rules for the structure.
layout(buffer_reference, std430) readonly buffer VertexBuffer{
	Vertex vertices[];
};

// We have our push_constant block which holds a single instance of our
// VertexBuffer, and a matrix. Because the vertex buffer is declared as
// buffer_reference, this is a uint64 handle, while the matrix is a normal matrix
// (no references).
//push constants block
layout( push_constant ) uniform constants
{
	mat4 camera_matrix;
	mat4 object_matrix;
	VertexBuffer vertexBuffer;
} PushConstants;

void main()
{
	// From our main(), we index the vertex array using gl_VertexIndex, same as we
	// did with the hardcoded array. We dont have -> like in cpp when accessing
	// pointers, in glsl buffer address is accessed as a reference so it uses . to
	// access it. With the vertex grabbed, we just output the color and position we
	// want, multiplying the position with the render matrix.

	//load vertex data from device adress
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	//output data
	gl_Position = PushConstants.camera_matrix * PushConstants.object_matrix * vec4(v.position, 1.0f);
	outColor = v.color.xyz;
	outUV.x = v.uv_x;
	outUV.y = v.uv_y;
}
