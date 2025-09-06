-- the entire buffer must be resized to fit browser limitations (supporting Librewolf)
local function max_axis_resize(size, max_size)
	if size.x < max_size.x and size.y < max_size.y then
		return size
	end

	if size.x > max_size.x then
		local rescale_needed = size.x / max_size.x
		return max_axis_resize(Vec2.new(size.y / rescale_needed, size.x / rescale_needed))
	elseif size.y > max_size.y then
		local rescale_needed = size.y / max_size.y
		return max_axis_resize(Vec2.new(size.y / rescale_needed, size.x / rescale_needed))
	end

	return size
end

return max_axis_resize
