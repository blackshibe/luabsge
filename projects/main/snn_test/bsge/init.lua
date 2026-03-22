World.rendering.camera = Camera.new()
World.rendering.camera.fov = 90 -- degrees
World.rendering.camera.near_clip = 0.1
World.rendering.camera.far_clip = 100
World.rendering.camera.transform = Mat4.new(1)
Window.set_window_size(1240, 1240)
