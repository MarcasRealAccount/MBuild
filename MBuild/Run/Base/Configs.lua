MBuild.Configs = MBuild.Configs or {
	handlers = {}
};

local Configs   = MBuild.Configs;
Configs.Handler = Configs.Handler or {};
local Handler   = Configs.Handler;

function Handler:new(settings)
	local handler = {
		name     = settings.name,
		callback = settings.callback
	};
	setmetatable(handler, self);
	self.__index = self;
	return handler;
end

function Configs.RegisterHandler(settings)
	local handler = Handler:new(settings);
	Configs.handlers[handler.name] = handler;
end

function Configs.RegisterConfig(settings)
	local handler     = Configs.handlers[settings.type];
	_G[settings.name] = function(...)
		handler.callback(settings, ...);
	end
end