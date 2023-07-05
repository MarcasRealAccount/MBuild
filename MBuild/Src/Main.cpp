#include <lua.hpp>

#include <Build.h>

#include <cstdint>
#include <cstdio>

#include <filesystem>
#include <stdexcept>

static int WrapExceptions(lua_State* L, lua_CFunction f)
{
	try
	{
		return f(L);
	}
	catch (const char* s)
	{
		lua_pushstring(L, s);
	}
	catch (const std::exception& e)
	{
		lua_pushstring(L, e.what());
	}
	catch (...)
	{
		lua_pushliteral(L, "caught (...)");
	}
	return lua_error(L);
}

static int DumpStack(lua_State* L)
{
	std::printf("-- Stack\n");

	int top    = lua_gettop(L);
	int bottom = 1;
	lua_getglobal(L, "tostring");
	for (int i = top; i >= bottom; --i)
	{
		lua_pushvalue(L, -1);
		lua_pushvalue(L, i);
		lua_pcall(L, 1, 1, 0);
		const char* str = lua_tostring(L, -1);
		if (str)
			std::printf("%s\n", str);
		else
			std::printf("%s\n", luaL_typename(L, i));
		lua_pop(L, 1);
	}
	lua_pop(L, 1);

	std::printf("   Stack --\n");
	return 0;
}

static int DumpSources(lua_State* L)
{
	std::printf("-- Sources\n");
	for (int i = 1; i <= 10; ++i)
	{
		lua_Debug debug {};
		if (!lua_getstack(L, i, &debug))
			break;
		lua_getinfo(L, "S", &debug);

		std::printf("%d: %s\n", i, debug.source);
	}
	std::printf("   Sources --\n\n");
	return 0;
}

static int osHost(lua_State* L)
{
#if BUILD_IS_SYSTEM_WINDOWS
	lua_pushstring(L, "windows");
#elif BUILD_IS_SYSTEM_MACOSX
	lua_pushstring(L, "macosx");
#elif BUILD_IS_SYSTEM_LINUX
	lua_pushstring(L, "linux");
#else
	lua_pushstring(L, "unknown");
#endif
	return 1;
}

static int osArch(lua_State* L)
{
#if BUILD_IS_PLATFORM_AMD64
	lua_pushstring(L, "x86-64");
#else
	lua_pushstring(L, "unknown");
#endif
	return 1;
}

extern void AddFilesystemLib(lua_State* state);

int main(int argc, char** argv)
{
	lua_State* L = luaL_newstate();

	luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);

	lua_pushlightuserdata(L, &WrapExceptions);
	luaJIT_setmode(L, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_ON);
	lua_pop(L, 1);

	luaL_openlibs(L);

	AddFilesystemLib(L);

	lua_getglobal(L, "os");
	lua_pushcfunction(L, &osHost);
	lua_setfield(L, -2, "host");
	lua_pushcfunction(L, &osArch);
	lua_setfield(L, -2, "arch");
	lua_pop(L, 1);

	lua_getglobal(L, "debug");
	lua_pushcfunction(L, &DumpSources);
	lua_setfield(L, -2, "dump_sources");
	lua_pushcfunction(L, &DumpStack);
	lua_setfield(L, -2, "dump_stack");
	lua_pop(L, 1);

	auto initFile = std::filesystem::absolute("Base/Init.lua").lexically_normal();
	if (luaL_loadfile(L, initFile.string().c_str()))
	{
		std::fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
		return 1;
	}

	if (lua_pcall(L, 0, 0, 0))
	{
		std::fprintf(stderr, "Failed to run script: %s\n", lua_tostring(L, -1));
		return 1;
	}

	lua_getglobal(L, "MBuild");
	lua_getfield(L, -1, "InvokeMainScript");
	lua_pushvalue(L, -2);
	lua_pushstring(L, "MBuild.lua");
	if (lua_pcall(L, 2, 0, 0))
	{
		std::fprintf(stderr, "%s\n", lua_tostring(L, -1));
		return 1;
	}

	/*lua_getfield(L, -1, "Configure");
	lua_pushvalue(L, -2);
	if (lua_pcall(L, 1, 0, 0))
	{
		std::fprintf(stderr, "%s\n", lua_tostring(L, -1));
		return 1;
	}*/

	lua_pop(L, 1);

	lua_close(L);
	return 0;
}