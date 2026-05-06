---@meta

-- LuaBSGE Type Definitions for EmmyLua

---@class Camera
---@field fov number Field of view in degrees (default: 70.0)
---@field near_clip number Near clipping plane distance (default: 0.1)
---@field far_clip number Far clipping plane distance (default: 100.0)
---@field transform Mat4 4x4 transformation matrix for camera position, rotation, and scale
Camera = {}

---@return Camera
function Camera.new() end

---@return Mat4 projection matrix
function Camera:get_projection_matrix() end

---@param x number Window width
---@param y number Window height
---@return Mat4 projection matrix for specific resolution
function Camera:get_projection_matrix_for_resolution(x, y) end

---Camera.transform - 4x4 transformation matrix controlling camera position, rotation, and scale
---
---This field contains a Mat4 that defines the camera's position and orientation in 3D space.
---The engine uses this matrix to calculate the view transformation for rendering.
---
---Default: Mat4.new(1):translate(Vec3.new(0, 0, 5)) - Identity matrix translated 5 units back on Z-axis
---
---The transform matrix is automatically applied during World.rendering.render_pass() and
---used to set the "camera_transform" uniform in shaders via camera_set_shader_projection_matrix().
---
---Common patterns:
---```lua
----- Basic positioning
---camera.transform = Mat4.new(1):translate(Vec3.new(x, y, z))
---
----- Position with rotation
---camera.transform = Mat4.new(1):translate(pos):rotate(angle, axis)
---
----- Animated camera movement
---local spring_x = spring.new(0.5, 100, 50)
---spring_x.target = mouse_x
---local smooth_x = spring_x:update(delta_time)
---camera.transform = Mat4.new(1):translate(Vec3.new(smooth_x, 0, -4))
---
----- Look-at camera
---camera.transform = Mat4.lookAt(eye_pos, target_pos, up_vector)
---```

---@class Font
Font = {}

---@return Font
function Font.new() end

---@param path string Path to font file
---@return number 0 on success, 1 on error
function Font:load(path) end

---@param character string Character to get info for
---@return table Character information
function Font:get_info_for_char(character) end

---Debug function for font data
function Font:dbg_get_data() end

---@class Image
---@field id number OpenGL texture ID
---@field width number Image width in pixels
---@field height number Image height in pixels
Image = {}

---@param path string Path to image file (automatically loads on construction)
---@return Image
function Image.new(path) end

---@class Shader
Shader = {}

---@return Shader
function Shader.new() end

---@param type number Shader type (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc)
---@param path string Path to shader source file
function Shader:compile(type, path) end

---@class PhysicsObject
PhysicsObject = {}

---Create a new physics body from mesh data
---@param mesh Mesh The mesh object containing geometry and transformation data
---@param is_dynamic boolean true for dynamic (moveable) bodies, false for static bodies
---@return PhysicsObject
function PhysicsObject.new(mesh, is_dynamic) end

---Get the current transformation matrix from physics simulation
---@return Mat4 4x4 transformation matrix representing position, rotation, and scale
function PhysicsObject:get_transform() end

---@class Mesh
---@field texture number OpenGL texture ID to bind during rendering
---@field matrix Mat4 Transformation matrix for mesh position/rotation/scale
Mesh = {}

---@param path string Path to mesh file (.obj, .fbx, etc) - automatically loads on construction
---@return Mesh
function Mesh.new(path) end

---Renders the mesh with current matrix and texture
function Mesh:render() end

---@class Textlabel
---@field color Vec3 Text color (default: white)
---@field position Vec2 Screen position (default: 10,10)
---@field anchor Vec2 Anchor point (default: 0,0)
---@field scale number Text scale multiplier (default: 1.0)
---@field text string Text content
---@field font Font Font to use for rendering
Textlabel = {}

---@return Textlabel
function Textlabel.new() end

function Textlabel:render() end

---@class Signal
Signal = {}

---@return Signal
function Signal.new() end

---@param callback function Function to connect
function Signal:connect(callback) end

---@param ... any Arguments to pass to connected functions
function Signal:fire(...) end

---@class Vec2
---@field x number
---@field y number
Vec2 = {}

---@return Vec2
---@overload fun(value: number): Vec2
---@overload fun(x: number, y: number): Vec2
function Vec2.new() end

---@return number Length of vector
function Vec2:length() end

---@return Vec2 Normalized vector
function Vec2:normalize() end

---@param other Vec2
---@return number Dot product
function Vec2:dot(other) end

---@param other Vec2
---@return number Distance between vectors
function Vec2:distance(other) end

-- Vec2 supports arithmetic operators: +, -, *, /, unary -

---@class Vec3
---@field x number
---@field y number
---@field z number
Vec3 = {}

---@return Vec3
---@overload fun(value: number): Vec3
---@overload fun(x: number, y: number, z: number): Vec3
function Vec3.new() end

---@return number Length of vector
function Vec3:length() end

---@return Vec3 Normalized vector
function Vec3:normalize() end

---@param other Vec3
---@return number Dot product
function Vec3:dot(other) end

---@param other Vec3
---@return Vec3 Cross product
function Vec3:cross(other) end

---@param other Vec3
---@return number Distance between vectors
function Vec3:distance(other) end

-- Vec3 supports arithmetic operators: +, -, *, /, unary -

---@class Vec4
---@field x number
---@field y number
---@field z number
---@field w number
Vec4 = {}

---@return Vec4
---@overload fun(value: number): Vec4
---@overload fun(x: number, y: number, z: number, w: number): Vec4
function Vec4.new() end

---@class Mat4
Mat4 = {}

---@return Mat4
---@overload fun(diagonal: number): Mat4
function Mat4.new() end

---@param translation Vec3
---@return Mat4
function Mat4:translate(translation) end

---@param angle number Rotation angle in radians
---@param axis Vec3 Rotation axis
---@return Mat4
function Mat4:rotate(angle, axis) end

---@param scale Vec3|number Scale vector or uniform scale
---@return Mat4
function Mat4:scale(scale) end

---@return Mat4 Inverted matrix
function Mat4:inverse() end

---@return Mat4 Identity matrix
function Mat4.identity() end

---@param fov number Field of view in radians
---@param aspect number Aspect ratio
---@param near number Near clipping plane
---@param far number Far clipping plane
---@return Mat4 Perspective projection matrix
function Mat4.perspective(fov, aspect, near, far) end

---@param left number
---@param right number
---@param bottom number
---@param top number
---@param near number
---@param far number
---@return Mat4 Orthographic projection matrix
function Mat4.ortho(left, right, bottom, top, near, far) end

---@param eye Vec3 Camera position
---@param center Vec3 Look at target
---@param up Vec3 Up vector
---@return Mat4 View matrix
function Mat4.lookAt(eye, center, up) end

---@return Vec3 Translation component
function Mat4:to_vec3() end

-- Mat4 supports matrix multiplication operator: *

---@class World
---@field rendering {camera: Camera, step: Signal, render_pass: fun()}
---@field input table Input system
World = {}

