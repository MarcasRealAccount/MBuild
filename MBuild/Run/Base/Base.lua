MBuild = MBuild or {
	workspaces       = {},
	currentScript    = nil,
	currentWorkspace = nil,
	currentProject   = nil,
	currentConfig    = nil,
	currentConfigs   = nil,
	currentFiles     = nil,
	currentWhen      = nil,
	currentLayer     = 0,
	-- Layer Enumerations:
	globalLayer    = 0,
	workspaceLayer = 1,
	projectLayer   = 2,
	configLayer    = 3,
	filesLayer     = 4
};

function MBuild.ShallowCopy(orig)
	if type(orig) == "table" then
		local copy = {};
		for k, v in pairs(orig) do
			copy[k] = v;
		end
		setmetatable(copy, getmetatable(orig));
		return copy;
	else
		return orig;
	end
end

function MBuild.DeepCopy(orig, copies)
	copies = copies or {};
	if type(orig) == "table" then
		if copies[orig] then
			return copies[orig];
		else
			local copy   = {};
			copies[orig] = copy;
			for key, value in next, orig, nil do
				copy[self.DeepCopy(key, copies)] = self.DeepCopy(value, copies);
			end
			setmetatable(copy, self.DeepCopy(getmetatable(orig), copies));
			return copy;
		end
	else
		return orig;
	end
end

function MBuild:InvokeMainScript(script)
	local origWorkspaces = self.workspaces;
	self.workspaces      = {};
	self.currentScript   = fs.normalize(fs.absolute_script(script, 1));

	local suc, err = import(self.currentScript);
	if suc then
		for _, workspace in ipairs(self.workspaces) do
			table.insert(origWorkspaces, workspace);
		end
	else
		print(err);
	end

	self.workspaces    = origWorkspaces;
	self.currentScript = nil;
	return suc;
end

function MBuild:ConfigureWhens(configMap, whens, previousLayer)
	for name, arr in pairs(configMap) do
		local origConfig = _G.configuration;
		_G.configuration = name;

		for platform, config in pairs(arr) do
			local origPlatform = _G.platform;
			local origArch     = _G.architecture;
			local origSystem   = _G.system;
			_G.platform        = platform;
			_G.architecture    = config.arch;
			_G.system          = config.system;

			self.currentConfig  = config;
			self.currentConfigs = config.configs;
			self.currentLayer   = self.configLayer;

			for _, when in ipairs(whens) do
				self.currentWhen = when;
				when:Evaluate();
				self.currentWhen = nil;
			end

			self.currentConfig  = nil;
			self.currentConfigs = nil;
			self.currentLayer   = previousLayer;

			_G.platform     = origPlatform;
			_G.architecture = origArch;
			_G.system       = origSystem;
		end

		_G.configuration = origConfig;
	end
end

function MBuild:ConfigureFiles(files, previousLayer)
	self.currentFiles = files;
	self.currentLayer = self.filesLayer;

	if files.callback then
		self.currentConfigs = files.configs;
		files.callback();
		self.currentConfigs = nil;
	end

	files.configMap = MBuild.Config.ExtendMap(self.currentProject.configMap, files.configs);
	self:ConfigureWhens(files.configMap, files.whens, self.filesLayer);

	self.currentFiles = nil;
	self.currentLayer = previousLayer;
end

function MBuild:ConfigureProject(project, previousLayer)
	self.currentProject = project;
	self.currentLayer   = self.projectLayer;

	self.currentConfigs = project.configs;
	project.callback();
	self.currentConfigs = nil;

	project.configMap = MBuild.Config.ExtendMap(self.currentWorkspace.configMap, project.configs);
	self:ConfigureWhens(project.configMap, project.whens, self.projectLayer);

	for _, files in ipairs(project.files) do
		self:ConfigureFiles(files, self.projectLayer);
	end

	self.currentProject = nil;
	self.currentLayer   = previousLayer;
end

function MBuild:ConfigureWorkspace(workspace, previousLayer)
	self.currentWorkspace = workspace;
	self.currentLayer     = self.workspaceLayer;

	self.currentConfigs = workspace.configs;
	workspace.callback();
	self.currentConfigs = nil;

	workspace.configMap = MBuild.Config.CreateMap(workspace.configurations, workspace.platforms, workspace.configs);
	self:ConfigureWhens(workspace.configMap, workspace.whens, self.workspaceLayer);

	for _, project in ipairs(workspace.projects) do
		self:ConfigureProject(project, self.workspaceLayer);
	end

	self.currentWorkspace = nil;
	self.currentLayer     = previousLayer;
end

function MBuild:Configure()
	for _, workspace in ipairs(self.workspaces) do
		self:ConfigureWorkspace(workspace, self.globalLayer);
	end

	-- Debug
	for _, workspace in ipairs(self.workspaces) do
		print(string.format("Workspace %s configs:", workspace.name));
		for name, arr in pairs(workspace.configMap) do
			for platform, config in pairs(arr) do
				printf("  [\"%s\"][\"%s\"]: arch = %s, system = %s", name, platform, config.arch, config.system);
				for k, v in pairs(config.configs) do
					printf("    %s = %s", tostring(k), tostring(v));
				end
			end
		end
		print();
	
		for _, project in ipairs(workspace.projects) do
			print(string.format("Project %s configs:", project.name));
			for name, arr in pairs(project.configMap) do
				for platform, config in pairs(arr) do
					printf("  [\"%s\"][\"%s\"]: arch = %s, system = %s", name, platform, config.arch, config.system);
					for k, v in pairs(config.configs) do
						printf("    %s = %s", tostring(k), tostring(v));
					end
				end
			end
			print();
	
			for _, files in ipairs(project.files) do
				for _, inclusion in ipairs(files.inclusions) do
					printf("Files %s", inclusion);
				end
				for _, exclusion in ipairs(files.exclusions) do
					printf("Excluding files %s", exclusion);
				end
				print("Configs:");
	
				for name, arr in pairs(files.configMap) do
					for platform, config in pairs(arr) do
						printf("  [\"%s\"][\"%s\"]: arch = %s, system = %s", name, platform, config.arch, config.system);
						for k, v in pairs(config.configs) do
							printf("    %s = %s", tostring(k), tostring(v));
						end
					end
				end
				print();
			end
		end
	end
end