function printf(fmt, ...)
	print(string.format(fmt, ...));
end

function import(filename)
	local path              = fs.normalize(fs.absolute_script(filename, 1));
	_G._MBuildImports       = _G._MBuildImports or {};
	if _G._MBuildImports[path] then
		return unpack(_G._MBuildImports[path]);
	end

	local source = loadfile(path);
	if not source then
		_G._MBuildImports[path] = { false, string.format("Failed to load file '%s'", path) };
	end

	_G._MBuildImports[path] = { pcall(source) };
	return unpack(_G._MBuildImports[path]);
end

function include(filename)
	local path = fs.normalize(fs.absolute_script(filename, 1));
	return dofile(path);
end

local files = {
	"Base.lua",
	"Workspace.lua"
};
for _, file in ipairs(files) do
	import(file);
end