---World.rendering.render_pass() - Core rendering function that processes and renders all ECS entities
---
---This function performs a complete render pass of the 3D scene:
---1. Calculates hierarchical depths for all ECS entities with transform components
---2. Sorts entities by depth to ensure proper parent-child rendering order
---3. Computes final world transforms by combining local transforms with parent transforms
---4. Updates transforms based on physics bodies when physics components are present
---5. Renders all meshes with proper transformation matrices and textures
---6. Sets up shader uniforms for camera projection and transformation
---
---The render pass handles the Entity-Component-System rendering pipeline automatically.
---Call this function inside your custom render_pass() function after rendering UI elements.
---
---Example usage:
---```lua
---function render_pass()
---    display_label:render()  -- Render UI first
---    World.rendering.render_pass()  -- Render 3D scene
---    Gizmo.draw_grid(100, 100, Vec3.new(0.25, 0.25, 0.25))  -- Render debug gizmos
---end
---```

---@class Window
Window = {}

---@return Vec2 Window dimensions
function Window.get_window_dimensions() end

function Window.quit() end

---@return boolean Window focus state
function Window.get_focused() end

---@param enabled boolean Enable/disable VSync (vertical synchronization)
function Window.set_vsync(enabled) end

---@return boolean Current VSync state
function Window.get_vsync() end

---@param width number Window width in pixels
---@param height number Window height in pixels
function Window.set_window_size(width, height) end

---@param enabled boolean Enable fullscreen mode
function Window.set_fullscreen(enabled) end

---Maximize the window to fill the screen
function Window.maximize() end

---@class Gizmo
Gizmo = {}

---@param start Vec3 Line start position
---@param end_pos Vec3 Line end position
---@param color Vec3 Line color
function Gizmo.draw_line(start, end_pos, color) end

---@param size number Grid size
---@param divisions number Number of grid divisions
---@param color Vec3 Grid color
function Gizmo.draw_grid(size, divisions, color) end

---@param width number Line width
function Gizmo.set_line_width(width) end

---@param enabled boolean Enable/disable depth testing
function Gizmo.set_depth_test(enabled) end

---@class ImGui
ImGui = {}

-- Window functions
---@param name string Window name
---@return boolean Window is open
---@overload fun(name: string, p_open: boolean): boolean
---@overload fun(name: string, p_open: boolean, flags: number): boolean
function ImGui.Begin(name) end

function ImGui.End() end

---@param x number X position in pixels
---@param y number Y position in pixels
function ImGui.SetNextWindowPos(x, y) end

---@param x number Width in pixels
---@param y number Height in pixels
function ImGui.SetNextWindowSize(x, y) end

-- Styling
---@param color Vec4 Text color
function ImGui.PushTextColor(color) end

function ImGui.PopStyleColor() end

-- Text widgets
---@param text string Text to display
function ImGui.Text(text) end

---@param r number Red component
---@param g number Green component
---@param b number Blue component
---@param a number Alpha component
---@param text string Text to display
function ImGui.TextColored(r, g, b, a, text) end

---@param text string Text to display
function ImGui.TextDisabled(text) end

---@param text string Text to display
function ImGui.TextWrapped(text) end

---@param label string Label text
---@param text string Value text
function ImGui.LabelText(label, text) end

---@param text string Text to display
function ImGui.BulletText(text) end

-- Buttons
---@param label string Button label
---@return boolean Button was clicked
---@overload fun(label: string, size_x: number, size_y: number): boolean
function ImGui.Button(label) end

---@param label string Button label
---@return boolean Button was clicked
function ImGui.SmallButton(label) end

-- Input widgets
---@param label string Input label
---@param text string Current text
---@return boolean, string Changed, new text
function ImGui.InputText(label, text) end

---@param label string Input label
---@param value number Current value
---@return boolean, number Changed, new value
function ImGui.InputFloat(label, value) end

---@param label string Input label
---@param value number Current value
---@return boolean, number Changed, new value
function ImGui.InputInt(label, value) end

---@param label string Slider label
---@param value number Current value
---@param v_min number Minimum value
---@param v_max number Maximum value
---@return boolean, number Changed, new value
function ImGui.SliderFloat(label, value, v_min, v_max) end

---@param label string Slider label
---@param value number Current value
---@param v_min number Minimum value
---@param v_max number Maximum value
---@return boolean, number Changed, new value
function ImGui.SliderInt(label, value, v_min, v_max) end

---@param label string Checkbox label
---@param value boolean Current value
---@return boolean, boolean Changed, new value
function ImGui.Checkbox(label, value) end

-- Layout
function ImGui.Separator() end

---@overload fun(offset_from_start_x: number): nil
---@overload fun(offset_from_start_x: number, spacing: number): nil
function ImGui.SameLine() end

function ImGui.NewLine() end
function ImGui.Spacing() end

---@param size_x number Width
---@param size_y number Height
function ImGui.Dummy(size_x, size_y) end

---@overload fun(indent_w: number): nil
function ImGui.Indent() end

---@overload fun(indent_w: number): nil
function ImGui.Unindent() end

function ImGui.BeginGroup() end
function ImGui.EndGroup() end

-- Tree nodes
---@param label string Node label
---@return boolean Node is open
---@overload fun(str_id: string, fmt: string): boolean
function ImGui.TreeNode(label) end

---@param label string Node label
---@param flags number Tree node flags
---@return boolean Node is open
function ImGui.TreeNodeEx(label, flags) end

function ImGui.TreePop() end

---@param str_id string Tree ID
function ImGui.TreePush(str_id) end

---@param is_open boolean Open state
---@overload fun(is_open: boolean, cond: number): nil
function ImGui.SetNextItemOpen(is_open) end

-- Color editing
---@param label string Color picker label
---@param r number Red component
---@param g number Green component
---@param b number Blue component
---@return boolean, number, number, number Changed, new RGB
function ImGui.ColorEdit3(label, r, g, b) end

---@param label string Color picker label
---@param r number Red component
---@param g number Green component
---@param b number Blue component
---@param a number Alpha component
---@return boolean, number, number, number, number Changed, new RGBA
function ImGui.ColorEdit4(label, r, g, b, a) end

-- Item state queries
---@return boolean Item is hovered
function ImGui.IsItemHovered() end

---@return boolean Item is active
function ImGui.IsItemActive() end

---@return boolean Item is focused
function ImGui.IsItemFocused() end

---@return boolean Item was clicked
---@overload fun(mouse_button: number): boolean
function ImGui.IsItemClicked() end

---@return boolean Item is visible
function ImGui.IsItemVisible() end

---@return boolean Item was edited
function ImGui.IsItemEdited() end

-- Window state queries
---@return boolean Window is appearing
function ImGui.IsWindowAppearing() end

---@return boolean Window is collapsed
function ImGui.IsWindowCollapsed() end

---@return boolean Window is focused
---@overload fun(flags: number): boolean
function ImGui.IsWindowFocused() end

---@return boolean Window is hovered
---@overload fun(flags: number): boolean
function ImGui.IsWindowHovered() end

---@return number, number Window position x, y
function ImGui.GetWindowPos() end

---@return number, number Window size width, height
function ImGui.GetWindowSize() end

---@return number Window width
function ImGui.GetWindowWidth() end

---@return number Window height
function ImGui.GetWindowHeight() end

