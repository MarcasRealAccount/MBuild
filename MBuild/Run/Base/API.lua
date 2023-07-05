function Workspace(name, callback)
	if MBuild.currentLayer ~= MBuild.globalLayer then
		error("Workspace() has to be invoked globally!");
	end

	table.insert(MBuild.workspaces, MBuild.Workspace:new(name, callback));
end

function Project(name, callback)
	if MBuild.currentLayer ~= MBuild.workspaceLayer then
		error("Project() has to be invoked inside a workspace!");
	end

	table.insert(MBuild.currentWorkspace.projects, MBuild.Project:new(name, callback));
end

function When(conditions, callback)
	if type(conditions) ~= "table" then
		if type(conditions) ~= "string" then
			error("When() requires conditions to be either a string or an array of strings!");
		end

		conditions = { conditions };
	end
	for k, v in pairs(conditions) do
		if type(v) ~= "string" then
			error("When() requires conditions to be either a string or an array of strings!");
		end
	end

	local cur;
	if MBuild.currentLayer == MBuild.workspaceLayer then
		cur = MBuild.currentWorkspace;
	elseif MBuild.currentLayer == MBuild.projectLayer then
		cur = MBuild.currentProject;
	elseif MBuild.currentLayer == MBuild.filesLayer then
		cur = MBuild.currentFiles;
	else
		error("When() has to be invoked inside a workspace, project or files!");
	end
	table.insert(cur.whens, MBuild.When:new(conditions, callback));
end

function Files(inclusions, exclusions, callback)
	if MBuild.currentLayer ~= MBuild.projectLayer then
		error("Files() has to be invoked inside a project!");
	end

	if type(inclusions) ~= "table" then
		if type(inclusions) ~= "string" then
			error("Files() requires inclusions to be either a string or an array of strings!");
		end

		inclusions = { inclusions };
	end
	for k, v in pairs(inclusions) do
		if type(v) ~= "string" then
			error("Files() requires inclusions to be either a string or an array of strings!");
		end
	end

	if type(exclusions) ~= "table" then
		if type(exclusions) ~= "string" then
			error("Files() requires exclusions to be either a string or an array of strings!");
		end

		exclusions = { exclusions };
	end
	for k, v in pairs(exclusions) do
		if type(v) ~= "string" then
			error("Files() requires exclusions to be either a string or an array of strings!");
		end
	end

	table.insert(MBuild.currentProject.files, MBuild.Files:new(inclusions, exclusions, callback));
end

function Location(path)
	local cur;
	if MBuild.currentLayer == MBuild.workspaceLayer then
		cur = MBuild.currentWorkspace;
	elseif MBuild.currentLayer == MBuild.projectLayer then
		cur = MBuild.currentProject;
	else
		error("Location() has to be invoked inside a workspace or project!");
	end
	
	cur.location = path;
end

function Configurations(configurations)
	if MBuild.currentLayer ~= MBuild.workspaceLayer then
		error("Configurations() has to be invoked inside a workspace!");
	end

	if type(configurations) ~= "table" then
		if type(configurations) ~= "string" then
			error("Configurations() requires configurations to be either a string or an array of strings!");
		end

		configurations = { configurations };
	end
	for k, v in pairs(configurations) do
		if type(v) ~= "string" then
			error("Configurations() requires configurations to be either a string or an array of strings!");
		end
	end

	MBuild.currentWorkspace.configurations = configurations;
end

function Platforms(platforms)
	if MBuild.currentLayer ~= MBuild.workspaceLayer then
		error("Platforms() has to be invoked inside a workspace!");
	end

	if type(platforms) ~= "table" then
		if type(platforms) ~= "string" then
			error("Platforms() requires platforms to be either a string or an array of strings!");
		end

		platforms = { platforms };
	end
	for k, v in pairs(platforms) do
		if type(v) ~= "string" then
			error("Platforms() requires platforms to be either a string or an array of strings!");
		end
	end

	MBuild.currentWorkspace.platforms = platforms;
end

function Architecture(arch)
	if MBuild.currentLayer ~= MBuild.configLayer then
		error("Architecture() has to be invoked inside a when!");
	end

	MBuild.currentConfig.arch = arch;
end

function System(system)
	if MBuild.currentLayer ~= MBuild.configLayer then
		error("System() has to be invoked inside a when!");
	end

	MBuild.currentConfig.system = system;
end

local Configs = MBuild.Configs;

Configs.RegisterHandler({
	name     = "bool",
	callback = function(settings, value)
		if type(value) ~= "boolean" then
			error(string.format("'%s' requires boolean parameter got '%s'", settings.name, type(value)));
		end
		MBuild.currentConfigs[settings.key] = value;
	end
});
Configs.RegisterHandler({
	name     = "number",
	callback = function(settings, value)
		if type(value) ~= "number" then
			error(string.format("'%s' requires number parameter got '%s'", settings.name, type(value)));
		end
		MBuild.currentConfigs[settings.key] = value;
	end
});
Configs.RegisterHandler({
	name     = "int",
	callback = function(settings, value)
		if type(value) ~= "number" then
			error(string.format("'%s' requires integer parameter got '%s'", settings.name, type(value)));
		end
		value = math.tointeger(value);
		if not value then
			error(string.format("'%s' requires integer parameter got 'double'", settings.name));
		end
		MBuild.currentConfigs[settings.key] = value;
	end
});
Configs.RegisterHandler({
	name     = "string",
	callback = function(settings, value)
		if type(value) ~= "string" then
			error(string.format("'%s' requires string parameter got '%s'", settings.name, type(value)));
		end
		MBuild.currentConfigs[settings.key] = value;
	end
});

