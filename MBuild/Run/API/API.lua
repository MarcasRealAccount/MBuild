dofile("API/MBuild.lua");
dofile("API/Workspace.lua");
dofile("API/Project.lua");
dofile("API/Config.lua");
dofile("API/When.lua");
dofile("API/Files.lua");

function MBuild:DeepCopy(variable, copies)
	copies        = copies or {};
	local varType = type(variable);
	if varType == "table" then
		if copies[variable] then
			return copies[variable];
		else
			local copy       = {};
			copies[variable] = copy;
			for key, value in next, variable, nil do
				copy[self:DeepCopy(key, copies)] = self:DeepCopy(value, copies);
			end
			setmetatable(copy, self:DeepCopy(getmetatable(variable), copies));
			return copy;
		end
	else
		return variable;
	end
end

function MBuild:SetConfig(key, value)
	if self.currentLayer == self.workspaceLayer then
		self.currentWorkspace.configs[key] = value;
		return true;
	elseif self.currentLayer == self.projectLayer then
		self.currentProject.configs[key] = value;
		return true;
	elseif self.currentLayer == self.configLayer then
		self.currentConfig.configs[key] = value;
		return true;
	else
		return false;
	end
end

function MBuild:NewConfig(name, key)
	local self2 = self;
	_G[name]    = function(value)
		self2:SetConfig(key, value);
	end
end

function MBuild:EvaluateWhen(when)
	if self.currentLayer ~= self.configLayer then
		return false;
	end

	local origConfiguration = configuration;
	local origPlatform      = platform;
	local origArchitecture  = architecture;
	local origSystem        = system;

	local config  = MBuild.currentConfig;
	configuration = config.name;
	platform      = config.platform;
	architecture  = config.arch;
	system        = config.os;

	local result = true;

	for _, condition in ipairs(when.conditions) do
		local suc, res = pcall(loadstring("return " .. condition));
		if not res then
			result = false;
			break;
		end
	end
	
	configuration = origConfiguration;
	platform      = origPlatform;
	architecture  = origArchitecture;
	system        = origSystem;

	return result;
end

function MBuild:InvokeMainScript(script)
	local scriptCode = loadfile(script);
	if not scriptCode then
		error(string.format("Failed to load main script %s", script));
	end

	local suc, err = pcall(scriptCode);
	if not suc then
		error(string.format("Failed to invoke main script %s with error:\n%s", script, err));
	end
end

function MBuild:Configure()
	for i, workspace in ipairs(self.workspaces) do
		self.currentWorkspace = workspace;
		self.currentLayer     = self.workspaceLayer;
		
		workspace.callback();
	
		for _, configuration in ipairs(workspace.configurations) do
			for _, platform in ipairs(workspace.platforms) do
				local config = MBuild.Config:new(configuration, platform, workspace.configs);
				
				workspace.configMap[string.format("%s-%s", configuration, platform)] = config;
		
				self.currentConfig = config;
				self.currentLayer  = self.configLayer;
		
				for _, when in ipairs(workspace.whens) do
					if self:EvaluateWhen(when) then
						self.currentWhen = when;
						when.callback();
						self.currentWhen = nil;
					end
				end
		
				self.currentConfig = nil;
				self.currentLayer  = self.workspaceLayer;
			end
		end
	
		for j, project in ipairs(workspace.projects) do
			self.currentProject = project;
			self.currentLayer   = self.projectLayer;
			
			project.callback();
		
			project.configMap = self:DeepCopy(workspace.configMap);
			for _, config in pairs(project.configMap) do
				for k, v in pairs(project.configs) do
					config.configs[k] = v;
				end

				self.currentConfig = config;
				self.currentLayer  = self.configLayer;

				for _, when in ipairs(project.whens) do
					if self:EvaluateWhen(when) then
						self.currentWhen = when;
						when.callback();
						self.currentWhen = nil;
					end
				end
			
				self.currentConfig = nil;
				self.currentLayer  = self.projectLayer;
			end
		
			for k, files in ipairs(project.files) do
				self.currentFiles = files;
				self.currentLayer = self.filesLayer;
			
				if files.callback then
					files.callback();
				end
			
				files.configMap = self:DeepCopy(project.configMap);
				for _, config in pairs(files.configMap) do
					for k, v in pairs(files.configs) do
						config.configs[k] = v;
					end

					self.currentConfig = config;
					self.currentLayer  = self.configLayer;
					
					for _, when in ipairs(files.whens) do
						if self:EvaluateWhen(when) then
							self.currentWhen = when;
							when.callback();
							self.currentWhen = nil;
						end
					end
					
					self.currentConfig = nil;
					self.currentLayer  = self.filesLayer;
				end
				
				self.currentFiles = nil;
				self.currentLayer = self.projectLayer;
			end
			
			self.currentProject = nil;
			self.currentLayer   = self.workspaceLayer;
		end
		
		self.currentWorkspace = nil;
		self.currentLayer     = self.globalLayer;
	end

	-- Debug
	for i, workspace in ipairs(self.workspaces) do
		print(string.format("Workspace %s configs:", workspace.name));
		for key, config in pairs(workspace.configMap) do
			print(string.format("  [\"%s\"]: name = %s, platform = %s, arch = %s, os = %s", key, config.name, config.platform, config.arch, config.os));
			for k,v in pairs(config.configs) do
				print(string.format("    %s = %s", tostring(k), tostring(v)));
			end
		end
		print();
	
		for j, project in ipairs(workspace.projects) do
			print(string.format("Project %s configs:", project.name));
			for key, config in pairs(project.configMap) do
				print(string.format("  [\"%s\"]: name = %s, platform = %s, arch = %s, os = %s", key, config.name, config.platform, config.arch, config.os));
				for k,v in pairs(config.configs) do
					print(string.format("    %s = %s", tostring(k), tostring(v)));
				end
			end
			print();
	
			for k, files in ipairs(project.files) do
				for _, inclusion in ipairs(files.inclusions) do
					print(string.format("Files %s", inclusion));
				end
				for _, exclusion in ipairs(files.exclusions) do
					print(string.format("Excluding files %s", exclusion));
				end
				print("Configs:");
	
				for key, config in pairs(files.configMap) do
					print(string.format("  [\"%s\"]: name = %s, platform = %s, arch = %s, os = %s", key, config.name, config.platform, config.arch, config.os));
					for k,v in pairs(config.configs) do
						print(string.format("    %s = %s", tostring(k), tostring(v)));
					end
				end
	
				print();
			end
		end
	end