-- Child Windows
---@param str_id string Child ID
---@return boolean Child window is open
---@overload fun(str_id: string, size_x: number, size_y: number): boolean
---@overload fun(str_id: string, size_x: number, size_y: number, border: boolean): boolean
function ImGui.BeginChild(str_id) end

function ImGui.EndChild() end

---@param label string Selectable label
---@return boolean Selectable was clicked
---@overload fun(label: string, selected: boolean): boolean, boolean
function ImGui.Selectable(label) end

---@param id string Invisible button ID
---@param size_x number Width
---@param size_y number Height
---@return boolean Button was clicked
function ImGui.InvisibleButton(id, size_x, size_y) end

---@param str_id string Arrow button ID
---@param dir number Direction
---@return boolean Button was clicked
function ImGui.ArrowButton(str_id, dir) end

---@param user_texture number Texture ID
---@param pos Vec2 Image size
---@param invert? boolean Invert UV (default: false)
function ImGui.Image(user_texture, pos, invert) end

-- Cursor/Layout
---@return number x, number y Cursor position
function ImGui.GetCursorPos() end

---@param x number X position
---@param y number Y position
function ImGui.SetCursorPos(x, y) end

---@return number X position
function ImGui.GetCursorPosX() end

---@return number Y position
function ImGui.GetCursorPosY() end

---@param x number X position
function ImGui.SetCursorPosX(x) end

---@param y number Y position
function ImGui.SetCursorPosY(y) end

-- Scrolling
---@return number Scroll X
function ImGui.GetScrollX() end

---@return number Scroll Y
function ImGui.GetScrollY() end

---@param scroll_x number Scroll X
function ImGui.SetScrollX(scroll_x) end

---@param scroll_y number Scroll Y
function ImGui.SetScrollY(scroll_y) end

---@return number Max scroll X
function ImGui.GetScrollMaxX() end

---@return number Max scroll Y
function ImGui.GetScrollMaxY() end

---@param center_x_ratio? number Center ratio (default: 0.5)
function ImGui.SetScrollHereX(center_x_ratio) end

---@param center_y_ratio? number Center ratio (default: 0.5)
function ImGui.SetScrollHereY(center_y_ratio) end

-- Additional item state queries
---@return boolean
function ImGui.IsItemActivated() end

---@return boolean
function ImGui.IsItemDeactivated() end

---@return boolean
function ImGui.IsItemDeactivatedAfterEdit() end

---@return boolean
function ImGui.IsItemToggledOpen() end

---@return boolean
function ImGui.IsAnyItemHovered() end

---@return boolean
function ImGui.IsAnyItemActive() end

---@return boolean
function ImGui.IsAnyItemFocused() end

-- Window Flags (accessed as ImGui.WindowFlags_*)
---@type number
ImGui.WindowFlags_None = 0
---@type number
ImGui.WindowFlags_NoTitleBar = 0
---@type number
ImGui.WindowFlags_NoResize = 0
---@type number
ImGui.WindowFlags_NoMove = 0
---@type number
ImGui.WindowFlags_NoScrollbar = 0
---@type number
ImGui.WindowFlags_NoScrollWithMouse = 0
---@type number
ImGui.WindowFlags_NoCollapse = 0
---@type number
ImGui.WindowFlags_AlwaysAutoResize = 0
---@type number
ImGui.WindowFlags_NoBackground = 0
---@type number
ImGui.WindowFlags_NoSavedSettings = 0
---@type number
ImGui.WindowFlags_NoMouseInputs = 0
---@type number
ImGui.WindowFlags_MenuBar = 0
---@type number
ImGui.WindowFlags_HorizontalScrollbar = 0
---@type number
ImGui.WindowFlags_NoFocusOnAppearing = 0
---@type number
ImGui.WindowFlags_NoBringToFrontOnFocus = 0
---@type number
ImGui.WindowFlags_AlwaysVerticalScrollbar = 0
---@type number
ImGui.WindowFlags_AlwaysHorizontalScrollbar = 0
---@type number
ImGui.WindowFlags_AlwaysUseWindowPadding = 0

-- Tree Node Flags (accessed as ImGui.TreeNodeFlags_*)
---@type number
ImGui.TreeNodeFlags_None = 0
---@type number
ImGui.TreeNodeFlags_Selected = 0
---@type number
ImGui.TreeNodeFlags_Framed = 0
---@type number
ImGui.TreeNodeFlags_AllowItemOverlap = 0
---@type number
ImGui.TreeNodeFlags_NoTreePushOnOpen = 0
---@type number
ImGui.TreeNodeFlags_NoAutoOpenOnLog = 0
---@type number
ImGui.TreeNodeFlags_DefaultOpen = 0
---@type number
ImGui.TreeNodeFlags_OpenOnDoubleClick = 0
---@type number
ImGui.TreeNodeFlags_OpenOnArrow = 0
---@type number
ImGui.TreeNodeFlags_Leaf = 0
---@type number
ImGui.TreeNodeFlags_Bullet = 0
---@type number
ImGui.TreeNodeFlags_FramePadding = 0
---@type number
ImGui.TreeNodeFlags_SpanAvailWidth = 0
---@type number
ImGui.TreeNodeFlags_SpanFullWidth = 0
---@type number
ImGui.TreeNodeFlags_NavLeftJumpsBackHere = 0

-- Menu functions
---@return boolean Menu bar is active
function ImGui.BeginMenuBar() end

function ImGui.EndMenuBar() end

---@return boolean Main menu bar is active
function ImGui.BeginMainMenuBar() end

function ImGui.EndMainMenuBar() end

---@param label string Menu label
---@return boolean Menu is open
---@overload fun(label: string, enabled: boolean): boolean
function ImGui.BeginMenu(label) end

function ImGui.EndMenu() end

---@param label string Menu item label
---@return boolean Item was clicked
---@overload fun(label: string, shortcut: string): boolean
---@overload fun(label: string, shortcut: string, selected: boolean): boolean
---@overload fun(label: string, shortcut: string, selected: boolean, enabled: boolean): boolean
function ImGui.MenuItem(label) end

-- Text utilities
---@param text string Text to measure
---@return number Width in pixels
function ImGui.CalcTextSize(text) end

---@return number Available width in pixels
function ImGui.GetContentRegionAvail() end

-- Style variables
---@param idx number Style variable index (e.g., ImGui.StyleVar_WindowPadding)
---@param val number Single float value
---@overload fun(idx: number, x: number, y: number): nil
function ImGui.PushStyleVar(idx, val) end

---@param count? number Number of style vars to pop (default: 1)
function ImGui.PopStyleVar(count) end

-- Style variable constants
---@type number
ImGui.StyleVar_WindowPadding = 0
---@type number
ImGui.StyleVar_FramePadding = 0
---@type number
ImGui.StyleVar_ItemSpacing = 0
---@type number
ImGui.StyleVar_ItemInnerSpacing = 0

-- Input system (World.input)
---@class InputSystem
---@field get_mouse_position fun(): Vec2
---@field get_mouse_delta fun(): Vec2
---@field is_left_mouse_down fun(): boolean
---@field is_right_mouse_down fun(): boolean
---@field is_middle_mouse_down fun(): boolean
---@field lock_mouse fun(): nil
---@field unlock_mouse fun(): nil
---@field is_mouse_locked fun(): boolean
---@field is_key_down fun(key: number): boolean
---@field is_key_up fun(key: number): boolean
local InputSystem = {}

