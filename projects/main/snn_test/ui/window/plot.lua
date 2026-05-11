local plot = {}

plot.name = "Plot"
plot.buffers = {}

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
			}
			plot.buffers[i].store:AddPoint(NETWORK_RUNTIME_SECONDS, neuron.stored)
			plot.buffers[i].output:AddPoint(NETWORK_RUNTIME_SECONDS, neuron.output)
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
		ImPlot.SetupAxisLimits(ImPlot.Axis_Y1, -0.1, 1.1, ImPlot.Cond_Once)
		ImPlot.PlotLineBuffer("Output", plot.buffers[i].output)
		ImPlot.PlotLineBuffer("Stored", plot.buffers[i].store)
		ImPlot.PlotLineBuffer("Threshold", plot.buffers[i].threshold)
		ImPlot.EndPlot()
	end

	ImGui.EndChild()
end

return plot
