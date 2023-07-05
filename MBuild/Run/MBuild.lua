Workspace("MBuild", function()
	Location("./"); -- Default is './'

	Configurations({ "Debug", "Release", "Dist" });

	-- Defaults:
	--CompilerMap({ ".cpp", ".c" }, function()
	--	When("system == 'windows' and architecture == 'x86-64'", function() Compiler("MSVC/CL/x64"); end);
	--	When("system == 'windows' and architecture == 'x86'",    function() Compiler("MSVC/CL/x86"); end);
	--	When("system == 'linux' and architecture == 'x86-64'",   function() Compiler("GNU/gcc"); end);
	--	When("system == 'linux' and architecture == 'x86'",      function() Compiler("GNU/gcc"); end);
	--	When("system == 'macosx' and architecture == 'x86-64'",  function() Compiler("LLVM/clang"); end);
	--	When("system == 'macosx' and architecture == 'x86'",     function() Compiler("LLVM/clang"); end);
	--end);
	--

	ObjDir("${workspace.location}/Bin/Int-${config.system}-${config.arch}-${config.name}/");
	BinDir("${workspace.location}/Bin/${config.system}-${config.arch}-${config.name}/");
	RunDir("${project.location}/Run/");

	Project("MBuild", function()
		Location("${workspace.location}/MBuild/"); -- Default is './'
		Warnings("Extra");
		Kind("ConsoleApp");
		
		IncludeDirs({ "${project.location}/Src/" });
		Files({ "${project.location}/Src/**" }, { "*.DS_Store" });
	end);
end);
