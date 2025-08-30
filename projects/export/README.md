# Exporter

steps:
[/] - Exporting datamodel and list of rbxassetids
[/] - Turning rbxassetid into assets - https://www.roblox.com/asset/?id=14576660028
[ ] - Mesh importing (https://devforum.roblox.com/t/roblox-filemesh-format-specification/326114/1)
[/] - Importing into files and luabsge project

[EDIT] : It is actually possible to download meshes directly in the .OBJ (only face, vertices and uv data) format by using https://thumbnails.roblox.com/v1/assets-thumbnail-3d?assetId=AssetId
This will return an URL under ‘imageUrl’ field which will return the URL for the .OBJ object under ‘obj’ field.
