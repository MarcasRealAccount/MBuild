MBuild.Project = MBuild.Project or {};
local Project  = MBuild.Project;

function Project:new(name, callback)
	project = {
		name     = name,
		callback = callback,
		files    = {},
		whens    = {},

		location  = "./",
		configs   = {},
		configMap = {}
	};
	setmetatable(project, self);
	self.__index = self;
	return project;
end