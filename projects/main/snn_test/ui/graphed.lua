local preferences = require("persistence.prefs")

local graphed_neurons = preferences.data.graphed_neurons or {}
preferences.data.graphed_neurons = graphed_neurons

return {
	neurons = graphed_neurons,
}
