#pragma once
#include "path_util.h"
#include <Windows.h>

namespace {
	template<typename Fun>
	bool list_dir_internal(std::vector<std::string> &result, const std::string path, const std::string& pattern, Fun cb)
	{
		WIN32_FIND_DATA find_data;

		HANDLE handle = FindFirstFileA((path + "\\" + pattern).c_str(), &find_data);

		if (handle == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		do
		{
			if (cb(find_data))
			{
				result.push_back(path + "\\" + find_data.cFileName);
			}
		} while (FindNextFile(handle, &find_data) != 0);


		DWORD err = GetLastError();
		if (err != ERROR_NO_MORE_FILES)
		{
			return false;
		}

		FindClose(handle);
		return true;
	}
}

bool list_dir(std::vector<std::string> &result, const std::string& path, const std::string& pattern, bool recursive)
{
	if (recursive) {
		std::vector<std::string> dirs{};
		list_dir_internal(dirs, path, "*", [](const WIN32_FIND_DATA& find_data) {
			return (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && find_data.cFileName[0] != '.' ;
		});

		for (auto &dir : dirs)
		{
			list_dir(result, dir, pattern, recursive);
		}
	}

	list_dir_internal(result, path, pattern, [](const WIN32_FIND_DATA& find_data) {
		return !(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	});

	return true;
}