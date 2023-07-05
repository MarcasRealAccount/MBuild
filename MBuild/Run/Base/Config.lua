MBuild.Config = MBuild.Config or {};
local Config  = MBuild.Config;

function Config:new(name, platform, initialConfigs)
	config = {
		name     = name,
		platform = platform,
		arch     = os.arch(), -- Default arch is the current host arch
		system   = os.host(), -- Default system is the current host system

		configs = MBuild.ShallowCopy(initialConfigs or {})
	};
	setmetatable(config, self);
	self.__index = self;
	return config;
end

function Config:Apply(configs)
	for k, v in pairs(configs) do
		self.configs[k] = v;
	end
end

function Config.CreateMap(names, platforms, initialConfigs)
	local configMap = {};
	for _, name in ipairs(names) do
		configMap[name] = {};
		for _, platform in ipairs(platforms) do
			local config = MBuild.Config:new(name, platform, initialConfigs);

			configMap[name][platform] = config;
		end
	end
	return configMap;
end

function Config.ExtendMap(initialConfigMap, initialConfigs)
	local configMap = {};
	for name, arr in pairs(initialConfigMap) do
		configMap[name] = {};
		for platform, config in pairs(arr) do
			local newConfig  = MBuild.Config:new(name, platform, config.configs);
			newConfig.arch   = config.arch;
			newConfig.system = config.system;
			newConfig:Apply(initialConfigs);
			configMap[name][platform] = newConfig;
		end
	end
	return configMap;
end