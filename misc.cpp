#include <string>
#include <unistd.h>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include <vector>

#include "misc.h"

namespace IO {
	void write_file(std::string path, std::string in) {
		std::ofstream of;
		of.open(path);

		if (!of)
			return;

		of << in;
		of.close();
	}

	std::string read_file(std::string path) {
		std::ifstream ifs(path);

		// return "" and not null to avoid runtime errors
		if (!ifs)
			return "";
	
		return std::string((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
	}

	uint16_t get_pid(std::string process_name) {
		std::vector<std::string> proc_files;
		DIR* dirp = opendir("/proc");
    struct dirent *dp;
		uint16_t pid = getpid();
    while ((dp = readdir(dirp)) != NULL) {
			std::string proc_file = dp->d_name;
			if (proc_file == "." || proc_file == ".." || proc_file == "" || proc_file == std::to_string(pid))
				continue;
			proc_files.push_back(proc_file);
		}
    closedir(dirp);	
		for (std::string proc_file : proc_files) {
			std::string cmdline = read_file("/proc/" + proc_file + "/cmdline");
			if (cmdline.find(process_name) != std::string::npos)
				return stoi(proc_file);
		}
		return 0;
	}
}

