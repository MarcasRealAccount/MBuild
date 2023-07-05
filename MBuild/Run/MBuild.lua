Workspace("MBuild", function()
	Location("./"); -- Default is './'

	Configurations({ "Debug", "Release", "Dist" });

	ObjDir("${workspace.location}/Bin/Int-${config.os}-${config.arch}-${config.name}/");
	BinDir("${workspace.location}/Bin/${config.os}-${config.arch}-${config.name}/");
	RunDir("${project.location}/Run/");

	Project("MBuild", function()
		Location("${workspace.location}/MBuild/"); -- Default is './'
		Warnings("Extra");
		Kind("ConsoleApp");
		
		IncludeDirs({ "${project.location}/Src/" });
		Files({ "${project.location}/Src/**" }, { "*.DS_Store" });

		Files({ "${project.location}/Test/**" }, { "*.DS_Store" });

		ExternalDependencies({ "commonbuild", "backtrace", "luajit" });
	end);
end);
