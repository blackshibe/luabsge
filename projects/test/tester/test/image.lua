-- Tests if the image API is at least somewhat functional
-- The main problems are often with metatables and things not being defined correctly, not actual behaviour

return function()
	local image = _temp_Image("image/fox.jpg")

	return true
end
