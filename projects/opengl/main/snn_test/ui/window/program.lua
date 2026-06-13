local program = {}

program.name = "Program"

function program.update()
	ImGui.Text("Program: " .. (SNN_PROGRAM.name or "Unknown"))
	ImGui.Separator()

	if SNN_PROGRAM.imgui_update then
		SNN_PROGRAM.imgui_update()
	else
		ImGui.TextDisabled("No controls available")
	end
end

return program
