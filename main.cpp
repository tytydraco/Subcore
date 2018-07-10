#include <iostream>
#include <string>
#include <vector>

#include "sysfs.h"

int main(int argc, const char** argv) {
	SysFs sysfs;
	SysFs::Cpu cpu;
	SysFs::Block block;

	block.get_blkdevs();

	return 0;
}
