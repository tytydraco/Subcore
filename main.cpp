#include <iostream>
#include <string>
#include <vector>

#include "sysfs.h"

int main(int argc, const char** argv) {
	SysFs sysfs;
	SysFs::Cpu cpu;
	SysFs::Block block;

	return 0;
}

/* Goal: Increase efficiency while conserving power based on usage.	
 * [IDLE]
 * - Set ioshced to noop
 * - Set CPU governor to powersave
 * - Set max freq to min
 * - Set min freq to min
 * - Set GPU freq to max - 2
 * - Set LMK to High
 * - Make subcore scan less frequent
 * - Reduce readahead to 128KB
 * - Set swappiness to 0
 * [AWAKE]
 * - Set iosched to deadline
 * - Set CPU governor to conservative or interactive based on battery % and usage
 * - Set max freq based on battery % and usage
 * - set min freq to min
 * - set GPU freq based on battery %
 * - Set LMK to Light
 * - Make subcore scan normally
 * - Set readahead to 512KB
 * - Set swappiness to 25 
 */
void alg() {
	
}

