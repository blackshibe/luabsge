local preferences = require("persistence.prefs")
local graphed = require("ui.graphed")

local visualization = {}

visualization.name = "Visualization"

function visualization.push_styles()
	ImGui.PushStyleVar(ImGui.StyleVar_WindowPadding, 0, 0)
end

function visualization.pop_styles()
	ImGui.PopStyleVar()
end

-- Network visualization state
local layer_info = nil
local show_weights = true
local weight_threshold = 0.1
local context_target = nil
local PICK_RADIUS = 0.05

local function get_layer_info()
	if layer_info then
		return layer_info
	end

	layer_info = {}
	local total = 0
	for i, layer in ipairs(SNN_LAYERS) do
		layer_info[i] = {
			name = layer.name,
			config = layer.config,
			count = layer.config.count,
			start = GetSnnLayerStartIndex(layer.config),
			labels = layer.labels,
			x = layer.x or (i - 1),
			y = layer.y or 0,
			layout = layer.layout or "grid",
		}
		total = total + layer.config.count
	end
	layer_info.total = total
	return layer_info
end

local function neuron_position(j, count, layer_x, layer_y, layout)
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
	for layer_index = 1, num_layers do
		local layer = info[layer_index]
		if layer.x < min_x then
			min_x = layer.x
		end
		if layer.x > max_x then
			max_x = layer.x
		end
		if layer.y - 1.2 < min_y then
			min_y = layer.y - 1.2
		end
		if layer.y + 1.2 > max_y then
			max_y = layer.y + 1.2
		end
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
								local fx, fy = neuron_position(j, from.count, from.x, from.y, from.layout)
								local tx, ty = neuron_position(i, to.count, to.x, to.y, to.layout)
								local xs = { fx, tx }
								local ys = { fy, ty }
								ImPlot.PlotLineStyled(("##c%d"):format(conn_id), xs, ys, r, g, b, alpha)
							end
						end
					end
				end
			end
		end

		-- right-click hit-test against neurons (only when plot is hovered)
		local hit_neuron = nil
		local plot_hovered = ImPlot.IsPlotHovered()
		local right_clicked = plot_hovered and ImGui.IsMouseClicked(1)
		local mx, my
		if plot_hovered then
			mx, my = ImPlot.GetPlotMousePos()
		end

		-- draw neuron points per layer, colored by stored charge
		local dot_id = 0
		for li = 1, num_layers do
			local layer = info[li]

			for j = 0, layer.count - 1 do
				local neuron = SNN_NETWORK_CONFIG:get_in_layer(layer.config, j)
				local nx, ny = neuron_position(j, layer.count, layer.x, layer.y, layer.layout)

				if right_clicked and not hit_neuron then
					local dx, dy = mx - nx, my - ny
					if dx * dx + dy * dy <= PICK_RADIUS then
						hit_neuron = { layer = layer, j = j, name = layer.name, global = layer.start + j }
					end
				end

				dot_id = dot_id + 1
				local xs = { nx }
				local ys = { ny }

				if neuron.output > 0.5 then
					ImPlot.PlotScatterStyled(
						("##n%d"):format(dot_id),
						xs,
						ys,
						ImPlot.Marker_Circle,
						8,
						1.0,
						1.0,
						0.2,
						1.0
					)
				else
					-- lerp blue -> orange by stored/threshold
					local t = math.min(neuron.stored / neuron.threshold, 1.0)
					local r = 0.3 + t * 0.7
					local g = 0.3 + t * 0.4
					local b = 0.8 - t * 0.6
					local size = 4 + t * 3

					if neuron.role == NeuronRole.Input then
						r = 0
						g = 0.5 + 5 * 0.5
						b = 0
					end

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

		if hit_neuron then
			context_target = hit_neuron
			ImGui.OpenPopup("##neuron_ctx")
		end
	end

	if ImGui.BeginPopup("##neuron_ctx") then
		if context_target then
			local n = SNN_NETWORK_CONFIG:get_in_layer(context_target.layer.config, context_target.j)
			ImGui.Text(
				("%s neuron #%d (global %d)"):format(context_target.name, context_target.j, context_target.global)
			)
			ImGui.Text(("stored: %.3f  threshold: %.3f"):format(n.stored, n.threshold))
			ImGui.Separator()
			if ImGui.MenuItem("Graph neuron") then
				local already_graphed = false
				for i, g in ipairs(graphed.neurons) do
					if g.global == context_target.global then
						already_graphed = true
						break
					end
				end
				if not already_graphed then
					table.insert(graphed.neurons, {
						global = context_target.global,
						name = context_target.name,
						j = context_target.j,
					})
					preferences.save()
				end
			end
			if ImGui.MenuItem("Force spike") then
				n:spike()
			end
			if ImGui.MenuItem("Reset stored") then
				n.stored = 0
			end
		end
		ImGui.EndPopup()
	end
end

function visualization.update()
	local _, new_weights = ImGui.Checkbox("Show Connections", show_weights)
	show_weights = new_weights

	if show_weights then
		ImGui.SameLine()
		local _, new_thresh = ImGui.SliderFloat("Min Weight", weight_threshold, 0.0, 1.0)
		weight_threshold = new_thresh
	end

	plot_graph()
end

return visualization
