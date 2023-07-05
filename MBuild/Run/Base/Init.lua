function printf(fmt, ...)
	print(string.format(fmt, ...));
end

function import(filename)
	local path              = fs.normalize(fs.absolute_script(filename, 1));
	_G._MBuildImports       = _G._MBuildImports or {};
	if _G._MBuildImports[path] then
		return unpack(_G._MBuildImports[path]);
	end

	local source, err = loadfile(path);
	if not source then
		local res = { false, string.format("Failed to load file '%s':\n  %s", path, err) };
		_G._MBuildImports[path] = res;
		print(res[2]);
		return unpack(res);
	end

	_G._MBuildImports[path] = { pcall(source) };
	local res = _G._MBuildImports[path];
	if not res[1] then
		print(res[2]);
	end
	return unpack(res);
end

function include(filename)
	local path = fs.normalize(fs.absolute_script(filename, 1));
	return dofile(path);
end

local files = {
	"Base.lua",
	"Workspace.lua",
	"Project.lua",
	"Files.lua",
	"When.lua",
	"Config.lua",
	"Configs.lua",

	"API.lua"
};
for _, file in ipairs(files) do
	import(file);
end