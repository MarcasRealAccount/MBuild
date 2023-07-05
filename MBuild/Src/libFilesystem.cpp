#include <lua.hpp>

#include <chrono>
#include <filesystem>

static std::filesystem::directory_options ParseDirectoryOptions(const char* str)
{
	std::filesystem::directory_options options = std::filesystem::directory_options::none;

	char c;
	while ((c = *str) != '\0')
	{
		++str;

		switch (c)
		{
		case 's': options |= std::filesystem::directory_options::follow_directory_symlink; break;
		case 'p': options |= std::filesystem::directory_options::skip_permission_denied; break;
		}
	}
	return options;
}

static std::filesystem::copy_options ParseCopyOptions(const char* str)
{
	std::filesystem::copy_options options = std::filesystem::copy_options::none;

	char c;
	while ((c = *str) != '\0')
	{
		++str;

		switch (c)
		{
		case '>': options |= std::filesystem::copy_options::skip_existing; break;
		case '=': options |= std::filesystem::copy_options::overwrite_existing; break;
		case '+': options |= std::filesystem::copy_options::update_existing; break;

		case 'r': options |= std::filesystem::copy_options::recursive; break;

		case '.': options |= std::filesystem::copy_options::copy_symlinks; break;
		case ',': options |= std::filesystem::copy_options::skip_symlinks; break;

		case 'd': options |= std::filesystem::copy_options::directories_only; break;
		case ':': options |= std::filesystem::copy_options::create_symlinks; break;
		case ';': options |= std::filesystem::copy_options::create_hard_links; break;
		}
	}
	return options;
}

static std::filesystem::perm_options ParsePermOptions(const char* str)
{
	std::filesystem::perm_options options = std::filesystem::perm_options::replace;

	char c;
	while ((c = *str) != '\0')
	{
		++str;

		switch (c)
		{
		case '=': options = std::filesystem::perm_options::replace; break;
		case '+': options = std::filesystem::perm_options::add; break;
		case '-': options = std::filesystem::perm_options::remove; break;
		case 's': options = std::filesystem::perm_options::nofollow; break;
		}
	}

	return options;
}

static std::filesystem::perms ParsePerms(const char* str)
{
	std::filesystem::perms perms = std::filesystem::perms::none;

	char c;
	while ((c = *str) != '\0')
	{
		++str;

		switch (c)
		{
		case 'R': perms = std::filesystem::perms::owner_read; break;
		case 'W': perms = std::filesystem::perms::owner_write; break;
		case 'X': perms = std::filesystem::perms::owner_exec; break;

		case 't': perms = std::filesystem::perms::group_read; break;
		case 'e': perms = std::filesystem::perms::group_write; break;
		case 'c': perms = std::filesystem::perms::group_exec; break;

		case 'r': perms = std::filesystem::perms::others_read; break;
		case 'w': perms = std::filesystem::perms::others_write; break;
		case 'x': perms = std::filesystem::perms::others_exec; break;

		case 'u': perms = std::filesystem::perms::set_uid; break;
		case 'g': perms = std::filesystem::perms::set_gid; break;
		case 's': perms = std::filesystem::perms::sticky_bit; break;
		}
	}

	return perms;
}

static constexpr const char* FileTypeToString(std::filesystem::file_type type)
{
	switch (type)
	{
	case std::filesystem::file_type::none: return "none";
	case std::filesystem::file_type::not_found: return "not_found";
	case std::filesystem::file_type::regular: return "regular";
	case std::filesystem::file_type::directory: return "directory";
	case std::filesystem::file_type::symlink: return "symlink";
	case std::filesystem::file_type::block: return "block";
	case std::filesystem::file_type::character: return "character";
	case std::filesystem::file_type::fifo: return "fifo";
	case std::filesystem::file_type::socket: return "socket";
	case std::filesystem::file_type::unknown: return "unknown";
	default: return "implementation_defined";
	}
}

