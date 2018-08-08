#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <signal.h>

#include "sysfs.h"
#include "subcore.h"

Subcore subcore;

void onexit_handler(int signum) {
	if (subcore.debug)
		std::cout << "[*] Restoring user settings" << std::endl;
	subcore.user_settings.load();
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
  for (uint16_t x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
		close (x);
  }

}

int main(int argc, char** argv) {
	// fork self
	deamon();

	// perform the root check
	if (!Root::is_root()) {
		std::cout << "[!] EUID is not 0. Please run this with root privileges." << std::endl;
		std::exit(1);
	}

	// setup the presets based on the device
	subcore.setup_levels();
	
	// scan for command line args
	int opt;
	while ((opt = getopt(argc, argv, "dmp")) != -1) {
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
			case '?':  // unknown option...
				exit(1);
				break;
		}
	}

	// user settings managenent
	if (subcore.debug)
		std::cout << "[*] Saving user settings" << std::endl;
	subcore.user_settings.save();

	// register exit handler to restore settings
	signal(SIGINT, onexit_handler);
	signal(SIGTERM, onexit_handler);

	if (subcore.debug)
		std::cout << "[*] SubCore Init" << std::endl;

	// start the algorithm itself
	while (1) {
		subcore.algorithm();
	}	
	
	return 0;
}

