dofile("API/MBuild.lua");

MBuild.When = MBuild.When or {};

local When = MBuild.When;

function When:new(conditions, callback)
	when = {
		conditions = conditions,
		callback   = callback
	};
	setmetatable(when, self);
	self.__index = self;
	return when;
end