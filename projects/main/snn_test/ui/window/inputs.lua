local plot_inputs_for_program = require("ui.plot_inputs_for_program").plot_inputs_for_program

local inputs = {}

inputs.name = "Inputs"

function inputs.update()
    plot_inputs_for_program(SNN_PROGRAM)
end


return inputs