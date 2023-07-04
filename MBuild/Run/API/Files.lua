dofile("API/MBuild.lua");

MBuild.Files = MBuild.Files or {};

local Files = MBuild.Files;

function Files:new(inclusions, exclusions, callback)
	files = {
		inclusions = inclusions,
		exclusions = exclusions,
		callback   = callback,
		whens      = {},
		configs    = {},
		configMap  = {}
	};
	setmetatable(files, self);
	self.__index = self;
	return files;
end