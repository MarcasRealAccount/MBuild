MBuild        = MBuild or {};
MBuild.Config = MBuild.Config or {};

local Config = MBuild.Config;

function Config:new(name, platform, configs)
	config = {
		name     = name,
		platform = platform,
		arch     = os.arch(), -- Default arch is the current host arch
		os       = os.host(), -- Default os is the current host os
		configs  = MBuild:DeepCopy(configs or {})
	};
	setmetatable(config, self);
	self.__index = self;
	return config;
end