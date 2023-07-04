MBuild = MBuild or {
	workspaces       = {},
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