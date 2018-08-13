#pragma once

#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace IO {
	void write_file(std::string path, std::string in);
	std::string read_file(std::string path);
	uint16_t get_pid(std::string process_name);

	inline bool path_exists(std::string path) {
		struct stat st;
		return (stat(path.c_str(), &st) == 0);
	}
}

namespace Root {
	inline bool is_root() {
		return (geteuid() == 0);
	}
}

