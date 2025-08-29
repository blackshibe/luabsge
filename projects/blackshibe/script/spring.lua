-- Usage: local spring = Spring.new(target, stiffness, damping)

local Spring = {}
Spring.__index = Spring

function Spring.new(target, stiffness, damping)
	local self = setmetatable({}, Spring)
	self.target = target or 0
	self.position = target or 0
	self.velocity = 0
	self.stiffness = stiffness or 100 -- Higher = stiffer spring
	self.damping = damping or 10 -- Higher = more damping
	return self
end

function Spring:update(dt)
	-- Spring force equation: F = -k * (x - target) - d * velocity
	local force = -self.stiffness * (self.position - self.target) - self.damping * self.velocity

	-- Apply force to velocity and position
	self.velocity = self.velocity + force * dt
	self.position = self.position + self.velocity * dt

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
