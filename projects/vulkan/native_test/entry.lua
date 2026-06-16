---@diagnostic disable: undefined-global

-- https://pthom.github.io/imgui_explorer/

-- https://google.github.io/filament/Filament.md.html
-- https://learnopengl.com/Advanced-Lighting/Deferred-Shading

local import = GLTF.import("asset/scene.glb")
import.parent = Scene

local function draw_instance(instance)
	local children = instance.children
	local child_count = #children

	ImGui.TableNextRow()
	ImGui.TableNextColumn()

	local flags = ImGui.TreeNodeFlags_SpanFullWidth + ImGui.TreeNodeFlags_OpenOnArrow + ImGui.TreeNodeFlags_DefaultOpen
	if child_count == 0 then
		flags = flags + ImGui.TreeNodeFlags_Leaf
	end

	local open = ImGui.TreeNodeEx(instance.name, flags)

	ImGui.TableNextColumn()
	ImGui.Text(tostring(instance.parent and instance.parent.name))

	if open then
		for _, child in pairs(children) do
			draw_instance(child)
		end
		ImGui.TreePop()
	end
end

local camera_position = { x = 0, y = 0, z = 0 }

TODO_RENDER = function()
	if ImGui.Begin("Data Model") then
		local camera
		for i, v in pairs(import.children) do
			if v.name == "Camera" then
				camera = v
			end
		end

		local flags = ImGui.TableFlags_RowBg + ImGui.TableFlags_Borders + ImGui.TableFlags_Resizable

		if ImGui.BeginTable("datamodel", 3, flags) then
			ImGui.TableSetupColumn("Name")
			ImGui.TableSetupColumn("Parent", ImGui.TableColumnFlags_WidthFixed, 80)
			ImGui.TableSetupColumn("Children", ImGui.TableColumnFlags_WidthFixed, 70)
			ImGui.TableHeadersRow()
			draw_instance(Scene)
			ImGui.EndTable()

			local _, x = ImGui.SliderFloat("X", camera_position.x, -1.0, 1.0)
			local _, y = ImGui.SliderFloat("Y", camera_position.y, -1.0, 1.0)
			local _, z = ImGui.SliderFloat("Z", camera_position.z, -10.0, 10.0)
			camera_position.x, camera_position.y, camera_position.z = x, y, z

			if camera then
				-- camera.transform = Transform.new(Vec3.new(x, y, z))
			end
		end
	end
	ImGui.End()
end

-- local box_object = Object.new()
-- box_object.transform = Mat4.new(1):translate(Vec3.new(0, 4, 0))
-- box_object:add_component(ECS_MESH_COMPONENT, { mesh = box })
-- box_object:add_component(ECS_MESH_TEXTURE_COMPONENT, { texture = texture })
-- box_object:add_component(ECS_PHYSICS_COMPONENT, { mesh = box, is_dynamic = true })

-- local top_object = Object.new()
-- top_object.transform = Mat4.new(1):translate(Vec3.new(1, 6, 0.25))
-- top_object:add_component(ECS_MESH_COMPONENT, { mesh = box, color = Vec4.new(1, 1, 1, 0.5) })
-- top_object:add_component(ECS_MESH_TEXTURE_COMPONENT, { texture = texture })
-- top_object:add_component(ECS_PHYSICS_COMPONENT, { mesh = box, is_dynamic = true })

-- local framebuffer = Framebuffer.new(512, 512)
-- local base_matrix = Mat4.new(1)

-- -- you must create a central camera yourself to define the default position of it
-- World.rendering.camera = primary_camera
-- primary_camera.transform = base_matrix

-- function render_pass()
-- 	World.rendering.render_pass()

-- 	Gizmo.set_line_width(0.05)
-- 	Gizmo.draw_grid(100, 100, Vec3.new(0.25, 0.25, 0.25))

-- 	Gizmo.set_line_width(2)
-- 	Gizmo.draw_line(Vec3.new(), Vec3.new(10, 0, 0), Vec3.new(1, 0, 0))
-- 	Gizmo.draw_line(Vec3.new(), Vec3.new(0, 10, 0), Vec3.new(0, 1, 0))
-- 	Gizmo.draw_line(Vec3.new(), Vec3.new(0, 0, 10), Vec3.new(0, 0, 1))
-- end

-- World.rendering.step:connect(function(delta_time)
-- 	local dim = Window.get_window_dimensions()
-- 	local alpha = math.sin(now() * 0.001) * 0.5

-- 	primary_camera.transform = Mat4.new(1)
-- 		:translate(Vec3.new(0, 0, -10))
-- 		:rotate(0.25, Vec3.new(1, 0, 0))
-- 		:rotate((now() / 5000) % math.pi * 2, Vec3.new(0, 0.5, 0))

-- 	secondary_camera.transform =
-- 		Mat4.new(1):translate(Vec3.new(0, 0, -10)):rotate(0.25, Vec3.new(1, 0, 0)):rotate(0.25, Vec3.new(0, 0.5, 0))

-- 	if ImGui.Begin("LuaBSGE ImGui Demo") then
-- 		ImGui.Image(framebuffer.texture_id, Vec2.new(300, 300), false)

-- 		ImGui.Text("FPS: " .. string.format("%.1f", 1000.0 / (delta_time * 1000)))
-- 		ImGui.Separator()
-- 	end
-- 	ImGui.End()
-- end)
