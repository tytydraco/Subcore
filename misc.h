#ifndef MISC_H
#define MISC_H

#include <string>
namespace IO {
	void write_file(std::string path, std::string in);
	std::string read_file(std::string path);
}

#endif

