local M = {}

function M.count(t)
	local n = 0
	for _ in pairs(t) do
		n = n + 1
	end
	return n
end

function M.sorted_keys(t)
	local out = {}
	for k in pairs(t) do
		table.insert(out, k)
	end
	table.sort(out)
	return out
end

return M
