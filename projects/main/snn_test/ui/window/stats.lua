local plot_inputs_for_program = require("ui.plot_inputs_for_program").plot_inputs_for_program

local stats = {}

stats.name = "Stats"

function stats.update()
    ImGui.Text("Stats?")
end


return stats