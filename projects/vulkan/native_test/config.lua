-- Config for the engine

local title = ("vulkan/native_test (LuaBSGE %s)"):format(BSGE_VERSION)

BSGE = {
	default_asset_directory = "../../common/",
	window_configuration = WindowConfiguration.new(1400, 900, title),
}

COMMON_PATH = BSGE.default_asset_directory
