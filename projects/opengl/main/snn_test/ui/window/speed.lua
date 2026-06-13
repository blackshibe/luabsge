local speed = {}

speed.name = "Speed"

function speed.update()
	local changed, new_speed = ImGui.SliderFloat("Speed", SPEED, 0.0, 10.0)
	if changed then
		SPEED = new_speed
	end

	if ImGui.Button("Pause") then
		SPEED = 0
	end
	ImGui.SameLine()
	if ImGui.Button("1x") then
		SPEED = 1
	end
	ImGui.SameLine()
	if ImGui.Button("2x") then
		SPEED = 2
	end
	ImGui.SameLine()
	if ImGui.Button("5x") then
		SPEED = 5
	end
end

return speed
