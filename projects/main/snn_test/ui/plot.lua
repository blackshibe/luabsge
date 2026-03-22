local buffers = {}

local WIDTH = 600

local function update_neuron(i, input)
	buffers[i] = buffers[i]
		or {
			output = ScrollingBuffer.new(),
			store = ScrollingBuffer.new(),
			threshold = ScrollingBuffer.new(),
			show_output = true,
			show_stored = true,
		}

	buffers[i].store:AddPoint(NETWORK_RUNTIME_SECONDS, input.stored)
	buffers[i].output:AddPoint(NETWORK_RUNTIME_SECONDS, input.output)
	buffers[i].threshold:AddPoint(NETWORK_RUNTIME_SECONDS, input.threshold)
end

local function plot_neuron(i, title, same_line)
	if not buffers[i] then
		return
	end

	if same_line then
		ImGui.SameLine()
	end
	ImGui.BeginChild(title or tostring(i), WIDTH, 340)

	local _, new_output = ImGui.Checkbox(("Show Output @ %i"):format(i), buffers[i].show_output)
	buffers[i].show_output = new_output
	ImGui.SameLine()
	local _, new_stored = ImGui.Checkbox(("Show Stored @ %i"):format(i), buffers[i].show_stored)
	buffers[i].show_stored = new_stored

	if ImPlot.BeginPlot(title or ("Neuron Activity @ %i"):format(i), WIDTH, 300) then
		ImPlot.SetupAxes("Time (ms)", "Activation")
		ImPlot.SetupAxisLimits(
			ImPlot.Axis_X1,
			NETWORK_RUNTIME_SECONDS - GRAPH_HISTORY_SECONDS,
			NETWORK_RUNTIME_SECONDS,
			ImPlot.Cond_Always
		)
		ImPlot.SetupAxisLimits(ImPlot.Axis_Y1, -0.1, 1.1, ImPlot.Cond_Once)
		if buffers[i].show_output then
			ImPlot.PlotLineBuffer("Output", buffers[i].output)
		end
		if buffers[i].show_stored then
			ImPlot.PlotLineBuffer("Stored", buffers[i].store)
		end
		ImPlot.PlotLineBuffer("Threshold", buffers[i].threshold)
		ImPlot.EndPlot()
	end

	ImGui.EndChild()
end

return {
	update_neuron = update_neuron,
	plot_neuron = plot_neuron,
}
