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
}

std::vector<std::string> SysFs::Cpu::get_govs() {
	// read and split avail freqs
	std::string freq_list_str = IO::read_file(PATH_CPU + "/cpu0" + "/cpufreq/scaling_available_governors");
	std::istringstream iss(freq_list_str);
	return std::vector<std::string>((std::istream_iterator<std::string>(iss)),
			std::istream_iterator<std::string>());
}

uint8_t SysFs::Cpu::get_loadavg() {
	long double a[4], b[4], loadavg;
	FILE *fp;

	fp = std::fopen(PATH_STAT.c_str(), "r");
	fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
	fclose(fp);
	
	usleep(STAT_AVG_SLEEP_MS * 1000);

	fp = std::fopen(PATH_STAT.c_str(), "r");
	fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);
	fclose(fp);

	loadavg = ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));
	loadavg *= 100;

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

