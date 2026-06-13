-- Usage: local spring = Spring.new(target, stiffness, damping)

local Spring = {}
Spring.__index = Spring

function Spring.new(target, stiffness, damping)
	local self = setmetatable({}, Spring)
	self.target = target or 0
	self.position = target or 0
	self.velocity = 0
	self.stiffness = stiffness or 100
	self.damping = damping or 10
	return self
end

function Spring:update(dt)
	-- substep simulation to prevent the spring exploding
	dt = math.min(dt, 1 / 60)
	local iterations = math.min(math.max((1 / dt) * 60, 1), 10)

	for _ = 1, iterations do
		-- Spring force equation: F = -k * (x - target) - d * velocity
		local force = -self.stiffness * (self.position - self.target) - self.damping * self.velocity

		self.velocity = self.velocity + force * (dt / iterations)
		self.position = self.position + self.velocity * (dt / iterations)
	end

	return self.position
end

function Spring:set_target(new_target)
	self.target = new_target
end

function Spring:set_position(new_position)
	self.position = new_position
end

function Spring:is_settled(threshold)
	threshold = threshold or 0.01
	return math.abs(self.position - self.target) < threshold and math.abs(self.velocity) < threshold
end

return Spring
