local M = {}

local layer_info = nil
local show_weights = true
local weight_threshold = 0.1

local function get_layer_info()
	if layer_info then
		return layer_info
	end

	layer_info = {}
	local offset = 0
	for i, layer in ipairs(SNN_LAYERS) do
		layer_info[i] = {
			name = layer.name,
			config = layer.config,
			count = layer.config.count,
			start = offset,
			labels = layer.labels,
			x = layer.x or (i - 1),
			y = layer.y or 0,
			layout = layer.layout or "grid",
		}
		offset = offset + layer.config.count
	end
	layer_info.total = offset
	return layer_info
end

local function neuron_pos(j, count, layer_x, layer_y, layout)
	if count == 1 then
		return layer_x, layer_y
	end

	if layout == "line" then
		local ny = layer_y + (j / (count - 1)) * 2.0 - 1.0
		return layer_x, ny
	end

	-- grid layout for recurrent layers
	local cols = math.ceil(math.sqrt(count))
	local rows = math.ceil(count / cols)
	local col = j % cols
	local row = math.floor(j / cols)

	local spacing = 0.12
	local nx = layer_x + (col - (cols - 1) / 2) * spacing
	local ny = layer_y + (row - (rows - 1) / 2) * spacing

	return nx, ny
end

local function plot_graph()
	local info = get_layer_info()
	local num_layers = #info

	-- find bounds from layer positions
	local min_x, max_x, min_y, max_y = 0, 0, 0, 0
	for li = 1, num_layers do
		local layer = info[li]
		if layer.x < min_x then min_x = layer.x end
		if layer.x > max_x then max_x = layer.x end
		if layer.y - 1.2 < min_y then min_y = layer.y - 1.2 end
		if layer.y + 1.2 > max_y then max_y = layer.y + 1.2 end
	end

	local plot_flags = ImPlot.Flags_NoLegend + ImPlot.Flags_Equal + ImPlot.Flags_NoMouseText
	if ImPlot.BeginPlot("##network_graph", -1, -1, plot_flags) then
		local axis_flags = ImPlot.AxisFlags_NoDecorations
		ImPlot.SetupAxes("", "", axis_flags, axis_flags)
		ImPlot.SetupAxisLimits(ImPlot.Axis_X1, min_x - 0.5, max_x + 0.5, ImPlot.Cond_Once)
		ImPlot.SetupAxisLimits(ImPlot.Axis_Y1, min_y - 0.3, max_y + 0.3, ImPlot.Cond_Once)

		-- draw connection lines between layers
		if show_weights then
			local conn_id = 0
			for li = 1, num_layers do
				for lj = 1, num_layers do
					local from = info[li]
					local to = info[lj]

					for i = 0, to.count - 1 do
						for j = 0, from.count - 1 do
							local w = SNN_get_weights_to(to.start + i, from.start + j)
							if math.abs(w) > weight_threshold then
								conn_id = conn_id + 1
								local alpha = math.abs(w)
								local r, g, b = 0.3, 0.6, 1.0
								if w < 0 then
									r, g, b = 1.0, 0.3, 0.3
								end
								local fx, fy = neuron_pos(j, from.count, from.x, from.y, from.layout)
								local tx, ty = neuron_pos(i, to.count, to.x, to.y, to.layout)
								local xs = { fx, tx }
								local ys = { fy, ty }
								ImPlot.PlotLineStyled(("##c%d"):format(conn_id), xs, ys, r, g, b, alpha)
							end
						end
					end
				end
			end
		end

		-- draw neuron points per layer, colored by stored charge
		local dot_id = 0
		for li = 1, num_layers do
			local layer = info[li]

			for j = 0, layer.count - 1 do
				local neuron = SNN_NETWORK:get_in_layer(layer.config, j)
				local nx, ny = neuron_pos(j, layer.count, layer.x, layer.y, layer.layout)

				dot_id = dot_id + 1
				local xs = { nx }
				local ys = { ny }

				if neuron.output > 0.5 then
					ImPlot.PlotScatterStyled(("##n%d"):format(dot_id), xs, ys, ImPlot.Marker_Circle, 8, 1.0, 1.0, 0.2, 1.0)
				else
					-- lerp blue -> orange by stored/threshold
					local t = math.min(neuron.stored / neuron.threshold, 1.0)
					local r = 0.3 + t * 0.7
					local g = 0.3 + t * 0.4
					local b = 0.8 - t * 0.6
					local size = 4 + t * 3
					ImPlot.PlotScatterStyled(("##n%d"):format(dot_id), xs, ys, ImPlot.Marker_Circle, size, r, g, b, 1.0)
				end

				if layer.labels and layer.labels[j + 1] then
					local offset_x = -0.15
					if li == num_layers then
						offset_x = 0.15
					end
					ImPlot.PlotText(layer.labels[j + 1], nx + offset_x, ny)
				end
			end

			ImPlot.PlotText(layer.name, layer.x, layer.y - 1.2)
		end

		ImPlot.EndPlot()
	end
end

function M.plot_network()
	-- local _, new_weights = ImGui.Checkbox("Show Connections", show_weights)
	-- show_weights = new_weights

	-- if show_weights then
	-- 	ImGui.SameLine()
	-- 	local _, new_thresh = ImGui.SliderFloat("Min Weight", weight_threshold, 0.0, 1.0)
	-- 	weight_threshold = new_thresh
	-- end

	plot_graph()
end

return M
