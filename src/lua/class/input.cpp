#include "input.h"
#include <GLFW/glfw3.h>

// Global mouse state
static glm::vec2 mouse_position{0.0f};
static glm::vec2 mouse_delta{0.0f};
static glm::vec2 previous_mouse_position{0.0f};
static bool left_button_down = false;
static bool right_button_down = false;
static bool middle_button_down = false;
static bool cursor_locked = false;
static bool first_mouse = true;
static GLFWwindow* input_window = nullptr;

glm::vec2 get_mouse_position() {
	double xpos, ypos;

	glfwGetCursorPos(input_window, &xpos, &ypos);

	return glm::vec2(xpos, ypos);
}

void update_mouse_input() {
	if (!input_window) return;
	
	// Update previous position
	previous_mouse_position = mouse_position;
	
	// Get current mouse position
	double xpos, ypos;
	glfwGetCursorPos(input_window, &xpos, &ypos);
	mouse_position = glm::vec2(xpos, ypos);
	
	// Calculate delta
	if (first_mouse) {
		previous_mouse_position = mouse_position;
		first_mouse = false;
	}
	mouse_delta = mouse_position - previous_mouse_position;
	
	// Update button states
	left_button_down = (glfwGetMouseButton(input_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
	right_button_down = (glfwGetMouseButton(input_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
	middle_button_down = (glfwGetMouseButton(input_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
}

glm::vec2 get_mouse_delta() {
	return mouse_delta;
}

bool is_left_mouse_down() {
	return left_button_down;
}

bool is_right_mouse_down() {
	return right_button_down;
}

bool is_middle_mouse_down() {
	return middle_button_down;
}

void lock_mouse() {
	if (input_window && !cursor_locked) {
		glfwSetInputMode(input_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		cursor_locked = true;
		first_mouse = true; // Reset delta calculation
	}
}

void unlock_mouse() {
	if (input_window && cursor_locked) {
		glfwSetInputMode(input_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		cursor_locked = false;
		first_mouse = true; // Reset delta calculation
	}
}

bool is_mouse_locked() {
	return cursor_locked;
}

bool is_key_down(int key) {
	if (!input_window) return false;
	return glfwGetKey(input_window, key) == GLFW_PRESS;
}

bool is_key_up(int key) {
	if (!input_window) return false;
	return glfwGetKey(input_window, key) == GLFW_RELEASE;
}

void set_input_window(GLFWwindow* window) {
	input_window = window;
}

void lua_bsge_init_input(sol::state &lua) {
	sol::table world = lua["World"];
	sol::table input = lua.create_named_table("input");

	input["get_mouse_position"] = &get_mouse_position;
    input["get_mouse_delta"] = &get_mouse_delta;
    input["is_left_mouse_down"] = &is_left_mouse_down;
    input["is_right_mouse_down"] = &is_right_mouse_down;
    input["is_middle_mouse_down"] = &is_middle_mouse_down;
    input["lock_mouse"] = &lock_mouse;
    input["unlock_mouse"] = &unlock_mouse;
    input["is_mouse_locked"] = &is_mouse_locked;
    input["is_key_down"] = &is_key_down;
    input["is_key_up"] = &is_key_up;

	world.set("input", input);
}
