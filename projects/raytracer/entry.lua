---@diagnostic disable: undefined-global

local rt_vert_shader = Shader.new()
rt_vert_shader:compile(GL_VERTEX_SHADER, "shader/rt/vertex_default.glsl")

local rt_frag_shader = Shader.new()
rt_frag_shader:compile(GL_FRAGMENT_SHADER, "shader/rt/frag_default.glsl")

local camera = Camera.new()
camera.fov = 70 -- degrees
camera.near_clip = 0.1
camera.far_clip = 100

local base_matrix = Mat4.new(1)
camera.position = base_matrix

World.rendering.camera = camera
World.rendering.step:connect(function(delta_time)
	if ImGui.Begin("LuaBSGE Ray Tracer") then
		ImGui.Text("FPS: " .. string.format("%.1f", 1000.0 / (delta_time * 1000)))
	end
	ImGui.End()
end)
