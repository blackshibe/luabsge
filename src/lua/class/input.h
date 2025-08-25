#pragma once

#include <sol/sol.hpp>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

void update_mouse_input();
bool is_left_mouse_down();
bool is_right_mouse_down();
bool is_middle_mouse_down();
void lock_mouse();
void unlock_mouse();
bool is_mouse_locked();

glm::vec2 get_mouse_delta();
glm::vec2 get_mouse_position();

bool is_key_down(int key);
bool is_key_up(int key);

void set_input_window(GLFWwindow* window);

void lua_bsge_init_input(sol::state &lua);