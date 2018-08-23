#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <signal.h>

#include "sysfs.h"
#include "subcore.h"

subcore subcore;

void onexit_handler(int signum) {
	if (subcore.debug)
		std::cout << "[*] Restoring user settings" << std::endl;
	subcore.settings.load();
	std::exit(signum);
}

void deamon() {
	pid_t pid = fork();

	if (pid < 0)
		exit(EXIT_FAILURE);
	else if (pid > 0)
		exit(EXIT_SUCCESS);
	
	if (setsid() < 0)
		exit(EXIT_FAILURE);

	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	pid = fork();
	if (pid < 0)
		exit(EXIT_FAILURE);
	else if (pid > 0)
		exit(EXIT_SUCCESS);

	umask(0);
	chdir("/");
	for (int32_t x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
		close (x);

}

int main(int argc, char** argv) {	
	// scan for command line args
	int16_t opt;
	bool fork = true;
	while ((opt = getopt(argc, argv, "dmpf")) != -1) {
		switch (opt) {
			case 'd':
				std::cout << "[*] Debug enabled" << std::endl;
				subcore.debug = true;
				break;
			case 'm':
				if (subcore.debug)
					std::cout << "[*] Low-Memory enabled" << std::endl;
				subcore.low_mem = true;
				break;
			case 'p':
				if (subcore.debug)
					std::cout << "[*] Power-Aware disabled" << std::endl;
				subcore.power_aware = false;
				break;
			case 'f':
				if (subcore.debug)
					std::cout << "[*] Not forking" << std::endl;
				fork = false;
				break;
			case '?':  // unknown option...
				exit(1);
				break;
		}
	}

	if (fork)
		deamon();

	// register exit handler to restore settings
	signal(SIGINT, onexit_handler);
	signal(SIGTERM, onexit_handler);

	// perform the root check
	if (!root::is_root()) {
		std::cout << "[!] EUID is not 0. Please run this with root privileges." << std::endl;
		std::exit(1);
	}	

	// user settings managenent
	if (subcore.debug)
		std::cout << "[*] Saving user settings" << std::endl;
	subcore.settings.save();

	// setup the presets based on the device
	if (subcore.debug)
		std::cout << "[*] Generating level profiles" << std::endl;
	subcore.setup_levels();

	if (subcore.debug)
		std::cout << "[*] Initializing algorithm loop" << std::endl;

	// start the algorithm itself
	while (1) {
		subcore.algorithm();
	}	
	
	return 0;
}