static constexpr std::string PermsToString(std::filesystem::perms perms)
{
	if (perms == std::filesystem::perms::unknown)
		return "?";

	std::string str;

	if ((perms & std::filesystem::perms::owner_read) != std::filesystem::perms::none) str += 'R';
	if ((perms & std::filesystem::perms::owner_write) != std::filesystem::perms::none) str += 'W';
	if ((perms & std::filesystem::perms::owner_exec) != std::filesystem::perms::none) str += 'X';

	if ((perms & std::filesystem::perms::group_read) != std::filesystem::perms::none) str += 't';
	if ((perms & std::filesystem::perms::group_write) != std::filesystem::perms::none) str += 'e';
	if ((perms & std::filesystem::perms::group_exec) != std::filesystem::perms::none) str += 'c';

	if ((perms & std::filesystem::perms::others_read) != std::filesystem::perms::none) str += 'r';
	if ((perms & std::filesystem::perms::others_write) != std::filesystem::perms::none) str += 'w';
	if ((perms & std::filesystem::perms::others_exec) != std::filesystem::perms::none) str += 'x';

	if ((perms & std::filesystem::perms::set_uid) != std::filesystem::perms::none) str += 'u';
	if ((perms & std::filesystem::perms::set_gid) != std::filesystem::perms::none) str += 'g';
	if ((perms & std::filesystem::perms::sticky_bit) != std::filesystem::perms::none) str += 's';

	return str;
}

static int FSAppend(lua_State* L)
{
	std::filesystem::path lhs, rhs;
	if (lua_isstring(L, 1))
		lhs = lua_tostring(L, 1);
	if (lua_isstring(L, 2))
		rhs = lua_tostring(L, 2);

	lhs /= rhs;

	lua_pushstring(L, lhs.string().c_str());
	return 1;
}

static int FSConcat(lua_State* L)
{
	std::filesystem::path lhs, rhs;
	if (lua_isstring(L, 1))
		lhs = lua_tostring(L, 1);
	if (lua_isstring(L, 2))
		lhs = lua_tostring(L, 2);
	lhs += rhs;

	lua_pushstring(L, lhs.string().c_str());
	return 1;
}

