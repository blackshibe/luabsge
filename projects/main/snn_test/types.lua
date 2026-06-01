---@meta

-- Neural Network Types

---@class NeuronRole
---@field Input integer
---@field Neuron integer
---@field Output integer
NeuronRole = {}

---@class NeuronLayerConfiguration
---@field index integer
---@field count integer
---@field role integer
local NeuronLayerConfiguration = {}

---@param role integer NeuronRole value
---@param count integer Number of neurons in this layer
---@return NeuronLayerConfiguration
function NeuronLayerConfiguration.new(role, count) end

---@class NeuronLIF
---@field output number Current output potential
---@field stored number Current stored potential (alias for output)
---@field threshold number Firing threshold
---@field role integer NeuronRole value
local NeuronLIF = {}

---@return NeuronLIF
function NeuronLIF.new() end

function NeuronLIF:spike() end
function NeuronLIF:step() end

---@class NeuronNetworkConfigurationLIF
local NeuronNetworkConfigurationLIF = {}

---@return NeuronNetworkConfigurationLIF
function NeuronNetworkConfigurationLIF.new() end

---@param layer NeuronLayerConfiguration
function NeuronNetworkConfigurationLIF:add_group(layer) end

---@param from_layer integer Layer index
---@param to_layer integer Layer index
---@param weight number Weight value
function NeuronNetworkConfigurationLIF:set_weight_config(from_layer, to_layer, weight) end

---@param from_layer integer Layer index
---@param to_layer integer Layer index
---@param weight number Weight value (positive only)
function NeuronNetworkConfigurationLIF:set_weight_config_positive(from_layer, to_layer, weight) end

---@param from_layer integer Layer index
---@param to_layer integer Layer index
---@param weight number Fixed weight value
function NeuronNetworkConfigurationLIF:set_fixed_weight(from_layer, to_layer, weight) end

function NeuronNetworkConfigurationLIF:build() end

---@param delta_time number Time step in seconds
function NeuronNetworkConfigurationLIF:update(delta_time) end

---@param layer NeuronLayerConfiguration
---@param index integer Neuron index within layer (0-based)
---@return NeuronLIF
function NeuronNetworkConfigurationLIF:get_in_layer(layer, index) end

function NeuronNetworkConfigurationLIF:reward() end
function NeuronNetworkConfigurationLIF:punish() end

-- Global SNN functions

---@param to integer Target neuron global index
---@param from integer Source neuron global index
---@return number Weight value
function SNN_get_weights_to(to, from) end

---@param to integer Target neuron global index
---@param from integer Source neuron global index
---@return number Eligibility trace value
function SNN_get_eligibility_to(to, from) end

-- ImGui Types

---@class ImGui
---@field Cond_Once integer Apply condition once
---@field Cond_FirstUseEver integer Apply on first use ever
---@field Cond_Always integer Always apply
---@field StyleVar_WindowPadding integer Window padding style variable
ImGui = {}

---@param name string Window name
---@return boolean visible
---@overload fun(name: string, closable: boolean): boolean, boolean
---@overload fun(name: string, flags: integer): boolean
function ImGui.Begin(name) end

function ImGui.End() end

---@param width number
---@param height number
---@overload fun(width: number, height: number, cond: integer)
function ImGui.SetNextWindowSize(width, height) end

---@param x number
---@param y number
function ImGui.SetNextWindowPos(x, y) end

---@param idx integer Style variable index
---@param val number|nil Single value
---@param y number|nil Y value for vec2
function ImGui.PushStyleVar(idx, val, y) end

function ImGui.PopStyleVar() end

---@param label string
---@return boolean clicked
function ImGui.SmallButton(label) end

---@param text string
function ImGui.Text(text) end

---@param label string
---@param shortcut string|nil
---@param selected boolean|nil
---@return boolean clicked
function ImGui.MenuItem(label, shortcut, selected) end

-- ImPlot Types

---@class ImPlot
---@field Flags_None integer
---@field Flags_NoTitle integer
---@field Flags_NoLegend integer
---@field Flags_NoMouseText integer
---@field Flags_NoInputs integer
---@field Flags_NoMenus integer
---@field Flags_NoBoxSelect integer
---@field Flags_NoFrame integer
---@field Flags_Equal integer
---@field AxisFlags_NoDecorations integer
---@field Axis_X1 integer
---@field Axis_Y1 integer
---@field Cond_Once integer
---@field Cond_Always integer
---@field Marker_Circle integer
---@field Col_FrameBg integer
---@field Col_PlotBg integer
ImPlot = {}

---@param title string
---@param width number
---@param height number
---@param flags integer|nil
---@return boolean
function ImPlot.BeginPlot(title, width, height, flags) end

function ImPlot.EndPlot() end

---@param x_label string
---@param y_label string
---@param x_flags integer|nil
---@param y_flags integer|nil
function ImPlot.SetupAxes(x_label, y_label, x_flags, y_flags) end

---@param axis integer
---@param min number
---@param max number
---@param cond integer|nil
function ImPlot.SetupAxisLimits(axis, min, max, cond) end

---@param label string
---@param buffer ScrollingBuffer
function ImPlot.PlotLineBuffer(label, buffer) end

---@return boolean
function ImPlot.IsPlotHovered() end

---@return number x, number y
function ImPlot.GetPlotMousePos() end

---@param col integer
---@param r number
---@param g number
---@param b number
---@param a number
function ImPlot.PushStyleColor(col, r, g, b, a) end

---@param count integer
function ImPlot.PopStyleColor(count) end

-- ScrollingBuffer

---@class ScrollingBuffer
local ScrollingBuffer = {}

---@return ScrollingBuffer
function ScrollingBuffer.new() end

---@param x number
---@param y number
function ScrollingBuffer:AddPoint(x, y) end
