MBuild = MBuild or {
	workspaces       = {},
	currentScript    = nil,
	currentWorkspace = nil,
	currentProject   = nil,
	currentConfig    = nil,
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

function MBuild.DeepCopy(orig, copies)
	copies = copies or {};
	local origType = type(orig);
	if origType == "table" then
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