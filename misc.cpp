#include <string>
#include <unistd.h>
#include <fstream>

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
}

