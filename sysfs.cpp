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

std::vector<uint32_t> SysFs::Cpu::get_freqs(uint8_t core) {
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

	std::sort(freq_list.begin(), freq_list.end());
	return freq_list;
}

void SysFs::Cpu::set_max_freq(uint8_t core, uint32_t freq) {
	IO::write_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_max_freq", std::to_string(freq));
}

uint32_t SysFs::Cpu::get_max_freq(uint8_t core) {
	std::string str = IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_max_freq");
	return (uint32_t) atoi(str.c_str());
}

void SysFs::Cpu::set_min_freq(uint8_t core, uint32_t freq) {
	IO::write_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_min_freq", std::to_string(freq));
}

uint32_t SysFs::Cpu::get_min_freq(uint8_t core) {
	std::string str = IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_min_freq");
	return (uint32_t) atoi(str.c_str());
}

uint8_t SysFs::Cpu::get_present() {
	std::string present_str = IO::read_file(PATH_CPU + "/present");
	return ((uint8_t) present_str.at(2) - '0') + 1;
}

void SysFs::Cpu::set_gov(uint8_t core, std::string gov) {
	IO::write_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_governor", gov);
}

std::string SysFs::Cpu::get_gov(uint8_t core) {
	return IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_governor");
}

uint8_t SysFs::Cpu::get_loadavg() {
	long double a[4], b[4], loadavg;
	FILE *fp;
	char dump[50];

	fp = std::fopen(PATH_STAT.c_str(), "r");
	fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
	fclose(fp);
	usleep(STAT_AVG_SLEEP_MS * 1000);

	fp = std::fopen(PATH_STAT.c_str(), "r");
	fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);
	fclose(fp);

	loadavg = ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));
	loadavg *= 100;

	if (loadavg > 100)
		loadavg = 100;
	if (loadavg < 0)
		loadavg = 0;
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

	std::sort(freq_list.begin(), freq_list.end());
	return freq_list;
}

void SysFs::Gpu::set_max_freq(uint16_t freq) {
	IO::write_file(PATH_GPU + "/gpu_max_clock", std::to_string(freq));
}

uint16_t SysFs::Gpu::get_max_freq() {
	std::string str = IO::read_file(PATH_GPU + "/gpu_max_clock");
	return (uint16_t) stoi(str);
}

void SysFs::Gpu::set_min_freq(uint16_t freq) {
	IO::write_file(PATH_GPU + "/gpu_min_clock", std::to_string(freq));
}

uint16_t SysFs::Gpu::get_min_freq() {
	std::string str = IO::read_file(PATH_GPU + "/gpu_min_clock");
	return (uint16_t) stoi(str);
}

void SysFs::Gpu::set_gov(std::string gov) {
	IO::write_file(PATH_GPU + "/gpu_governor", gov);
}

std::string SysFs::Gpu::get_gov() {
	std::string str = IO::read_file(PATH_GPU + "/gpu_governor");
	return str;
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

void SysFs::Block::set_lmk(std::string minfree) {
	IO::write_file(PATH_LMK + "/minfree", minfree);
}

std::string SysFs::Block::get_lmk() {
	return IO::read_file(PATH_LMK + "/minfree");
}

void SysFs::Block::set_read_ahead(std::string blkdev, uint16_t read_ahead) {
	IO::write_file(PATH_BLOCK + "/" + blkdev + "/queue/read_ahead_kb", std::to_string(read_ahead));
}

uint16_t SysFs::Block::get_read_ahead(std::string blkdev) {
	std::string str = IO::read_file(PATH_BLOCK + "/" + blkdev + "/queue/read_ahead_kb");
	return (uint16_t) stoi(str);
}

void SysFs::Block::set_swappiness(uint8_t swappiness) {
	IO::write_file(PATH_VM + "/swappiness", std::to_string(swappiness));
}

uint8_t SysFs::Block::get_swappiness() {
	std::string str = IO::read_file(PATH_VM + "/swappiness");
	return (uint8_t) stoi(str);
}

void SysFs::Block::set_cache_pressure(uint8_t pressure) {
		IO::write_file(PATH_VM + "/vfs_cache_pressure", std::to_string(pressure));
}

uint8_t SysFs::Block::get_cache_pressure() {
	std::string str = IO::read_file(PATH_VM + "/vfs_cache_pressure");
	return (uint8_t) stoi(str);
}

void SysFs::Block::set_dirty_ratio(uint8_t ratio) {
		IO::write_file(PATH_VM + "/dirty_ratio", std::to_string(ratio));
}

uint8_t SysFs::Block::get_dirty_ratio() {
	std::string str = IO::read_file(PATH_VM + "/dirty_ratio");
	return (uint8_t) stoi(str);
}

void SysFs::Block::set_dirty_background_ratio(uint8_t ratio) {
		IO::write_file(PATH_VM + "/dirty_background_ratio", std::to_string(ratio));
}

uint8_t SysFs::Block::get_dirty_background_ratio() {
	std::string str = IO::read_file(PATH_VM + "/dirty_background_ratio");
	return (uint8_t) stoi(str);
}

uint8_t SysFs::Battery::capacity() {
	std::string capacity_str = IO::read_file(PATH_BATTERY + "/capacity");
	return (uint8_t) stoi(capacity_str);
}

bool SysFs::Battery::charging() {
	std::string status_str = IO::read_file(PATH_BATTERY + "/status");
	return (status_str.find("Charging") != std::string::npos);
}

