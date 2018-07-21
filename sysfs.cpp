#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iterator>
#include <sys/types.h>
#include <dirent.h>
#include <algorithm>
#include <cstdio>
#include <unistd.h>

#include "misc.h"
#include "sysfs.h"

std::vector<uint32_t> SysFs::Cpu::get_freqs(uint16_t core) {
	// read and split avail freqs
	std::string freq_list_str = IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_available_frequencies");
	try {
		std::istringstream iss(freq_list_str);
		std::vector<std::string> freq_list_split((std::istream_iterator<std::string>(iss)),
				std::istream_iterator<std::string>());
		std::vector<uint32_t> freq_list;

		// iterate through freq list
		for (std::string freq_str : freq_list_split) {
			freq_list.push_back(atoi(freq_str.c_str()));
		}

		// sort from least to greatest
		std::sort(freq_list.begin(), freq_list.end());
		return freq_list;
	} catch (...) {
		return std::vector<uint32_t> {};
	}
}

std::vector<std::string> SysFs::Cpu::get_govs() {
	// read and split avail freqs
	std::string freq_list_str = IO::read_file(PATH_CPU + "/cpu0" + "/cpufreq/scaling_available_governors");
	try {
		std::istringstream iss(freq_list_str);
		return std::vector<std::string>((std::istream_iterator<std::string>(iss)),
				std::istream_iterator<std::string>());
	} catch (...) {
		return std::vector<std::string> {};
	}
}

uint8_t SysFs::Cpu::get_loadavg() {
	long double a[4], b[4], loadavg;
	FILE *fp;
	try {
		// take first measurement
		fp = std::fopen(PATH_STAT.c_str(), "r");
		fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
		fclose(fp);
	
		// sleep
		usleep(STAT_AVG_SLEEP_MS * 1000);

		// take second measurement
		fp = std::fopen(PATH_STAT.c_str(), "r");
		fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);
		fclose(fp);

		// calculate avg based on measurements
		loadavg = ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));
		loadavg *= 100;
	} catch (...) {
		std::cout << "[!] Error: get_loadavg() failed. Device may be unsupported." << std::endl;
		exit(1);
	}

	// limit bounds
	if (loadavg > 100)
		return (uint8_t) 100;
	if (loadavg < 0)
		return (uint8_t) 0;

	return (uint8_t) (loadavg);
}

std::vector<uint16_t> SysFs::Gpu::get_freqs() {
	// read and split avail freqs
	std::string freq_list_str = IO::read_file(PATH_GPU + "/gpu_freq_table");
	try {
		std::istringstream iss(freq_list_str);
		std::vector<std::string> freq_list_split((std::istream_iterator<std::string>(iss)),
				std::istream_iterator<std::string>());
		std::vector<uint16_t> freq_list;
	
		// iterate through freq list
		for (std::string freq_str : freq_list_split) {
			freq_list.push_back(atoi(freq_str.c_str()));
		}
	
		// sort in order of least to greatest
		std::sort(freq_list.begin(), freq_list.end());
		return freq_list;
	} catch (...) {
		return std::vector<uint16_t> {};
	}
}

uint8_t SysFs::Gpu::get_load() {
	std::string str = IO::read_file(PATH_GPU + "/gpu_load");
	try {
		std::istringstream iss(str);
		std::vector<std::string> str_split((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
		return stoi(str_split[0]);
	} catch (...) {
		return 0;
	}
}

std::vector<std::string> SysFs::Block::get_blkdevs() {
	DIR* d;
	std::vector<std::string> usable_blkdevs;
	struct dirent* dir;

	if ((d = opendir("/sys/block/")) == NULL)
			return {};

	// get MMC and SDX devices only
	std::vector<std::string> prefixes = {"mmcblk", "sd"};

	// excluse these because they are not what we are looking for
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

uint32_t SysFs::Memory::get_ram_size() {
	std::string str = IO::read_file(PATH_MEMINFO);
	try {
		std::istringstream iss(str);
		std::vector<std::string> str_split((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
		return stoi(str_split[1]);
	} catch (...) {
		return 0;
	}
}

uint32_t SysFs::Memory::get_avail_ram() {
	std::string str = IO::read_file(PATH_MEMINFO);
	try {
		std::istringstream iss(str);
		std::vector<std::string> str_split((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
		return stoi(str_split[4]);
	} catch (...) {
		return 0;
	}
}

bool SysFs::Display::get_suspended() {
	try {
		if (IO::path_exists(PATH_STATE_NOTIFIER)) {
			std::string str = IO::read_file(PATH_STATE_NOTIFIER + "/state_suspended");
			return (str.find("Y") != std::string::npos);
		} else if (IO::path_exists(PATH_POWER_SUSPEND)) {
			std::string str = IO::read_file(PATH_POWER_SUSPEND + "/power_suspend_state");
			return (str.find("1") != std::string::npos);
		} else if (IO::path_exists(PATH_FB0)) {
			std::string str = IO::read_file(PATH_FB0 + "/idle_notify");
			return (str.find("yes") != std::string::npos);
		}

		// not accessible
		return false;
	} catch (...) {
		return false;
	}
}

