---@diagnostic disable: undefined-global


local camera = Camera.new()
camera.fov = 70 -- degrees
camera.near_clip = 0.1
camera.far_clip = 100

local base_matrix = Mat4.new(1)
camera.position = base_matrix

World.rendering.camera = camera
World.rendering.step:connect(function(delta_time)


end)

