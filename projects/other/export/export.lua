-- exports a Roblox game

local HttpService = game:GetService("HttpService")

local meshes = ""
local images = ""

local datamodel = { workspace = {} }

local funcs = {
	Attachment = function(v)
		return {
			transform = { v.WorldCFrame:GetComponents() },
		}
	end,
	MeshPart = function(v)
		meshes = meshes .. v.Name .. "," .. v.MeshId .. "\n"
		images = images .. v.Name .. "_texture" .. "," .. v.TextureID .. "\n"
		return {
			transform = { v.CFrame:GetComponents() },
			mesh_id = v.MeshId,
			texture_id = v.TextureID,
		}
	end,
	Weld = function(v) -- not relevant
		return {}
	end,
}

local function export_service(data, target, name)
	local function export_instance(path, v)
		local exporter = funcs[v.ClassName]
		local data = {}
		if not exporter then
			warn("no exporter for", v.ClassName)
			data = {}
		else
			data = exporter(v)
		end

		data.children = {}
		for _, v in pairs(v:GetChildren()) do
			data.children[v.Name] = export_instance(v.Name, v)
		end

		return data
	end

	for _, v in pairs(target:GetChildren()) do
		data[v.Name] = export_instance(v.Name, v)
	end
end

export_service(datamodel.workspace, workspace, "Workspace")

print("DATAMODEL")
print(HttpService:JSONEncode(datamodel))

print("MESHES")
print(meshes)

print("IMAGES")
print(images)
