---@meta

---This is an autodoc tool test
---@class Instance
---@field parent integer Parent of entity.
---@field name string Name of entity.
---@field children Instance[] Instances parented to this one.
Instance = {}

---Creates a new blank instance with the provided name.
---@param name string
---@return Instance
function Instance.new(name) end