end

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
		conditions = { conditions };
	end

	if MBuild.currentLayer == MBuild.workspaceLayer then
		table.insert(MBuild.currentWorkspace.whens, MBuild.When:new(conditions, callback));
	elseif MBuild.currentLayer == MBuild.projectLayer then
		table.insert(MBuild.currentProject.whens, MBuild.When:new(conditions, callback));
	elseif MBuild.currentLayer == MBuild.filesLayer then
		table.insert(MBuild.currentFiles.whens, MBuild.When:new(conditions, callback));
	else
		error("When() has to be invoked inside a workspace, project or files!");
	end
end

function Files(inclusions, exclusions, callback)
	if MBuild.currentLayer ~= MBuild.projectLayer then
		error("Files() has to be invoked inside a project!");
	end

	if type(inclusions) ~= "table" then
		inclusions = { inclusions };
	end

	if type(exclusions) ~= "table" then
		exclusions = { exclusions };
	end

	table.insert(MBuild.currentProject.files, MBuild.Files:new(inclusions, exclusions, callback));
end

function Location(path)
	if MBuild.currentLayer == MBuild.workspaceLayer then
		MBuild.currentWorkspace.location = path
	elseif MBuild.currentLayer == MBuild.projectLayer then
		MBuild.currentProject.location = path
	else
		error("Location() has to be invoked inside a workspace or project!");
	end
end

function Configurations(configurations)
	if MBuild.currentLayer ~= MBuild.workspaceLayer then
		error("Configurations() has to be invoked inside a workspace!");
	end

	if type(configurations) ~= "table" then
		configurations = { configurations };
	end

	MBuild.currentWorkspace.configurations = configurations;
end

function Platforms(platforms)
	if MBuild.currentLayer ~= MBuild.workspaceLayer then
		error("Platforms() has to be invoked inside a workspace!");
	end

	if type(platforms) ~= "table" then
		platforms = { platforms };
	end

	MBuild.currentWorkspace.platforms = platforms;
end

function Architecture(arch)
	if MBuild.currentLayer ~= MBuild.configLayer then
		error("Architecture() has to be invoked inside a when!");
	end

	MBuild.currentConfig.arch = arch;
end

function System(os)
	if MBuild.currentLayer ~= MBuild.configLayer then
		error("System() has to be invoked inside a when!");
	end

	MBuild.currentConfig.os = os;
end

MBuild:NewConfig("ObjDir", "objDir");
MBuild:NewConfig("BinDir", "binDir");
MBuild:NewConfig("RunDir", "runDir");
MBuild:NewConfig("Warnings", "warnings");
MBuild:NewConfig("Kind", "kind");
MBuild:NewConfig("IncludeDirs", "includeDirs");
MBuild:NewConfig("ExternalDependencies", "externalDependencies");