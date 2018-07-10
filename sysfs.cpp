#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iterator>
#include <sys/types.h>
#include <dirent.h>
#include <algorithm>

#include "misc.h"
#include "sysfs.h"

std::vector<int> SysFs::Cpu::get_freqs(int core) {
	// read and split avail freqs
	std::string freq_list_str = IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_available_frequencies");
	std::istringstream iss(freq_list_str);
	std::vector<std::string> freq_list_split((std::istream_iterator<std::string>(iss)),
			std::istream_iterator<std::string>());
	std::vector<int> freq_list;

	// iterate through freq list
	for (std::string freq_str : freq_list_split) {
		freq_list.push_back(atoi(freq_str.c_str()));
	}

	return freq_list;
}

void SysFs::Cpu::set_max_freq(int core, int freq) {
	IO::write_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_max_freq", std::to_string(freq));
}

int SysFs::Cpu::get_max_freq(int core) {
	std::string str = IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_max_freq");
	return atoi(str.c_str());
}

void SysFs::Cpu::set_min_freq(int core, int freq) {
	IO::write_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_min_freq", std::to_string(freq));
}

int SysFs::Cpu::get_min_freq(int core) {
	std::string str = IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_min_freq");
	return atoi(str.c_str());
}

int SysFs::Cpu::get_present() {
	std::string present_str = IO::read_file(PATH_CPU + "/present");
	return ((int) present_str.at(2) - '0') + 1;
}

void SysFs::Cpu::set_gov(int core, std::string gov) {
	IO::write_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_governor", gov);
}

std::string SysFs::Cpu::get_gov(int core) {
	return IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_governor");
}

std::vector<std::string> SysFs::Block::get_blkdevs() {
	DIR* d;
	std::vector<std::string> usable_blkdevs;
	struct dirent* dir;

	if ((d = opendir("/sys/block/")) == NULL)
			return {};

	std::vector<std::string> prefixes = {"mmcblk", "sd"};
	std::vector<std::string> prefix_excls = {"mmcblk0rpmb"};

loopstart:
	// list each subdirectory
	while ((dir = readdir(d)) != NULL) {
		// check for the desired prefix
		for (std::string prefix : prefixes) {
			if (!std::string(dir->d_name).compare(0, prefix.size(), prefix)) {
				// check for exclusions
				for (std::string excl : prefix_excls)
					if (!std::string(dir->d_name).compare(excl))
						goto loopstart;
				// add to array
				usable_blkdevs.push_back(std::string(dir->d_name));
			}
		}
	}
	return usable_blkdevs;
}

void SysFs::Block::set_iosched(std::string blkdev, std::string iosched) {
	IO::write_file(PATH_BLOCK + "/" + blkdev + "/queue/scheduler", iosched);
}

std::string SysFs::Block::get_iosched(std::string blkdev) {
	return IO::read_file(PATH_BLOCK + "/" + blkdev + "/queue/scheduler");
}

void SysFs::Block::set_lmk_adj(std::string adj) {
	IO::write_file(PATH_LMK + "/adj", adj);
}

std::string SysFs::Block::get_lmk_adj() {
	return IO::read_file(PATH_LMK + "/adj");
}

void SysFs::Block::set_read_ahead(std::string blkdev, int read_ahead) {
	IO::write_file(PATH_BLOCK + "/" + blkdev + "/queue/read_ahead_kb", std::to_string(read_ahead));
}

int SysFs::Block::get_read_ahead(std::string blkdev) {
	std::string str = IO::read_file(PATH_BLOCK + "/" + blkdev + "/queue/read_ahead_kb");
	return stoi(str);
}

