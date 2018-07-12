#include <iostream>
#include <string>
#include <vector>

#include "sysfs.h"
#include "subcore.h"

SysFs::Cpu cpu;
SysFs::Block block;
SysFs::Gpu gpu;
SysFs::Battery battery;
Subcore subcore;

void test_alg() {
	int load = cpu.get_loadavg();
	std::cout << "[*] Load: " << std::to_string(load) << std::endl;

	//special cases
	int capacity = battery.capacity();
	bool charging = battery.charging();

	if (charging) {
		// alwats use highest load based algorithm
		load = 100;
	} else if (capacity <= 15) {
		// always use lowest load based algorithm
		load = 0;
	} else if (capacity <= 25) {
		// always use idle or lower
		load = (load > cpu.LOAD_IDLE_THRESH) ? cpu.LOAD_IDLE_THRESH : load;
	}

	// load based algorithm
	if (load <= cpu.LOAD_POWERSAVE_THRESH) {
		subcore.set_sysfs(subcore.powersave);
		std::cout << "[*] powersave" << std::endl;
	} else if (load <= cpu.LOAD_IDLE_THRESH) {
		subcore.set_sysfs(subcore.idle);
		std::cout << "[*] idle" << std::endl;
	} else if (load <= cpu.LOAD_LOW_LAT_THRESH) {
		subcore.set_sysfs(subcore.low_lat);
		std::cout << "[*] low_lat" << std::endl;
	} else if (load <= cpu.LOAD_PERFORMANCE_THRESH) {
		subcore.set_sysfs(subcore.performance);
		std::cout << "[*] performance" << std::endl;
	} else {
		std::cout << "[*] other" << std::endl;
	}
}

int main(int argc, const char** argv) {
	std::cout << "[*] SubCore Init" << std::endl;
	subcore.setup_presets();
	while (1) {
		test_alg();
	}	
	
	return 0;
}

/* Goal: Increase efficiency while conserving power based on usage.	
 * [MAX POWER]
 * - Set ioshced to noop
 * - Set CPU governor to powersave
 * - Set max freq to min
 * - Set GPU freq to min + 2
 * - Set LMK to Aggressive
 * - Make subcore scan less frequent
 * - Reduce readahead to 128KB
 * - Set swappiness to 0
 * [LOW LAT]
 * - Set iosched to deadline
 * - Set CPU governor to interactive or ondemand
 * - Set max freq to max - 2
 * - set GPU freq to max - 2
 * - Set LMK to Medium
 * - Make subcore scan normally
 * - Set readahead to 128KB
 * - Set swappiness to 25
 * [PERFORMANCE]
 * - Set iosched to deadline
 * - Set CPU governor to ondemand
 * - Set max freq to max
 * - Set GPU max freq to max
 * - Set LMK to Very Light
 * - Make subcore scan normally
 * - Set readahead to 2048KB
 * - Set swappiness to 25
 */