---@class Object
---@field transform Mat4 Object's 4x4 transformation matrix (position, rotation, scale). Returns physics body transform if physics component exists, otherwise ECS transform.
---@field parent Object Set this object's parent in the scene hierarchy. Creates parent-child relationship where this object inherits transformations.
Object = {}

---@return Object
function Object.new() end

---Add a component to this object
---@param component_type EcsComponentType Type of component to add
---@param data EcsMeshComponentData|EcsMeshTextureComponentData|EcsPhysicsComponentData Initialization data for the component
function Object:add_component(component_type, data) end

---Get a component from this object
---@param component_type EcsComponentType Type of component to get
---@return table|nil Component data as a table, or nil if component doesn't exist
function Object:get_component(component_type) end

---Patch an existing component on this object at runtime
---@param component_type EcsComponentType Type of component to patch
---@param data EcsMeshComponentData|EcsMeshTextureComponentData|EcsPhysicsComponentData Data fields to update in the component
---
---Allows runtime modification of component properties without full replacement.
---Useful for updating mesh colors, textures, or other component data dynamically.
---
---Example usage:
---```lua
----- Update mesh color at runtime
---my_object:patch_component(ECS_MESH_COMPONENT, { color = Vec4.new(1, 0, 0, 0.5) })
---
----- Update texture dynamically
---my_object:patch_component(ECS_MESH_TEXTURE_COMPONENT, { texture = new_texture })
---```
function Object:patch_component(component_type, data) end

---@class Template
---@field function_calls number Number of function calls
Template = {}

---@return Template
function Template.new() end

-- ECS Component System
---@alias EcsComponentType integer

-- Component Type Constants
ECS_MESH_COMPONENT = 0
ECS_MESH_TEXTURE_COMPONENT = 1
ECS_PHYSICS_COMPONENT = 2

-- ECS Component Data Structures
---@class EcsMeshComponentData
---@field mesh Mesh The mesh object to render
---@field color Vec4|nil Optional mesh color (R, G, B, A). Applied as uniform color multiplier in shader. If not provided, defaults to white (1, 1, 1, 1).

---@class EcsMeshTextureComponentData  
---@field texture Image The texture to apply to the mesh

---@class EcsPhysicsComponentData
---@field mesh Mesh The mesh to use for physics collision shape
---@field is_dynamic boolean Whether the physics body is dynamic (moveable) or static

-- Global functions
---@return number Current time in milliseconds
function now() end

---Print with blue [Lua] prefix
---@param ... any Values to print
function print(...) end

---Print with yellow [Lua] prefix for warnings
---@param ... any Values to print
function warn(...) end

---Create empty userdata (internal use)
function make_userdata() end

-- Temporary/Legacy Raytracer Functions (Desktop Only)
---@param position Vec3 Sphere position
---@param color Vec3 Sphere color
---@param radius number Sphere radius
---@param emissive number Emissive strength
function TEMP_make_sphere(position, color, radius, emissive) end

---Bind sphere texture buffer object
function TEMP_bind_tbo_texture() end

---@return number Sphere count
function TEMP_get_tbo_texture_count() end

-- Global variables
_G.Camera = Camera
_G.Font = Font
_G.Image = Image
_G.Mesh = Mesh
_G.Textlabel = Textlabel
_G.Signal = Signal
_G.Vec2 = Vec2
_G.Vec3 = Vec3
_G.Vec4 = Vec4
_G.Mat4 = Mat4
_G.World = World
_G.Window = Window
_G.Gizmo = Gizmo
---@class VFXEffect
---@field is_valid boolean
VFXEffect = {}

---@return VFXEffect
function VFXEffect.new() end

---@param vertex_path string Path to vertex shader
---@param fragment_path string Path to fragment shader
---@return boolean Success
function VFXEffect:load_shader(vertex_path, fragment_path) end

---@param fragment_path string Path to fragment shader
---@return boolean Success
function VFXEffect:load_fragment_shader(fragment_path) end

function VFXEffect:render() end

---@param name string Uniform name
---@param value number Float value
function VFXEffect:set_uniform_float(name, value) end

---@param name string Uniform name
---@param x number X component
---@param y number Y component
function VFXEffect:set_uniform_vec2(name, x, y) end

---@param name string Uniform name
---@param x number X component
---@param y number Y component
---@param z number Z component
function VFXEffect:set_uniform_vec3(name, x, y, z) end

---@param name string Uniform name
---@param x number X component
---@param y number Y component
---@param z number Z component
---@param w number W component
function VFXEffect:set_uniform_vec4(name, x, y, z, w) end

---@param name string Uniform name
---@param value number Integer value
function VFXEffect:set_uniform_int(name, value) end

---@param name string Uniform name
---@param image Image Texture image
function VFXEffect:set_uniform_texture(name, image) end

---@param name string Uniform name
---@param matrix Mat4 Matrix value
function VFXEffect:set_uniform_mat4(name, matrix) end

---Bind shader program for use
function VFXEffect:bind() end

---Unbind shader program
function VFXEffect:unbind() end

---@class Framebuffer
---@field texture_id number OpenGL texture ID of color attachment
Framebuffer = {}

---@param width number Framebuffer width
---@param height number Framebuffer height
---@return Framebuffer
function Framebuffer.new(width, height) end

---Bind framebuffer for rendering
function Framebuffer:bind() end

---Bind framebuffer and execute callback, automatically unbinding afterward
---@param callback function Function to execute while framebuffer is bound
---
---This method provides automatic framebuffer management by binding the framebuffer,
---executing the provided callback function, and then automatically unbinding.
---Viewport dimensions are properly set and restored.
---
---Example usage:
---```lua
---scene_buffer:bind(function()
---    -- Render scene to framebuffer
---    World.rendering.render_pass()
---    display_label:render()
---end)
---```
---@overload fun(self: Framebuffer, callback: function)
function Framebuffer:bind(callback) end

---Restore default framebuffer
function Framebuffer:unbind() end

---Clear framebuffer contents
function Framebuffer:clear() end

---@param new_size Vec2 New dimensions
function Framebuffer:resize(new_size) end

---@param texture_slot number Texture slot to bind to
function Framebuffer:bind_texture(texture_slot) end

---@class MeshBufferObject
---@field matrix Mat4 Transformation matrix
---@field color Vec3 Mesh color
---@field emissive number Emissive strength
---@field triangles number Triangle count
---@field mesh bsgeMesh Mesh geometry data
MeshBufferObject = {}

---@return MeshBufferObject
function MeshBufferObject.new() end

---@return MeshBufferObject Register mesh for raytracing
function MeshBufferObject:register() end

---Update mesh data in GPU buffers
function MeshBufferObject:update() end

---@return BoundingBox Axis-aligned bounding box
function MeshBufferObject:get_bounding_box() end

---@return number Mesh count
function MeshBufferObject.get_count() end

---@param triangle_texture_slot number Triangle texture slot
---@param mesh_texture_slot number Mesh texture slot
function MeshBufferObject.bind_textures(triangle_texture_slot, mesh_texture_slot) end

