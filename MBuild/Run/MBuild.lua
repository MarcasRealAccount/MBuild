print(MBuild.currentScript);
print(fs.current_script_path());

--[[Workspace("MBuild", function()
	Location("./"); -- Default is './'

	Configurations({ "Debug", "Release", "Dist" });
	Platforms({ "x86-64", "x86" });
	When({}, function()
		Architecture("oof-arch");
		System("oof-os");
	end);

	ObjDir("${workspace.location}/Bin/Int-${config.os}-${config.arch}-${config.name}/");
	BinDir("${workspace.location}/Bin/${config.os}-${config.arch}-${config.name}/");
	RunDir("${project.location}/Run/");

	Project("MBuild", function()
		Location("${workspace.location}/MBuild/"); -- Default is './'
		Warnings("Extra");
		Kind("ConsoleApp");
		
		IncludeDirs({ "${project.location}/Src/" });
		Files({ "${project.location}/Src/**" }, { "*.DS_Store" }, function()
			When("platform == 'x86-64'", function()
				Warnings("Off");
			end);
		end);

		Files({ "${project.location}/Test/**" }, { "*.DS_Store" });

		ExternalDependencies({ "commonbuild", "backtrace", "luajit" });
	end);
end);]]--
