#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

#include "sysfs.h"
#include "subcore.h"

Subcore subcore;

int main(int argc, char** argv) {
	std::cout << "[*] SubCore Init" << std::endl;
	
	// perform the root check
	if (!Root::is_root()) {
		std::cout << "[!] EUID is not 0. Please run this with root privileges." << std::endl;
		std::exit(1);
	}

	// setup the presets based on the device
	subcore.setup_presets();
	
	// scan for command line args
	int opt;
	while ((opt = getopt(argc, argv, "dm")) != -1) {
		switch (opt) {
			case 'd':
				std::cout << "[*] Debug enabled" << std::endl;
				subcore.debug = true;
				break;
			case 'm':
				std::cout << "[*] Low-Memory enabled" << std::endl;
				subcore.low_mem = true;
			case '?':  // unknown option...
				exit(1);
				break;
		}
	}

	// start the algorithm itself
	while (1) {
		subcore.algorithm();
	}	
	
	return 0;
}

