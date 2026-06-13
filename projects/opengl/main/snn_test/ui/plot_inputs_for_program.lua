local buffers = {}
local separate_graphs = false

local function update_inputs_for_program(program)
	buffers[program.name] = buffers[program.name] or {}
	for i, input in pairs(program.inputs) do
		buffers[program.name][input.label] = buffers[program.name][input.label] or ScrollingBuffer.new()
		buffers[program.name][input.label]:AddPoint(NETWORK_RUNTIME_SECONDS, input.current)
	end
end

local function plot_inputs_for_program(program)
	local _, new_separate = ImGui.Checkbox("Separate Input Graphs", separate_graphs)
	separate_graphs = new_separate

	if not buffers[program.name] then
		return
	end

	if separate_graphs then
		local count = 0
		for i, input in pairs(program.inputs) do
			if buffers[program.name][input.label] then
				if count % 4 ~= 0 then
					ImGui.SameLine()
				end
				count = count + 1
				if ImPlot.BeginPlot(input.label, 300, 200) then
					ImPlot.SetupAxes("Time (ms)", "Activation")
					ImPlot.SetupAxisLimits(
						ImPlot.Axis_X1,
						NETWORK_RUNTIME_SECONDS - GRAPH_HISTORY_SECONDS,
						NETWORK_RUNTIME_SECONDS,
						ImPlot.Cond_Always
					)
					ImPlot.SetupAxisLimits(ImPlot.Axis_Y1, -0.1, 1.1, ImPlot.Cond_Once)
					ImPlot.PlotLineBuffer(input.label, buffers[program.name][input.label])
					ImPlot.EndPlot()
				end
			end
		end
	else
		if ImPlot.BeginPlot(program.name, -1, 300) then
			ImPlot.SetupAxes("Time (ms)", "Activation")
			ImPlot.SetupAxisLimits(
				ImPlot.Axis_X1,
				NETWORK_RUNTIME_SECONDS - GRAPH_HISTORY_SECONDS,
				NETWORK_RUNTIME_SECONDS,
				ImPlot.Cond_Always
			)
			ImPlot.SetupAxisLimits(ImPlot.Axis_Y1, -0.1, 1.1, ImPlot.Cond_Once)

			for i, input in pairs(program.inputs) do
				if buffers[program.name][input.label] then
					ImPlot.PlotLineBuffer(input.label, buffers[program.name][input.label])
				end
			end

			ImPlot.EndPlot()
		end
	end
end

return {
	update_inputs_for_program = update_inputs_for_program,
	plot_inputs_for_program = plot_inputs_for_program,
}
