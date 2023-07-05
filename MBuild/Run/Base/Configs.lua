MBuild.Configs = MBuild.Configs or {
	handlers = {},
	configs  = {}
};

local Configs   = MBuild.Configs;
Configs.Handler = Configs.Handler or {};
local Handler   = Configs.Handler;
Configs.Config  = Configs.Config or {};
local Config    = Configs.Config;

function Handler:new(settings)
	local handler = {
		name     = settings.name,
		callback = settings.callback,
		evaluate = settings.evaluate
	};
	setmetatable(handler, self);
	self.__index = self;
	return handler;
end

function Config:new(settings)
	local config   = settings;
	config.handler = Configs.handlers[config.type];
	setmetatable(config, self);
	self.__index = self;
	return config;
end

function Configs.TestValid(value, valids)
	if not valids then
		return true;
	end

	for _, v in ipairs(valids) do
		if value == v then
			return true;
		end
	end
	return false;
end

function Configs.ValidValuesToString(valids)
	local str = "";
	for _, v in ipairs(valids) do
		if str:len() > 0 then
			str = str .. ", ";
		end

		local vtype = type(v);
		if vtype == "table" then
			str = str .. "table";
		elseif vtype == "string" then
			str = str .. string.format("\"%s\"", string.escape(v));
		else
			str = str .. tostring(v);
		end
	end
	return str;
end

function Configs.RegisterHandler(settings)
	local handler = Handler:new(settings);

	Configs.handlers[handler.name] = handler;
end

function Configs.RegisterConfig(settings)
	local config = Config:new(settings);

	Configs.configs[config.key] = config;
	_G[config.name] = function(...)
		config.handler.callback(config, ...);
	end
end