Configs.RegisterHandler({
	name     = "bool[]",
	callback = function(settings, value)
		if type(value) ~= "table" then
			if type(value) ~= "boolean" then
				error(string.format("'%s' requires a boolean or an array of boolean parameters got '%s'", settings.name, type(value)));
			end
			value = { value };
		end
		for k, v in pairs(value) do
			if type(v) ~= "boolean" then
				error(string.format("'%s' requires a boolean or an array of boolean parameters got '%s'='%s'", settings.name, tostring(k), type(v)));
			end
		end
		if settings.append then
			MBuild.currentConfigs[settings.key] = MBuild.currentConfigs[settings.key] or {};
			
			local arr = MBuild.currentConfigs[settings.key];
			for k, v in pairs(value) do
				table.insert(arr, v);
			end
		else
			MBuild.currentConfigs[settings.key] = value;
		end
	end
});
Configs.RegisterHandler({
	name     = "number[]",
	callback = function(settings, value)
		if type(value) ~= "table" then
			if type(value) ~= "number" then
				error(string.format("'%s' requires a number or an array of number parameters got '%s'", settings.name, type(value)));
			end
			value = { value };
		end
		for k, v in pairs(value) do
			if type(v) ~= "number" then
				error(string.format("'%s' requires a number or an array of number parameters got '%s'='%s'", settings.name, tostring(k), type(v)));
			end
		end
		if settings.append then
			MBuild.currentConfigs[settings.key] = MBuild.currentConfigs[settings.key] or {};
			
			local arr = MBuild.currentConfigs[settings.key];
			for k, v in pairs(value) do
				table.insert(arr, v);
			end
		else
			MBuild.currentConfigs[settings.key] = value;
		end
	end
});
Configs.RegisterHandler({
	name     = "int[]",
	callback = function(settings, value)
		if type(value) ~= "number" then
			error(string.format("'%s' requires integer parameter got '%s'", settings.name, type(value)));
		end
		value = math.tointeger(value);
		if not value then
			error(string.format("'%s' requires integer parameter got 'double'", settings.name));
		end
		MBuild.currentConfigs[settings.key] = value;

		if type(value) ~= "table" then
			if type(value) ~= "number" then
				error(string.format("'%s' requires an integer or an array of integer parameters got '%s'", settings.name, type(value)));
			end
			value = math.tointeger(value);
			if not value then
				error(string.format("'%s' requires an integer or an array of integer parameters got 'double'", settings.name));
			end
			value = { value };
		end
		for k, v in pairs(value) do
			if type(v) ~= "number" then
				error(string.format("'%s' requires an integer or an array of integer parameters got '%s'='%s'", settings.name, tostring(k), type(v)));
			end
		end
		if settings.append then
			MBuild.currentConfigs[settings.key] = MBuild.currentConfigs[settings.key] or {};
			
			local arr = MBuild.currentConfigs[settings.key];
			for k, v in pairs(value) do
				v = math.tointeger(v);
				if not v then
					error(string.format("'%s' requires an integer or an array of integer parameters got '%s'='double'", settings.name, tostring(k)));
				end
				table.insert(arr, v);
			end
		else
			MBuild.currentConfigs[settings.key] = value;
		end
	end
});
Configs.RegisterHandler({
	name     = "string[]",
	callback = function(settings, value)
		if type(value) ~= "table" then
			if type(value) ~= "string" then
				error(string.format("'%s' requires a string or an array of string parameters got '%s'", settings.name, type(value)));
			end
			value = { value };
		end
		for k, v in pairs(value) do
			if type(v) ~= "string" then
				error(string.format("'%s' requires a string or an array of string parameters got '%s'='%s'", settings.name, tostring(k), type(v)));
			end
		end
		if settings.append then
			MBuild.currentConfigs[settings.key] = MBuild.currentConfigs[settings.key] or {};
			
			local arr = MBuild.currentConfigs[settings.key];
			for k, v in pairs(value) do
				table.insert(arr, v);
			end
		else
			MBuild.currentConfigs[settings.key] = value;
		end
	end
});

Configs.RegisterConfig({
	type = "string",
	name = "ObjDir",
	key  = "objDir"
});
Configs.RegisterConfig({
	type = "string",
	name = "BinDir",
	key  = "binDir"
});
Configs.RegisterConfig({
	type = "string",
	name = "RunDir",
	key  = "runDir"
});
Configs.RegisterConfig({
	type  = "string",
	name  = "Warnings",
	key   = "warnings",
	valid = { "Off", "On", "Extra" }
});
Configs.RegisterConfig({
	type  = "string",
	name  = "Kind",
	key   = "kind",
	valid = { "ConsoleApp", "WindowedApp" }
});
Configs.RegisterConfig({
	type   = "string[]",
	name   = "IncludeDirs",
	key    = "includeDirs",
	append = true
});
Configs.RegisterConfig({
	type   = "string[]",
	name   = "ExternalDependencies",
	key    = "externalDependencies",
	append = true
});