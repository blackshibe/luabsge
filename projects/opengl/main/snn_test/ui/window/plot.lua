local plot = {}

plot.name = "Plot"
plot.buffers = {}

-- Line colors (r, g, b, a)
plot.colors = {
	input = { 1.0, 1.0, 1.0, 0.4 },
	output = { 1.0, 0.5, 0.5, 1.0 },
	potential = { 1.0, 1.0, 1.0, 1.0 },
	threshold = { 1.0, 0.5, 0.5, 0.4 },
}

-- Dash pattern (dash_length, gap_length) in pixels
plot.dash = { 6.0, 4.0 }

function plot.always_update()
	for _, v in pairs(SNN_NETWORK_LAYERS) do
		local start = GetSnnLayerStartIndex(v)
		for j = 0, v.count - 1 do
			local neuron = SNN_NETWORK_CONFIG:get_in_layer(v, j)
			local i = start + j
			plot.buffers[i] = plot.buffers[i] or {
				output = ScrollingBuffer.new(),
				store = ScrollingBuffer.new(),
				threshold = ScrollingBuffer.new(),
				input = ScrollingBuffer.new(),
			}
			plot.buffers[i].store:AddPoint(NETWORK_RUNTIME_SECONDS, neuron.stored)
			plot.buffers[i].input:AddPoint(NETWORK_RUNTIME_SECONDS, neuron.input)
			plot.buffers[i].output:AddPoint(NETWORK_RUNTIME_SECONDS, neuron.output > 0 and 1 or 0)
			plot.buffers[i].threshold:AddPoint(NETWORK_RUNTIME_SECONDS, neuron.threshold)
		end
	end
end

function plot.plot_neuron(i, title, same_line, flags)
	if not plot.buffers[i] then
		return
	end

	if same_line then
		ImGui.SameLine()
	end
	ImGui.BeginChild(title or tostring(i), -1, 340)

	local plot_flags = (flags or 0)
	if ImPlot.BeginPlot(title or ("Neuron Activity @ %i"):format(i), -1, 300, plot_flags) then
		ImPlot.SetupAxes("Time (ms)", "Activation")
		ImPlot.SetupAxisLimits(
			ImPlot.Axis_X1,
			NETWORK_RUNTIME_SECONDS - GRAPH_HISTORY_SECONDS,
			NETWORK_RUNTIME_SECONDS,
			ImPlot.Cond_Always
		)
		ImPlot.SetupAxisLimits(ImPlot.Axis_Y1, -0.5, 1.5, ImPlot.Cond_Once)
		local c = plot.colors
		local d = plot.dash
		ImPlot.PlotLineBufferDashed("Input", plot.buffers[i].input, c.input[1], c.input[2], c.input[3], c.input[4], d[1], d[2])
		ImPlot.PlotLineBufferStyled("Potential", plot.buffers[i].store, c.potential[1], c.potential[2], c.potential[3], c.potential[4])
		ImPlot.PlotSpikes("Spike", plot.buffers[i].output, c.output[1], c.output[2], c.output[3], c.output[4], -0.5, 1.5)
		ImPlot.PlotLineBufferDashed("Threshold", plot.buffers[i].threshold, c.threshold[1], c.threshold[2], c.threshold[3], c.threshold[4], d[1], d[2])
		ImPlot.EndPlot()
	end

	ImGui.EndChild()
end

return plot