static int FSNormalize(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	std::filesystem::path path = lua_tostring(L, 1);
	path                       = path.lexically_normal();
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSRemoveFilename(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	std::filesystem::path path = lua_tostring(L, 1);
	path.remove_filename();
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSReplaceFilename(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	std::filesystem::path replacement;
	if (lua_isstring(L, 2))
		replacement = lua_tostring(L, 2);

	std::filesystem::path path = lua_tostring(L, 1);
	path.replace_filename(replacement);
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSReplaceExtension(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	std::filesystem::path replacement;
	if (lua_isstring(L, 2))
		replacement = lua_tostring(L, 2);

	std::filesystem::path path = lua_tostring(L, 1);
	path.replace_extension(replacement);
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSCompare(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushinteger(L, 1);
		return 1;
	}
	if (!lua_isstring(L, 2))
	{
		lua_pushinteger(L, -1);
		return 1;
	}

	std::filesystem::path lhs = lua_tostring(L, 1);
	std::filesystem::path rhs = lua_tostring(L, 2);
	lua_pushinteger(L, lhs.compare(rhs));
	return 1;
}

static int FSRootName(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	std::filesystem::path path = lua_tostring(L, 1);
	path                       = path.root_name();
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSRootDirectory(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	std::filesystem::path path = lua_tostring(L, 1);
	path                       = path.root_directory();
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSRootPath(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	std::filesystem::path path = lua_tostring(L, 1);
	path                       = path.root_path();
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSRelativePath(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	std::filesystem::path path = lua_tostring(L, 1);
	path                       = path.relative_path();
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSParentPath(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	std::filesystem::path path = lua_tostring(L, 1);
	path                       = path.parent_path();
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSFilename(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	std::filesystem::path path = lua_tostring(L, 1);
	path                       = path.filename();
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSStem(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	std::filesystem::path path = lua_tostring(L, 1);
	path                       = path.stem();
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSExtension(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	std::filesystem::path path = lua_tostring(L, 1);
	path                       = path.extension();
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSCurrentPath(lua_State* L)
{
	std::error_code ec;
	auto            path = std::filesystem::current_path(ec);
	if (ec)
	{
		lua_pushnil(L);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSCurrentScriptPath(lua_State* L)
{
	int offset = 0;
	if (lua_isnumber(L, 1))
		offset = lua_tonumber(L, 1);

	lua_Debug debug {};
	if (lua_getstack(L, 1 + offset, &debug))
		lua_getinfo(L, "S", &debug);

	std::error_code       ec;
	std::filesystem::path path;

	if (debug.source)
	{
		std::string_view view(debug.source + 1);
		view = view.substr(0, view.find_last_of("\\/") + 1);
		path = view;
		if (path.is_relative())
			path = std::filesystem::absolute(path, ec);
	}
	else
	{
		path = std::filesystem::current_path(ec);
	}

	if (ec)
	{
		lua_pushnil(L);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSAbsolute(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	std::filesystem::path path = lua_tostring(L, 1);
	if (path.is_absolute())
	{
		path = path.lexically_normal();
		lua_pushstring(L, path.string().c_str());
		return 1;
	}

	std::error_code ec;
	path = std::filesystem::absolute(path, ec);
	if (ec)
	{
		lua_pushnil(L);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSAbsoluteScript(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	std::error_code       ec;
	std::filesystem::path path = lua_tostring(L, 1);
	if (path.is_relative())
	{
		int offset = 0;
		if (lua_isnumber(L, 2))
			offset = lua_tonumber(L, 2);

		lua_Debug debug {};
		if (lua_getstack(L, 1 + offset, &debug))
			lua_getinfo(L, "S", &debug);

		if (debug.source)
		{
			std::string_view view(debug.source + 1);
			view                        = view.substr(0, view.find_last_of("\\/") + 1);
			std::filesystem::path path2 = view;
			if (path2.is_relative())
				path2 = std::filesystem::absolute(path2, ec);

			path = path2 / path;
		}
		else
		{
			path = std::filesystem::current_path(ec) / path;
		}
	}
	if (ec)
	{
		lua_pushnil(L);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSRelative(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	if (lua_isstring(L, 2))
	{
		std::error_code ec;
		auto            path = std::filesystem::relative(lua_tostring(L, 1), lua_tostring(L, 2), ec);
		if (ec)
		{
			lua_pushnil(L);
			lua_pushstring(L, ec.message().c_str());
			return 2;
		}
		lua_pushstring(L, path.string().c_str());
		return 1;
	}
	std::error_code ec;
	auto            path = std::filesystem::relative(lua_tostring(L, 1), ec);
	if (ec)
	{
		lua_pushnil(L);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSCanonical(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushstring(L, "");
		return 1;
	}

	std::error_code ec;
	auto            path = std::filesystem::canonical(lua_tostring(L, 1), ec);
	if (ec)
	{
		lua_pushnil(L);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSCopy(lua_State* L)
{
	if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Either from or to is not a valid string");
		return 2;
	}

	std::filesystem::copy_options options = std::filesystem::copy_options::none;
	if (lua_isstring(L, 3))
		options = ParseCopyOptions(lua_tostring(L, 3));

	std::filesystem::path from = lua_tostring(L, 1);
	std::filesystem::path to   = lua_tostring(L, 2);

	std::error_code ec;
	std::filesystem::copy(from, to, options, ec);
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushboolean(L, true);
	return 1;
}

static int FSCopyFile(lua_State* L)
{
	if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Either from or to is not a valid string");
		return 2;
	}

	std::filesystem::copy_options options = std::filesystem::copy_options::none;
	if (lua_isstring(L, 3))
		options = ParseCopyOptions(lua_tostring(L, 3));

	std::filesystem::path from = lua_tostring(L, 1);
	std::filesystem::path to   = lua_tostring(L, 2);

	std::error_code ec;
	bool            res = std::filesystem::copy_file(from, to, options, ec);
	if (res)
	{
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pushboolean(L, false);
	lua_pushstring(L, ec.message().c_str());
	return 2;
}

static int FSCopySymlink(lua_State* L)
{
	if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Either from or to is not a valid string");
		return 2;
	}

	std::filesystem::path from = lua_tostring(L, 1);
	std::filesystem::path to   = lua_tostring(L, 2);

	std::error_code ec;
	std::filesystem::copy_symlink(from, to, ec);
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushboolean(L, true);
	return 1;
}

static int FSCreateDirectory(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	if (std::filesystem::create_directory(lua_tostring(L, 1), ec))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pushboolean(L, false);
	lua_pushstring(L, ec.message().c_str());
	return 2;
}

static int FSCreateDirectories(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	if (std::filesystem::create_directories(lua_tostring(L, 1), ec))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pushboolean(L, false);
	lua_pushstring(L, ec.message().c_str());
	return 2;
}

static int FSCreateHardlink(lua_State* L)
{
	if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Either target or link is not a valid string");
		return 2;
	}

	std::filesystem::path target = lua_tostring(L, 1);
	std::filesystem::path link   = lua_tostring(L, 2);

	std::error_code ec;
	std::filesystem::create_hard_link(target, link, ec);
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushboolean(L, true);
	return 1;
}

static int FSCreateSymlink(lua_State* L)
{
	if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Either target or link is not a valid string");
		return 2;
	}

	std::filesystem::path target = lua_tostring(L, 1);
	std::filesystem::path link   = lua_tostring(L, 2);

	std::error_code ec;
	std::filesystem::create_symlink(target, link, ec);
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushboolean(L, true);
	return 1;
}

static int FSCreateDirectorySymlink(lua_State* L)
{
	if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Either target or link is not a valid string");
		return 2;
	}

	std::filesystem::path target = lua_tostring(L, 1);
	std::filesystem::path link   = lua_tostring(L, 2);

	std::error_code ec;
	std::filesystem::create_directory_symlink(target, link, ec);
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushboolean(L, true);
	return 1;
}

static int FSExists(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	if (std::filesystem::exists(lua_tostring(L, 1), ec))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pushboolean(L, false);
	lua_pushstring(L, ec.message().c_str());
	return 2;
}

static int FSEquivalent(lua_State* L)
{
	if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Either a or b is not a valid string");
		return 2;
	}

	std::filesystem::path a = lua_tostring(L, 1);
	std::filesystem::path b = lua_tostring(L, 2);

	std::error_code ec;
	if (std::filesystem::equivalent(a, b, ec))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pushboolean(L, false);
	lua_pushstring(L, ec.message().c_str());
	return 2;
}

static int FSFileSize(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	std::uintmax_t  fileSize = std::filesystem::file_size(lua_tostring(L, 1), ec);
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushboolean(L, true);
	lua_pushinteger(L, static_cast<lua_Integer>(fileSize));
	return 2;
}

static int FSHardlinkCount(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	std::uintmax_t  count = std::filesystem::hard_link_count(lua_tostring(L, 1), ec);
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushboolean(L, true);
	lua_pushinteger(L, static_cast<lua_Integer>(count));
	return 2;
}

static int FSLastWriteTime(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	if (lua_isnumber(L, 2))
	{
		std::int64_t newTime = static_cast<std::int64_t>(lua_tointeger(L, 2));

		std::error_code ec;
		std::filesystem::last_write_time(lua_tostring(L, 1), std::filesystem::file_time_type::clock::from_utc(std::chrono::utc_time<std::chrono::duration<std::int64_t, std::micro>>(std::chrono::duration<std::int64_t, std::micro>(newTime))), ec);
		if (ec)
		{
			lua_pushboolean(L, false);
			lua_pushstring(L, ec.message().c_str());
			return 2;
		}
		lua_pushboolean(L, true);
		return 1;
	}

	std::error_code ec;
	std::int64_t    time = std::chrono::time_point_cast<std::chrono::duration<std::int64_t, std::micro>, std::chrono::utc_clock>(std::filesystem::file_time_type::clock::to_utc(std::filesystem::last_write_time(lua_tostring(L, 1), ec))).time_since_epoch().count();
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushboolean(L, true);
	lua_pushinteger(L, static_cast<lua_Integer>(time));
	return 2;
}

static int FSPermissions(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	if (!lua_isstring(L, 2))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Perms has to be a string of permissions");
		return 2;
	}

	std::filesystem::perms        perms = ParsePerms(lua_tostring(L, 2));
	std::filesystem::perm_options opts  = std::filesystem::perm_options::replace;
	if (lua_isstring(L, 3))
		opts = ParsePermOptions(lua_tostring(L, 3));

	std::error_code ec;
	std::filesystem::permissions(lua_tostring(L, 1), perms, opts, ec);
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushboolean(L, true);
	return 1;
}

static int FSReadSymlink(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	auto            path = std::filesystem::read_symlink(lua_tostring(L, 1), ec);
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushboolean(L, true);
	lua_pushstring(L, path.string().c_str());
	return 2;
}

static int FSRemove(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	if (std::filesystem::remove(lua_tostring(L, 1), ec))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pushboolean(L, false);
	lua_pushstring(L, ec.message().c_str());
	return 2;
}

static int FSRemoveAll(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	std::uintmax_t  count = std::filesystem::remove_all(lua_tostring(L, 1), ec);
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushboolean(L, true);
	lua_pushinteger(L, static_cast<lua_Integer>(count));
	return 1;
}

static int FSRename(lua_State* L)
{
	if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Either from or to is not a valid string");
		return 2;
	}

	std::filesystem::path from = lua_tostring(L, 1);
	std::filesystem::path to   = lua_tostring(L, 2);

	std::error_code ec;
	std::filesystem::rename(from, to, ec);
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushboolean(L, true);
	return 1;
}

static int FSResizeFile(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}
	if (!lua_tointeger(L, 2))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "NewSize has to be a valid integer");
		return 2;
	}

	std::error_code ec;
	std::filesystem::resize_file(lua_tostring(L, 1), lua_tointeger(L, 2), ec);
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushboolean(L, true);
	return 1;
}

static int FSSpace(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	auto            info = std::filesystem::space(lua_tostring(L, 1), ec);
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}

	lua_pushboolean(L, true);
	lua_createtable(L, 0, 3);
	lua_pushinteger(L, static_cast<lua_Integer>(info.capacity));
	lua_setfield(L, -2, "capacity");
	lua_pushinteger(L, static_cast<lua_Integer>(info.free));
	lua_setfield(L, -2, "free");
	lua_pushinteger(L, static_cast<lua_Integer>(info.available));
	lua_setfield(L, -2, "available");
	return 2;
}

static int FSStatus(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	auto            status = std::filesystem::status(lua_tostring(L, 1), ec);
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}

	lua_pushboolean(L, true);
	lua_createtable(L, 0, 2);
	lua_pushstring(L, FileTypeToString(status.type()));
	lua_setfield(L, -2, "type");
	lua_pushstring(L, PermsToString(status.permissions()).c_str());
	lua_setfield(L, -2, "type");
	return 2;
}

static int FSSymlinkStatus(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	auto            status = std::filesystem::symlink_status(lua_tostring(L, 1), ec);
	if (ec)
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}

	lua_pushboolean(L, true);
	lua_createtable(L, 0, 2);
	lua_pushstring(L, FileTypeToString(status.type()));
	lua_setfield(L, -2, "type");
	lua_pushstring(L, PermsToString(status.permissions()).c_str());
	lua_setfield(L, -2, "type");
	return 2;
}

static int FSTempDirectoryPath(lua_State* L)
{
	std::error_code ec;
	auto            path = std::filesystem::temp_directory_path(ec);
	if (ec)
	{
		lua_pushnil(L);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}
	lua_pushstring(L, path.string().c_str());
	return 1;
}

static int FSIsBlockFile(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	if (std::filesystem::is_block_file(lua_tostring(L, 1), ec))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pushboolean(L, false);
	lua_pushstring(L, ec.message().c_str());
	return 2;
}

static int FSIsCharacterFile(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	if (std::filesystem::is_character_file(lua_tostring(L, 1), ec))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pushboolean(L, false);
	lua_pushstring(L, ec.message().c_str());
	return 2;
}

static int FSIsDirectory(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	if (std::filesystem::is_directory(lua_tostring(L, 1), ec))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pushboolean(L, false);
	lua_pushstring(L, ec.message().c_str());
	return 2;
}

static int FSIsEmpty(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	if (std::filesystem::is_empty(lua_tostring(L, 1), ec))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pushboolean(L, false);
	lua_pushstring(L, ec.message().c_str());
	return 2;
}

static int FSIsFIFO(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	if (std::filesystem::is_fifo(lua_tostring(L, 1), ec))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pushboolean(L, false);
	lua_pushstring(L, ec.message().c_str());
	return 2;
}

static int FSIsOther(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	if (std::filesystem::is_other(lua_tostring(L, 1), ec))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pushboolean(L, false);
	lua_pushstring(L, ec.message().c_str());
	return 2;
}

static int FSIsRegularFile(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	if (std::filesystem::is_regular_file(lua_tostring(L, 1), ec))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pushboolean(L, false);
	lua_pushstring(L, ec.message().c_str());
	return 2;
}

static int FSIsSocket(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	if (std::filesystem::is_socket(lua_tostring(L, 1), ec))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pushboolean(L, false);
	lua_pushstring(L, ec.message().c_str());
	return 2;
}

static int FSIsSymlink(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "Path has to be a valid string");
		return 2;
	}

	std::error_code ec;
	if (std::filesystem::is_symlink(lua_tostring(L, 1), ec))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pushboolean(L, false);
	lua_pushstring(L, ec.message().c_str());
	return 2;
}

static int FSInternalDirectoryIterator(lua_State* L) // Gets invoked by the for iteration logic, returned by FSDirectoryIterator
{
	if (lua_isnil(L, 1))
	{
		lua_pushnil(L);
		return 1;
	}

	std::filesystem::directory_iterator* iter = (std::filesystem::directory_iterator*) lua_touserdata(L, 1);
	if (*iter == std::filesystem::directory_iterator {})
	{
		lua_pushnil(L);
		return 1;
	}

	auto& path = (*iter)->path();
	lua_pushstring(L, path.string().c_str());
	++(*iter);
	return 1;
}

static int FSInternalRecursiveDirectoryIterator(lua_State* L) // Gets invoked by the for iteration logic, returned by FSRecursiveDirectoryIterator
{
	if (lua_isnil(L, 1))
	{
		lua_pushnil(L);
		return 1;
	}

	std::filesystem::recursive_directory_iterator* iter = (std::filesystem::recursive_directory_iterator*) lua_touserdata(L, 1);
	if (*iter == std::filesystem::recursive_directory_iterator {})
	{
		lua_pushnil(L);
		return 1;
	}

	auto& path = (*iter)->path();
	lua_pushstring(L, path.string().c_str());
	++(*iter);
	return 1;
}

static int FSDirectoryIterator(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushcfunction(L, &FSInternalDirectoryIterator);
		lua_pushnil(L);
		lua_pushnil(L);
		return 3;
	}

	std::filesystem::directory_options opts = std::filesystem::directory_options::none;
	if (lua_isstring(L, 2))
		opts = ParseDirectoryOptions(lua_tostring(L, 2));

	std::filesystem::path path = lua_tostring(L, 1);
	std::error_code       ec;

	lua_pushcfunction(L, &FSInternalDirectoryIterator);
	std::filesystem::directory_iterator* iter = (std::filesystem::directory_iterator*) lua_newuserdata(L, sizeof(*iter));
	new (iter) std::filesystem::directory_iterator(path, opts, ec);
	lua_pushnil(L);
	return 3;
}

static int FSRecursiveDirectoryIterator(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		lua_pushcfunction(L, &FSInternalRecursiveDirectoryIterator);
		lua_pushnil(L);
		lua_pushnil(L);
		return 3;
	}

	std::filesystem::directory_options opts = std::filesystem::directory_options::none;
	if (lua_isstring(L, 2))
		opts = ParseDirectoryOptions(lua_tostring(L, 2));

	std::filesystem::path path = lua_tostring(L, 1);
	std::error_code       ec;

	lua_pushcfunction(L, &FSInternalRecursiveDirectoryIterator);
	std::filesystem::recursive_directory_iterator* iter = (std::filesystem::recursive_directory_iterator*) lua_newuserdata(L, sizeof(*iter));
	new (iter) std::filesystem::recursive_directory_iterator(path, opts, ec);
	lua_pushnil(L);
	return 3;
}

void AddFilesystemLib(lua_State* L)
{
	lua_createtable(L, 0, 1);

	lua_pushcfunction(L, &FSAppend);
	lua_setfield(L, -2, "append");
	lua_pushcfunction(L, &FSConcat);
	lua_setfield(L, -2, "concat");
	lua_pushcfunction(L, &FSNormalize);
	lua_setfield(L, -2, "normalize");
	lua_pushcfunction(L, &FSRemoveFilename);
	lua_setfield(L, -2, "remove_filename");
	lua_pushcfunction(L, &FSReplaceFilename);
	lua_setfield(L, -2, "replace_filename");
	lua_pushcfunction(L, &FSReplaceExtension);
	lua_setfield(L, -2, "replace_extension");
	lua_pushcfunction(L, &FSCompare);
	lua_setfield(L, -2, "compare");
	lua_pushcfunction(L, &FSRootName);
	lua_setfield(L, -2, "root_name");
	lua_pushcfunction(L, &FSRootDirectory);
	lua_setfield(L, -2, "root_directory");
	lua_pushcfunction(L, &FSRootPath);
	lua_setfield(L, -2, "root_path");
	lua_pushcfunction(L, &FSRelativePath);
	lua_setfield(L, -2, "relative_path");
	lua_pushcfunction(L, &FSParentPath);
	lua_setfield(L, -2, "parent_path");
	lua_pushcfunction(L, &FSFilename);
	lua_setfield(L, -2, "filename");
	lua_pushcfunction(L, &FSStem);
	lua_setfield(L, -2, "stem");
	lua_pushcfunction(L, &FSExtension);
	lua_setfield(L, -2, "extension");

	lua_pushcfunction(L, &FSCurrentPath);
	lua_setfield(L, -2, "current_path");
	lua_pushcfunction(L, &FSCurrentScriptPath);
	lua_setfield(L, -2, "current_script_path");
	lua_pushcfunction(L, &FSAbsolute);
	lua_setfield(L, -2, "absolute");
	lua_pushcfunction(L, &FSAbsoluteScript);
	lua_setfield(L, -2, "absolute_script");
	lua_pushcfunction(L, &FSRelative);
	lua_setfield(L, -2, "relative");
	lua_pushcfunction(L, &FSCanonical);
	lua_setfield(L, -2, "canonical");
	lua_pushcfunction(L, &FSCopy);
	lua_setfield(L, -2, "copy");
	lua_pushcfunction(L, &FSCopyFile);
	lua_setfield(L, -2, "copy_file");
	lua_pushcfunction(L, &FSCopySymlink);
	lua_setfield(L, -2, "copy_symlink");
	lua_pushcfunction(L, &FSCreateDirectory);
	lua_setfield(L, -2, "create_directory");
	lua_pushcfunction(L, &FSCreateDirectories);
	lua_setfield(L, -2, "create_directories");
	lua_pushcfunction(L, &FSCreateHardlink);
	lua_setfield(L, -2, "create_hardlink");
	lua_pushcfunction(L, &FSCreateSymlink);
	lua_setfield(L, -2, "create_symlink");
	lua_pushcfunction(L, &FSCreateDirectorySymlink);
	lua_setfield(L, -2, "create_directory_symlink");
	lua_pushcfunction(L, &FSExists);
	lua_setfield(L, -2, "exists");
	lua_pushcfunction(L, &FSEquivalent);
	lua_setfield(L, -2, "equivalent");
	lua_pushcfunction(L, &FSFileSize);
	lua_setfield(L, -2, "file_size");
	lua_pushcfunction(L, &FSHardlinkCount);
	lua_setfield(L, -2, "hardlink_count");
	lua_pushcfunction(L, &FSLastWriteTime);
	lua_setfield(L, -2, "last_write_time");
	lua_pushcfunction(L, &FSPermissions);
	lua_setfield(L, -2, "permissions");
	lua_pushcfunction(L, &FSReadSymlink);
	lua_setfield(L, -2, "read_symlink");
	lua_pushcfunction(L, &FSRemove);
	lua_setfield(L, -2, "remove");
	lua_pushcfunction(L, &FSRemoveAll);
	lua_setfield(L, -2, "remove_all");
	lua_pushcfunction(L, &FSRename);
	lua_setfield(L, -2, "rename");
	lua_pushcfunction(L, &FSResizeFile);
	lua_setfield(L, -2, "resize_file");
	lua_pushcfunction(L, &FSSpace);
	lua_setfield(L, -2, "space");
	lua_pushcfunction(L, &FSStatus);
	lua_setfield(L, -2, "status");
	lua_pushcfunction(L, &FSSymlinkStatus);
	lua_setfield(L, -2, "symlink_status");
	lua_pushcfunction(L, &FSTempDirectoryPath);
	lua_setfield(L, -2, "temp_directory_path");
	lua_pushcfunction(L, &FSIsBlockFile);
	lua_setfield(L, -2, "is_block_file");
	lua_pushcfunction(L, &FSIsCharacterFile);
	lua_setfield(L, -2, "is_character_file");
	lua_pushcfunction(L, &FSIsDirectory);
	lua_setfield(L, -2, "is_directory");
	lua_pushcfunction(L, &FSIsEmpty);
	lua_setfield(L, -2, "is_empty");
	lua_pushcfunction(L, &FSIsFIFO);
	lua_setfield(L, -2, "is_fifo");
	lua_pushcfunction(L, &FSIsOther);
	lua_setfield(L, -2, "is_other");
	lua_pushcfunction(L, &FSIsRegularFile);
	lua_setfield(L, -2, "is_regular_file");
	lua_pushcfunction(L, &FSIsSocket);
	lua_setfield(L, -2, "is_socket");
	lua_pushcfunction(L, &FSIsSymlink);
	lua_setfield(L, -2, "is_symlink");
	lua_pushcfunction(L, &FSDirectoryIterator);
	lua_setfield(L, -2, "directory_iterator");
	lua_pushcfunction(L, &FSRecursiveDirectoryIterator);
	lua_setfield(L, -2, "recursive_directory_iterator");

	lua_setglobal(L, "fs");
}