---@class SphereBufferObject
---@field center Vec3 Sphere center position
---@field radius number Sphere radius
---@field color Vec3 Sphere color
---@field emissive number Emissive strength
SphereBufferObject = {}

---@return SphereBufferObject
function SphereBufferObject.new() end

---@return SphereBufferObject Register sphere for raytracing
function SphereBufferObject:register() end

---Update sphere data in GPU buffers
function SphereBufferObject:update() end

---@return number Sphere count
function SphereBufferObject.get_count() end

---@param texture_slot number Texture slot
function SphereBufferObject.bind_textures(texture_slot) end

---@class BoundingBox
---@field world_min Vec3 Minimum bounds
---@field world_max Vec3 Maximum bounds
BoundingBox = {}

-- Key Constants
KEY_A = 65
KEY_B = 66
KEY_C = 67
KEY_D = 68
KEY_E = 69
KEY_F = 70
KEY_G = 71
KEY_H = 72
KEY_I = 73
KEY_J = 74
KEY_K = 75
KEY_L = 76
KEY_M = 77
KEY_N = 78
KEY_O = 79
KEY_P = 80
KEY_Q = 81
KEY_R = 82
KEY_S = 83
KEY_T = 84
KEY_U = 85
KEY_V = 86
KEY_W = 87
KEY_X = 88
KEY_Y = 89
KEY_Z = 90

KEY_0 = 48
KEY_1 = 49
KEY_2 = 50
KEY_3 = 51
KEY_4 = 52
KEY_5 = 53
KEY_6 = 54
KEY_7 = 55
KEY_8 = 56
KEY_9 = 57

KEY_SPACE = 32
KEY_ENTER = 257
KEY_TAB = 258
KEY_BACKSPACE = 259
KEY_DELETE = 261
KEY_LEFT = 263
KEY_RIGHT = 262
KEY_UP = 265
KEY_DOWN = 264
KEY_PAGE_UP = 266
KEY_PAGE_DOWN = 267
KEY_HOME = 268
KEY_END = 269
KEY_INSERT = 260
KEY_LEFT_SHIFT = 340
KEY_RIGHT_SHIFT = 344
KEY_LEFT_CONTROL = 341
KEY_RIGHT_CONTROL = 345
KEY_LEFT_ALT = 342
KEY_RIGHT_ALT = 346
KEY_ESCAPE = 256

KEY_F1 = 290
KEY_F2 = 291
KEY_F3 = 292
KEY_F4 = 293
KEY_F5 = 294
KEY_F6 = 295
KEY_F7 = 296
KEY_F8 = 297
KEY_F9 = 298
KEY_F10 = 299
KEY_F11 = 300
KEY_F12 = 301

-- OpenGL Texture Constants
GL_TEXTURE0 = 33984
GL_TEXTURE1 = 33985
GL_TEXTURE2 = 33986
GL_TEXTURE3 = 33987
GL_TEXTURE4 = 33988
GL_TEXTURE5 = 33989
GL_TEXTURE6 = 33990
GL_TEXTURE7 = 33991
GL_TEXTURE8 = 33992
GL_TEXTURE9 = 33993
GL_TEXTURE10 = 33994
GL_TEXTURE11 = 33995
GL_TEXTURE12 = 33996
GL_TEXTURE13 = 33997
GL_TEXTURE14 = 33998
GL_TEXTURE15 = 33999

-- OpenGL Constants
GL_VERTEX_SHADER = 35633
GL_FRAGMENT_SHADER = 35632
GL_GEOMETRY_SHADER = 36313
GL_RGB = 6407
GL_RGBA = 6408
GL_TEXTURE_2D = 3553
GL_LINEAR = 9729
GL_NEAREST = 9728
GL_REPEAT = 10497
GL_CLAMP_TO_EDGE = 33071
GL_DEPTH_TEST = 2929
GL_BLEND = 3042

-- Additional Global Constants
BASE_FONT_HEIGHT = 96

---@class Emscripten
Emscripten = {}

---Download and load an image from a URL (Web platform only)
---@param src string URL of the image to download
---@return Image Downloaded image object
---
---This function is only available when running on the web platform (PLATFORM == "WEB").
---It downloads an image from the specified URL and returns an Image object that can be used
---for texturing meshes or other rendering operations.
---
---Example usage:
---```lua
---local backdrop = PLATFORM == "NATIVE" and Image.new("image/texture.jpg")
---    or Emscripten.download_image("https://example.com/texture.jpg")
---```
function Emscripten.download_image(src) end

---Get an integer value from the JavaScript-Lua communication bridge (Web platform only)
---@param key string The key to retrieve the integer value for
---@return number The integer value associated with the key, or 0 if the key doesn't exist
---
---This function provides cross-platform communication between JavaScript and Lua when running
---on the web platform via WebAssembly. It retrieves integer values that have been set by
---JavaScript code using the corresponding js_set_int() function from the browser side.
---
---The function accesses a global string-to-integer map that persists throughout the application's
---runtime. Values are not persistent between application restarts.
---
---Common use cases:
---- Reading configuration values set by web UI
---- Polling for state changes from JavaScript events
---- Receiving user input or selections from DOM elements
---- Cross-platform communication in web deployments
---
---Example usage:
---```lua
----- Poll for changes from JavaScript
---if Emscripten.get_int("selected") ~= last_selected then
---    last_selected = Emscripten.get_int("selected")
---    handle_selection_change()
---end
---
----- Display value in ImGui
---ImGui.Text(string.format("Current selection: %i", Emscripten.get_int("selected")))
---
----- Check for web-based configuration
---local quality_setting = Emscripten.get_int("graphics_quality")
---```
---
---Platform compatibility:
--- - WEB: Fully functional, enables JavaScript-Lua communication
--- - NATIVE: Function exists but returns 0 for all keys (no-op)
function Emscripten.get_int(key) end

-- Platform detection constant
---@type string "WEB" | "NATIVE"
---Global constant indicating the current platform:
--- "WEB" - Running in browser via Emscripten/WebAssembly
--- "NATIVE" - Running as native executable (Windows/Linux)
---
---Use this to conditionally load assets or enable platform-specific features:
---```lua
---local image = PLATFORM == "NATIVE" 
---    and Image.new("local/path.jpg")
---    or Emscripten.download_image("url/path.jpg")
---```
PLATFORM = "NATIVE"

-- Scene root object
---@type Object
---Global scene root object - the top-level parent in the ECS hierarchy.
---All objects without an explicit parent are children of Scene.
---
---This is a BSGEObject instance that serves as the default parent for all scene objects.
---Use it to organize your scene hierarchy:
---```lua
---local my_object = Object.new()
---my_object.parent = Scene  -- Parent to scene root
---
---local mesh_data = {
---    path = "mesh/cube.obj"
---}
---my_object:add_component(ECS_MESH_COMPONENT, mesh_data)
---```
Scene = {}

-- ScrollingBuffer - circular buffer for realtime scrolling plots
---@class ScrollingBuffer
---@field MaxSize number Maximum number of points
---@field Offset number Current write offset in circular buffer
ScrollingBuffer = {}

---@param max_size? number Maximum buffer size (default: 2000)
---@return ScrollingBuffer
function ScrollingBuffer.new(max_size) end

---@param x number X value
---@param y number Y value
function ScrollingBuffer:AddPoint(x, y) end

