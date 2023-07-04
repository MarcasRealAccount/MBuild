dofile("API/MBuild.lua");

MBuild.Workspace = MBuild.Workspace or {};

local Workspace = MBuild.Workspace;

function Workspace:new(name, callback)
	workspace = {
		name           = name,
		callback       = callback,
		projects       = {},
		whens          = {},
		location       = "./",
		configs        = {},
		configMap      = {},
		configurations = {},
		platforms      = {}
	};
	setmetatable(workspace, self);
	self.__index = self;
	return workspace;
end