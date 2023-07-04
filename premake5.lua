workspace("MBuild")
	common:addConfigs()
	common:addBuildDefines()

	cppdialect("C++20")
	rtti("Off")
	exceptionhandling("On")
	flags("MultiProcessorCompile")

	startproject("MBuild")
	project("MBuild")
		location("MBuild/")
		warnings("Extra")
		kind("ConsoleApp")

		common:outDirs()
		common:debugDir()

		includedirs({ "%{prj.location}/Src/" })
		files({ "%{prj.location}/Src/**" })
		removefiles({ "*.DS_Store" })

		pkgdeps({ "commonbuild", "backtrace", "luajit" })

		common:addActions()