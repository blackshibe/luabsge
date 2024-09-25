---@diagnostic disable: undefined-global

local font = Font.new()
font:load("font/FiraCode-Regular.ttf")

Template.new()
Template.function_calls = 1

print(Template.function_calls)