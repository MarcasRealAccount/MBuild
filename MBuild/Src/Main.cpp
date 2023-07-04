#include <lua.hpp>

#include <Build.h>

#include <cstdint>
#include <cstdio>

#include <stdexcept>

static int WrapExceptions(lua_State* luaState, lua_CFunction f)
{
	try
	{
		return f(luaState);
	}
	catch (const char* s)
	{
		lua_pushstring(luaState, s);
	}
	catch (const std::exception& e)
	{
		lua_pushstring(luaState, e.what());
	}
	catch (...)
	{
		lua_pushliteral(luaState, "caught (...)");
	}
	return lua_error(luaState);
}

static void DumpStack(lua_State* luaState)
{
	std::printf("\n");

	int top    = lua_gettop(luaState);
	int bottom = 1;
	lua_getglobal(luaState, "tostring");
	for (int i = top; i >= bottom; --i)
	{
		lua_pushvalue(luaState, -1);
		lua_pushvalue(luaState, i);
		lua_pcall(luaState, 1, 1, 0);
		const char* str = lua_tostring(luaState, -1);
		if (str)
			std::printf("%s\n", str);
		else
			std::printf("%s\n", luaL_typename(luaState, i));
		lua_pop(luaState, 1);
	}
	lua_pop(luaState, 1);
}

static int osHost(lua_State* luaState)
{
#if BUILD_IS_SYSTEM_WINDOWS
	lua_pushstring(luaState, "windows");
#elif BUILD_IS_SYSTEM_MACOSX
	lua_pushstring(luaState, "macosx");
#elif BUILD_IS_SYSTEM_LINUX
	lua_pushstring(luaState, "linux");
#else
	lua_pushstring(luaState, "unknown");
#endif
	return 1;
}

static int osArch(lua_State* luaState)
{
#if BUILD_IS_PLATFORM_AMD64
	lua_pushstring(luaState, "x86-64");
#else
	lua_pushstring(luaState, "unknown");
#endif
	return 1;
}

int main(int argc, char** argv)
{
	lua_State* luaState = luaL_newstate();

	luaJIT_setmode(luaState, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);

	lua_pushlightuserdata(luaState, &WrapExceptions);
	luaJIT_setmode(luaState, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_ON);
	lua_pop(luaState, 1);

	luaL_openlibs(luaState);

	lua_getglobal(luaState, "os");
	lua_pushcfunction(luaState, &osHost);
	lua_setfield(luaState, -2, "host");
	lua_pushcfunction(luaState, &osArch);
	lua_setfield(luaState, -2, "arch");
	lua_pop(luaState, 1);

	if (luaL_loadfile(luaState, "API/API.lua"))
	{
		std::fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(luaState, -1));
		return 1;
	}

	if (lua_pcall(luaState, 0, 0, 0))
	{
		std::fprintf(stderr, "Failed to run script: %s\n", lua_tostring(luaState, -1));
		return 1;
	}

	lua_getglobal(luaState, "MBuild");
	lua_getfield(luaState, -1, "InvokeMainScript");
	lua_pushvalue(luaState, -2);
	lua_pushstring(luaState, "MBuild.lua");
	if (lua_pcall(luaState, 2, 0, 0))
	{
		std::fprintf(stderr, "%s\n", lua_tostring(luaState, -1));
		return 1;
	}

	lua_getfield(luaState, -1, "Configure");
	lua_pushvalue(luaState, -2);
	if (lua_pcall(luaState, 1, 0, 0))
	{
		std::fprintf(stderr, "%s\n", lua_tostring(luaState, -1));
		return 1;
	}

	lua_pop(luaState, 1);

	lua_close(luaState);
	return 0;
}