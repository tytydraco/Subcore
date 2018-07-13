#include <iostream>
#include <string>
#include <vector>

#include "sysfs.h"
#include "subcore.h"

Subcore subcore;

int main(int argc, const char** argv) {
	std::cout << "[*] SubCore Init" << std::endl;
	subcore.setup_presets();
	
	if (!Root::is_root()) {
		std::cout << "[!] EUID is not 0. Please run this with root privledges." << std::endl;
		std::exit(1);
	}

	while (1) {
		subcore.algorithm();
	}	
	
	return 0;
}

