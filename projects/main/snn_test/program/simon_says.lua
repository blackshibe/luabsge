local showing_sequence = true
local sequence
local sequence_index = 0
local answered_correctly = -1
local retries_left = 20

local SEQUENCE_LENGTH = 1
local PULSES_TO_ANSWER = 5

WINS = 0
LOSSES = 0

local pulses_to_answer_left = 0

local function set_input(program, name, active)
	for i, v in pairs(program.inputs) do
		if v.label == name then
			v.current = active and 1 or 0
			return
		end
	end
end

local function index_to_input(i)
	if i == 1 then
		return "IN_RED"
	end
	if i == 2 then
		return "IN_BLUE"
	end
	if i == 3 then
		return "IN_GREEN"
	end
	if i == 4 then
		return "IN_YELLOW"
	end
end

-- 100ms pulses that mean the game stepped forward
local timer = NETWORK_RUNTIME_SECONDS + 0.1

local SimonSays
SimonSays = {
	name = "SimonSays",

	inputs = {
		{ label = "TIME", current = 0 },
		{ label = "IN_SELECTING", current = 0 },
		{ label = "REWARD", current = 0 },
		{ label = "PUNISH", current = 0 },
		{ label = "IN_RED", current = 0 },
		{ label = "IN_BLUE", current = 0 },
		{ label = "IN_GREEN", current = 0 },
		{ label = "IN_YELLOW", current = 0 },
	},

	outputs = {
		{ label = "RED", current = 1, sum = 0 },
		{ label = "BLUE", current = 0, sum = 0 },
		{ label = "GREEN", current = 0, sum = 0 },
		{ label = "YELLOW", current = 0, sum = 0 },
	},

	update = function()
		local is_next_spike = NETWORK_RUNTIME_SECONDS > timer
		if is_next_spike then
			set_input(SimonSays, "TIME", true)
			timer = NETWORK_RUNTIME_SECONDS + 0.1
		else
			set_input(SimonSays, "TIME", false)
		end

		set_input(SimonSays, "REWARD", false)
		set_input(SimonSays, "PUNISH", false)

		-- spike each input in sequence every timer step
		if not is_next_spike then
			return
		end

		if pulses_to_answer_left > 0 then
			pulses_to_answer_left = pulses_to_answer_left - 1

			return
		end

		-- encode simon says sequence, then wait for network to play it back
		set_input(SimonSays, "IN_SELECTING", showing_sequence)
		if showing_sequence then
			-- generate sequence
			if not sequence then
				print("new sequence:")
				sequence = {}
				sequence_index = 0
				for i = 1, SEQUENCE_LENGTH do
					sequence[i] = 4 --

					print("\t", index_to_input(sequence[i]))
				end
			end

			sequence_index = sequence_index + 1

			-- time to play
			if sequence_index > #sequence then
				print("playing")
				sequence_index = 1
				showing_sequence = false
				pulses_to_answer_left = PULSES_TO_ANSWER
				return
			end

			print("spiking", index_to_input(sequence[sequence_index]))

			set_input(SimonSays, "IN_RED", false)
			set_input(SimonSays, "IN_BLUE", false)
			set_input(SimonSays, "IN_GREEN", false)
			set_input(SimonSays, "IN_YELLOW", false)

			-- small delays
			pulses_to_answer_left = 1
			answered_correctly = -1

			return
		end

		-- keep the correct input active during answer period
		set_input(SimonSays, "IN_RED", false)
		set_input(SimonSays, "IN_BLUE", false)
		set_input(SimonSays, "IN_GREEN", false)
		set_input(SimonSays, "IN_YELLOW", false)

		local chosen_output
		local max_sum = 0
		for i, output in pairs(SimonSays.outputs) do
			if output.sum > max_sum then
				max_sum = output.sum
				chosen_output = i
			end
			output.sum = 0
		end

		if not chosen_output then
			warn("network chose nothing, waiting again")
			answered_correctly = false
			pulses_to_answer_left = PULSES_TO_ANSWER
			showing_sequence = true
			sequence_index = 0

			-- no chosen output, punish all outputs
			SNN_Punish()
			reset_sums()

			return
		end

		print("network chose", index_to_input(chosen_output), "@", max_sum)
		local correct = sequence[sequence_index]

		if correct == chosen_output then
			print("correct")
			sequence_index = sequence_index + 1
			answered_correctly = true
			pulses_to_answer_left = PULSES_TO_ANSWER
			SNN_Reward()
			reset_sums()

			if sequence_index > #sequence then
				print("network won game")
				showing_sequence = true
				sequence_index = 0
				sequence = nil

				WINS = WINS + 1
				set_input(SimonSays, "REWARD", true)

				return
			end
		else
			warn("wrong, restarting same sequence")
			answered_correctly = false
			pulses_to_answer_left = PULSES_TO_ANSWER
			showing_sequence = true
			sequence_index = 0

			SNN_Punish()
			reset_sums()
			LOSSES = LOSSES + 1

			set_input(SimonSays, "PUNISH", true)

			return
		end
	end,
}

return SimonSays