function ScrollingBuffer:Erase() end

---@return number count Number of points in buffer
function ScrollingBuffer:Size() end

-- RollingBuffer - rolling buffer that wraps around a time span
---@class RollingBuffer
---@field Span number Time span before wrapping
RollingBuffer = {}

---@param span? number Time span (default: 10.0)
---@return RollingBuffer
function RollingBuffer.new(span) end

---@param x number X value
---@param y number Y value
function RollingBuffer:AddPoint(x, y) end

function RollingBuffer:Erase() end

---@return number count Number of points in buffer
function RollingBuffer:Size() end

-- ImPlot
---@class ImPlot
ImPlot = {}

-- Begin/End Plot
---@param title_id string Plot title (use ## prefix to hide)
---@return boolean should_render True if plot is visible
---@overload fun(title_id: string, w: number, h: number): boolean
---@overload fun(title_id: string, w: number, h: number, flags: number): boolean
function ImPlot.BeginPlot(title_id) end

function ImPlot.EndPlot() end

-- Begin/End Subplots
---@param title_id string Subplot title
---@param rows number Number of rows
---@param cols number Number of columns
---@param w number Width
---@param h number Height
---@return boolean should_render
---@overload fun(title_id: string, rows: number, cols: number, w: number, h: number, flags: number): boolean
function ImPlot.BeginSubplots(title_id, rows, cols, w, h) end

function ImPlot.EndSubplots() end

-- Setup (call after BeginPlot, before plotting)
---@param axis number Axis index (ImPlot.Axis_X1, etc.)
---@param label? string Axis label
---@param flags? number ImPlotAxisFlags
function ImPlot.SetupAxis(axis, label, flags) end

---@param axis number Axis index
---@param v_min number Minimum value
---@param v_max number Maximum value
---@param cond? number ImPlotCond (default: Cond_Once)
function ImPlot.SetupAxisLimits(axis, v_min, v_max, cond) end

---@param axis number Axis index
---@param fmt string Format string (e.g. "%g")
function ImPlot.SetupAxisFormat(axis, fmt) end

---@param axis number Axis index
---@param scale number ImPlotScale value
function ImPlot.SetupAxisScale(axis, scale) end

---@param axis number Axis index
---@param v_min number Minimum constraint
---@param v_max number Maximum constraint
function ImPlot.SetupAxisLimitsConstraints(axis, v_min, v_max) end

---@param axis number Axis index
---@param z_min number Minimum zoom
---@param z_max number Maximum zoom
function ImPlot.SetupAxisZoomConstraints(axis, z_min, z_max) end

---@param x_label string X axis label
---@param y_label string Y axis label
---@param x_flags? number X axis flags
---@param y_flags? number Y axis flags
function ImPlot.SetupAxes(x_label, y_label, x_flags, y_flags) end

---@param x_min number
---@param x_max number
---@param y_min number
---@param y_max number
---@param cond? number ImPlotCond
function ImPlot.SetupAxesLimits(x_min, x_max, y_min, y_max, cond) end

---@param location number ImPlotLocation
---@param flags? number ImPlotLegendFlags
function ImPlot.SetupLegend(location, flags) end

---@param location number ImPlotLocation
---@param flags? number ImPlotMouseTextFlags
function ImPlot.SetupMouseText(location, flags) end

function ImPlot.SetupFinish() end

-- SetNext (call before BeginPlot)
---@param axis number Axis index
---@param v_min number Minimum value
---@param v_max number Maximum value
---@param cond? number ImPlotCond
function ImPlot.SetNextAxisLimits(axis, v_min, v_max, cond) end

---@param axis number Axis index
function ImPlot.SetNextAxisToFit(axis) end

---@param x_min number
---@param x_max number
---@param y_min number
---@param y_max number
---@param cond? number ImPlotCond
function ImPlot.SetNextAxesLimits(x_min, x_max, y_min, y_max, cond) end

function ImPlot.SetNextAxesToFit() end

-- Plot Items (table-based)
---@param label_id string Legend label
---@param values number[] Y values (auto X from 0..n)
---@overload fun(label_id: string, values: number[], xscale: number, xstart: number)
---@overload fun(label_id: string, xs: number[], ys: number[])
function ImPlot.PlotLine(label_id, values) end

---@param label_id string Legend label
---@param values number[] Y values
---@overload fun(label_id: string, values: number[], xscale: number, xstart: number)
---@overload fun(label_id: string, xs: number[], ys: number[])
function ImPlot.PlotScatter(label_id, values) end

---@param label_id string Legend label
---@param values number[] Y values
---@overload fun(label_id: string, xs: number[], ys: number[])
function ImPlot.PlotStairs(label_id, values) end

---@param label_id string Legend label
---@param values number[] Y values (shaded to y=0)
---@overload fun(label_id: string, values: number[], yref: number)
---@overload fun(label_id: string, xs: number[], ys: number[])
function ImPlot.PlotShaded(label_id, values) end

---@param label_id string Legend label
---@param values number[] Bar values
---@overload fun(label_id: string, values: number[], bar_size: number)
---@overload fun(label_id: string, xs: number[], ys: number[], bar_size: number)
function ImPlot.PlotBars(label_id, values) end

---@param label_id string Legend label
---@param xs number[] X positions
---@param ys number[] Y values
---@param err number[] Symmetric error values
---@overload fun(label_id: string, xs: number[], ys: number[], neg: number[], pos: number[])
function ImPlot.PlotErrorBars(label_id, xs, ys, err) end

---@param label_id string Legend label
---@param values number[] Stem values
---@overload fun(label_id: string, xs: number[], ys: number[])
function ImPlot.PlotStems(label_id, values) end

---@param label_id string Legend label
---@param values number[] Line positions
function ImPlot.PlotInfLines(label_id, values) end

---@param label_ids string[] Slice labels
---@param values number[] Slice values
---@param x number Center X
---@param y number Center Y
---@param radius number Pie radius
---@param label_fmt? string Label format (default: "%.1f")
function ImPlot.PlotPieChart(label_ids, values, x, y, radius, label_fmt) end

---@param label_id string Legend label
---@param values number[] Flat row-major data
---@param rows number Number of rows
---@param cols number Number of columns
---@param scale_min? number Min color scale (0 = auto)
---@param scale_max? number Max color scale (0 = auto)
---@param label_fmt? string Cell label format
function ImPlot.PlotHeatmap(label_id, values, rows, cols, scale_min, scale_max, label_fmt) end

---@param label_id string Legend label
---@param values number[] Data values
---@param bins? number Bin count or ImPlotBin method
---@return number max_count Largest bin count
function ImPlot.PlotHistogram(label_id, values, bins) end

---@param label_id string Legend label
---@param xs number[] X values
---@param ys number[] Y values (0 or 1)
function ImPlot.PlotDigital(label_id, xs, ys) end

---@param text string Text to display
---@param x number X position
---@param y number Y position
---@overload fun(text: string, x: number, y: number, pix_offset_x: number, pix_offset_y: number)
function ImPlot.PlotText(text, x, y) end

---@param label_id string Legend label
function ImPlot.PlotDummy(label_id) end

-- Buffer-based plot functions (zero-copy, handles circular offset)
---@param label_id string Legend label
---@param buf ScrollingBuffer|RollingBuffer Data buffer
function ImPlot.PlotLineBuffer(label_id, buf) end

---@param label_id string Legend label
---@param buf ScrollingBuffer|RollingBuffer Data buffer
---@overload fun(label_id: string, buf: ScrollingBuffer, yref: number)
function ImPlot.PlotShadedBuffer(label_id, buf) end

---@param label_id string Legend label
---@param buf ScrollingBuffer|RollingBuffer Data buffer
function ImPlot.PlotScatterBuffer(label_id, buf) end

-- Plot Tools
---@param id number Unique drag line ID
---@param x number X position
---@param r number Color red
---@param g number Color green
---@param b number Color blue
---@param a number Color alpha
---@return boolean changed, number x
function ImPlot.DragLineX(id, x, r, g, b, a) end

---@param id number Unique drag line ID
---@param y number Y position
---@param r number Color red
---@param g number Color green
---@param b number Color blue
---@param a number Color alpha
---@return boolean changed, number y
function ImPlot.DragLineY(id, y, r, g, b, a) end

---@param id number Unique drag point ID
---@param x number X position
---@param y number Y position
---@param r number Color red
---@param g number Color green
---@param b number Color blue
---@param a number Color alpha
---@return boolean changed, number x, number y
function ImPlot.DragPoint(id, x, y, r, g, b, a) end

---@param x number X position
---@param y number Y position
---@param r number Color red
---@param g number Color green
---@param b number Color blue
---@param a number Color alpha
---@param pix_x number Pixel offset X
---@param pix_y number Pixel offset Y
---@param clamp boolean Clamp to plot area
---@param fmt? string Text format string
function ImPlot.Annotation(x, y, r, g, b, a, pix_x, pix_y, clamp, fmt) end

---@param x number X position
---@param r number Color red
---@param g number Color green
---@param b number Color blue
---@param a number Color alpha
---@param fmt? string Tag text
function ImPlot.TagX(x, r, g, b, a, fmt) end

---@param y number Y position
---@param r number Color red
---@param g number Color green
---@param b number Color blue
---@param a number Color alpha
---@param fmt? string Tag text
function ImPlot.TagY(y, r, g, b, a, fmt) end

-- Plot Utils
---@param axis number Axis index
function ImPlot.SetAxis(axis) end

---@param x_axis number X axis index
---@param y_axis number Y axis index
function ImPlot.SetAxes(x_axis, y_axis) end

---@param x number Pixel X
---@param y number Pixel Y
---@return number plot_x, number plot_y
function ImPlot.PixelsToPlot(x, y) end

---@param x number Plot X
---@param y number Plot Y
---@return number pixel_x, number pixel_y
function ImPlot.PlotToPixels(x, y) end

---@return number x, number y Plot position (top-left) in pixels
function ImPlot.GetPlotPos() end

---@return number w, number h Plot size in pixels
function ImPlot.GetPlotSize() end

---@return number x, number y Mouse position in plot coordinates
function ImPlot.GetPlotMousePos() end

---@return boolean
function ImPlot.IsPlotHovered() end

---@param axis number Axis index
---@return boolean
function ImPlot.IsAxisHovered(axis) end

---@return boolean
function ImPlot.IsSubplotsHovered() end

---@return boolean
function ImPlot.IsPlotSelected() end

function ImPlot.CancelPlotSelection() end

---@param hidden? boolean
---@param cond? number ImPlotCond
function ImPlot.HideNextItem(hidden, cond) end

-- Legend Utils
---@param label_id string Legend entry label
---@param mouse_button? number Mouse button (default: 1/right)
---@return boolean
function ImPlot.BeginLegendPopup(label_id, mouse_button) end

function ImPlot.EndLegendPopup() end

---@param label_id string Legend entry label
---@return boolean
function ImPlot.IsLegendEntryHovered(label_id) end

-- Styling
function ImPlot.StyleColorsAuto() end
function ImPlot.StyleColorsClassic() end
function ImPlot.StyleColorsDark() end
function ImPlot.StyleColorsLight() end

---@param idx number ImPlotCol index
---@param r number Red
---@param g number Green
---@param b number Blue
---@param a number Alpha
function ImPlot.PushStyleColor(idx, r, g, b, a) end

---@param count? number Number of colors to pop (default: 1)
function ImPlot.PopStyleColor(count) end

---@param idx number ImPlotStyleVar index
---@param val number Float value
function ImPlot.PushStyleVarFloat(idx, val) end

---@param idx number ImPlotStyleVar index
---@param val number Int value
function ImPlot.PushStyleVarInt(idx, val) end

---@param idx number ImPlotStyleVar index
---@param x number X value
---@param y number Y value
function ImPlot.PushStyleVarVec2(idx, x, y) end

---@param count? number Number of vars to pop (default: 1)
function ImPlot.PopStyleVar(count) end

---@return number r, number g, number b, number a
function ImPlot.GetLastItemColor() end

-- Colormaps
---@param cmap number|string Colormap index or name
function ImPlot.PushColormap(cmap) end

---@param count? number Number to pop (default: 1)
function ImPlot.PopColormap(count) end

---@return number r, number g, number b, number a
function ImPlot.NextColormapColor() end

---@return number
function ImPlot.GetColormapCount() end

---@param cmap number Colormap index
---@return string
function ImPlot.GetColormapName(cmap) end

---@param name string Colormap name
---@return number index (-1 if invalid)
function ImPlot.GetColormapIndex(name) end

---@param cmap? number Colormap index (default: current)
---@return number
function ImPlot.GetColormapSize(cmap) end

---@param idx number Color index
---@param cmap? number Colormap index
---@return number r, number g, number b, number a
function ImPlot.GetColormapColor(idx, cmap) end

---@param t number Value in [0, 1]
---@param cmap? number Colormap index
---@return number r, number g, number b, number a
function ImPlot.SampleColormap(t, cmap) end

---@param label string Scale label
---@param scale_min number Min scale value
---@param scale_max number Max scale value
---@param w? number Width
---@param h? number Height
function ImPlot.ColormapScale(label, scale_min, scale_max, w, h) end

---@param label string Button label
---@param w? number Width
---@param h? number Height
---@return boolean
function ImPlot.ColormapButton(label, w, h) end

---@param plot_title_id? string Plot to bust (nil = all)
function ImPlot.BustColorCache(plot_title_id) end

function ImPlot.ShowDemoWindow() end

-- Axis constants
---@type number
ImPlot.Axis_X1 = 0
---@type number
ImPlot.Axis_X2 = 1
---@type number
ImPlot.Axis_X3 = 2
---@type number
ImPlot.Axis_Y1 = 3
---@type number
ImPlot.Axis_Y2 = 4
---@type number
ImPlot.Axis_Y3 = 5

-- Plot flags
---@type number
ImPlot.Flags_None = 0
---@type number
ImPlot.Flags_NoTitle = 0
---@type number
ImPlot.Flags_NoLegend = 0
---@type number
ImPlot.Flags_NoMouseText = 0
---@type number
ImPlot.Flags_NoInputs = 0
---@type number
ImPlot.Flags_NoMenus = 0
---@type number
ImPlot.Flags_NoBoxSelect = 0
---@type number
ImPlot.Flags_NoFrame = 0
---@type number
ImPlot.Flags_Equal = 0
---@type number
ImPlot.Flags_Crosshairs = 0
---@type number
ImPlot.Flags_CanvasOnly = 0

-- Axis flags
---@type number
ImPlot.AxisFlags_None = 0
---@type number
ImPlot.AxisFlags_NoLabel = 0
---@type number
ImPlot.AxisFlags_NoGridLines = 0
---@type number
ImPlot.AxisFlags_NoTickMarks = 0
---@type number
ImPlot.AxisFlags_NoTickLabels = 0
---@type number
ImPlot.AxisFlags_NoInitialFit = 0
---@type number
ImPlot.AxisFlags_NoMenus = 0
---@type number
ImPlot.AxisFlags_Opposite = 0
---@type number
ImPlot.AxisFlags_Invert = 0
---@type number
ImPlot.AxisFlags_AutoFit = 0
---@type number
ImPlot.AxisFlags_RangeFit = 0
---@type number
ImPlot.AxisFlags_LockMin = 0
---@type number
ImPlot.AxisFlags_LockMax = 0
---@type number
ImPlot.AxisFlags_Lock = 0
---@type number
ImPlot.AxisFlags_NoDecorations = 0
---@type number
ImPlot.AxisFlags_AuxDefault = 0

-- Legend flags
---@type number
ImPlot.LegendFlags_None = 0
---@type number
ImPlot.LegendFlags_NoButtons = 0
---@type number
ImPlot.LegendFlags_Outside = 0
---@type number
ImPlot.LegendFlags_Horizontal = 0
---@type number
ImPlot.LegendFlags_Sort = 0

-- Line flags
---@type number
ImPlot.LineFlags_None = 0
---@type number
ImPlot.LineFlags_Segments = 0
---@type number
ImPlot.LineFlags_Loop = 0
---@type number
ImPlot.LineFlags_SkipNaN = 0
---@type number
ImPlot.LineFlags_Shaded = 0

-- Bars flags
---@type number
ImPlot.BarsFlags_None = 0
---@type number
ImPlot.BarsFlags_Horizontal = 0

-- Location constants
---@type number
ImPlot.Location_Center = 0
---@type number
ImPlot.Location_North = 0
---@type number
ImPlot.Location_South = 0
---@type number
ImPlot.Location_West = 0
---@type number
ImPlot.Location_East = 0
---@type number
ImPlot.Location_NorthWest = 0
---@type number
ImPlot.Location_NorthEast = 0
---@type number
ImPlot.Location_SouthWest = 0
---@type number
ImPlot.Location_SouthEast = 0

-- Condition constants
---@type number
ImPlot.Cond_None = 0
---@type number
ImPlot.Cond_Always = 0
---@type number
ImPlot.Cond_Once = 0

-- Marker constants
---@type number
ImPlot.Marker_None = 0
---@type number
ImPlot.Marker_Auto = 0
---@type number
ImPlot.Marker_Circle = 0
---@type number
ImPlot.Marker_Square = 0
---@type number
ImPlot.Marker_Diamond = 0
---@type number
ImPlot.Marker_Up = 0
---@type number
ImPlot.Marker_Down = 0
---@type number
ImPlot.Marker_Left = 0
---@type number
ImPlot.Marker_Right = 0
---@type number
ImPlot.Marker_Cross = 0
---@type number
ImPlot.Marker_Plus = 0
---@type number
ImPlot.Marker_Asterisk = 0

-- Scale constants
---@type number
ImPlot.Scale_Linear = 0
---@type number
ImPlot.Scale_Time = 0
---@type number
ImPlot.Scale_Log10 = 0
---@type number
ImPlot.Scale_SymLog = 0

-- Colormap constants
---@type number
ImPlot.Colormap_Deep = 0
---@type number
ImPlot.Colormap_Dark = 0
---@type number
ImPlot.Colormap_Pastel = 0
---@type number
ImPlot.Colormap_Paired = 0
---@type number
ImPlot.Colormap_Viridis = 0
---@type number
ImPlot.Colormap_Plasma = 0
---@type number
ImPlot.Colormap_Hot = 0
---@type number
ImPlot.Colormap_Cool = 0
---@type number
ImPlot.Colormap_Pink = 0
---@type number
ImPlot.Colormap_Jet = 0
---@type number
ImPlot.Colormap_Twilight = 0
---@type number
ImPlot.Colormap_RdBu = 0
---@type number
ImPlot.Colormap_BrBG = 0
---@type number
ImPlot.Colormap_PiYG = 0
---@type number
ImPlot.Colormap_Spectral = 0
---@type number
ImPlot.Colormap_Greys = 0

-- Style color constants
---@type number
ImPlot.Col_FrameBg = 0
---@type number
ImPlot.Col_PlotBg = 0
---@type number
ImPlot.Col_PlotBorder = 0
---@type number
ImPlot.Col_LegendBg = 0
---@type number
ImPlot.Col_LegendBorder = 0
---@type number
ImPlot.Col_LegendText = 0
---@type number
ImPlot.Col_TitleText = 0
---@type number
ImPlot.Col_InlayText = 0
---@type number
ImPlot.Col_AxisText = 0
---@type number
ImPlot.Col_AxisGrid = 0
---@type number
ImPlot.Col_AxisTick = 0
---@type number
ImPlot.Col_AxisBg = 0
---@type number
ImPlot.Col_AxisBgHovered = 0
---@type number
ImPlot.Col_AxisBgActive = 0
---@type number
ImPlot.Col_Selection = 0
---@type number
ImPlot.Col_Crosshairs = 0

-- Histogram bin method constants
---@type number
ImPlot.Bin_Sqrt = 0
---@type number
ImPlot.Bin_Sturges = 0
---@type number
ImPlot.Bin_Rice = 0
---@type number
ImPlot.Bin_Scott = 0

-- Global variables
_G.Camera = Camera
_G.Font = Font
_G.Image = Image
_G.Mesh = Mesh
_G.Shader = Shader
_G.PhysicsObject = PhysicsObject
_G.Textlabel = Textlabel
_G.Signal = Signal
_G.Vec2 = Vec2
_G.Vec3 = Vec3
_G.Vec4 = Vec4
_G.Mat4 = Mat4
_G.World = World
_G.Window = Window
_G.Gizmo = Gizmo
_G.ImGui = ImGui
_G.VFXEffect = VFXEffect
_G.Framebuffer = Framebuffer
_G.MeshBufferObject = MeshBufferObject
_G.SphereBufferObject = SphereBufferObject
_G.BoundingBox = BoundingBox
_G.Object = Object
_G.Template = Template
_G.Emscripten = Emscripten
_G.PLATFORM = PLATFORM
_G.Scene = Scene
_G.ECS_MESH_COMPONENT = ECS_MESH_COMPONENT
_G.ECS_MESH_TEXTURE_COMPONENT = ECS_MESH_TEXTURE_COMPONENT
_G.ECS_PHYSICS_COMPONENT = ECS_PHYSICS_COMPONENT
_G.ImPlot = ImPlot
_G.ScrollingBuffer = ScrollingBuffer
_G.RollingBuffer = RollingBuffer
_G.now = now
