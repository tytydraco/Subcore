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

std::string test_alg() {
	int load = cpu.get_loadavg();
	std::cout << std::to_string(load) << std::endl;
	if (load <= cpu.STAT_AVG_IDLE_THRESH) {
		//subcore.set_sysfs(subcore.idle);
		return "idle";
	} 

	//subcore.set_sysfs(subcore.performance);
	return "other";
}

int main(int argc, const char** argv) {
	std::cout << "[*] SubCore Init" << std::endl;
	subcore.setup_presets();
	while (1) {
		std::cout << test_alg() << std::endl;
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
