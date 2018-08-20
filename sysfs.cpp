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

std::vector<uint32_t> sysfs::cpu::freqs(uint8_t core) {
	// read and split avail freqs
	std::string freq_list_str = io::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_available_frequencies");
	if (freq_list_str == "")
		return std::vector<uint32_t> {};
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

std::vector<std::string> sysfs::cpu::govs() {
	// read and split avail freqs
	std::string freq_list_str = io::read_file(PATH_CPU + "/cpu0" + "/cpufreq/scaling_available_governors");
	if (freq_list_str == "")
		return std::vector<std::string> {};
	std::istringstream iss(freq_list_str);
	return std::vector<std::string>((std::istream_iterator<std::string>(iss)),
			std::istream_iterator<std::string>());
}

uint8_t sysfs::cpu::loadavg() {
	long double a[4], b[4], loadavg = 0;

	// take first measurement
	FILE *fp = std::fopen(PATH_STAT.c_str(), "r");
	if (fp == NULL)
		return 0;
	fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
	fclose(fp);

	// sleep
	usleep(STAT_AVG_SLEEP_MS * 1000);

	// take second measurement
	fp = std::fopen(PATH_STAT.c_str(), "r");
	if (fp == NULL)
		return 0;
	fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);
	fclose(fp);

	// calculate avg based on measurements
	loadavg = ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));
	loadavg *= 100;

	// limit bounds
	if (loadavg > 100)
		return (uint8_t) 100;
	if (loadavg < 0)
		return (uint8_t) 0;

	return (uint8_t) (loadavg);
}

void sysfs::cpu::hotplug(std::string name, bool state) {
	std::string new_state = state ? "1" : "0";
	if (name == "thunderplug")
		io::write_file("/sys/kernel/thunderplug/hotplug_enabled", new_state);
	else if (name == "autosmp")
		io::write_file("/sys/kernel/autosmp/conf/online", state ? "Y" : "N");	
	else if (name == "blu_plug")
		io::write_file("/sys/module/blu_plug/parameters/enabled", new_state);	
	else if (name == "msm_hotplug")
		io::write_file("/sys/module/msm_hotplug/msm_enabled", new_state);	
	else if (name == "intelli_plug")
		io::write_file("/sys/kernel/intelli_plug/intelli_plug_active", new_state);	
	else if (name == "lazyplug")
		io::write_file("/sys/module/lazyplug/parameters/lazyplug_active", new_state);
	else if (name == "AiO_HotPlug")
		io::write_file("/sys/kernel/AiO_HotPlug/toggle", new_state);
	else if (name == "alucard_hotplug")
		io::write_file("/sys/kernel/alucard_hotplug/hotplug_enable", new_state);
	else if (name == "bricked_hotplug")
		io::write_file("/sys/kernel/bricked_hotplug/conf/enabled", new_state);
	else if (name == "mako_hotplug_control")
		io::write_file("/sys/class/misc/mako_hotplug_control/enabled", new_state);
	else if (name == "zen_decision")
		io::write_file("/sys/kernel/zen_decision/enabled", new_state);
}

std::string sysfs::cpu::hotplug() {
	if (io::read_file("/sys/kernel/thunderplug/hotplug_enabled") == "1")
		return "thunderplug";
	else if (io::read_file("/sys/kernel/autosmp/conf/online") == "Y")
		return "autosmp";
	else if (io::read_file("/sys/module/blu_plug/parameters/enabled") == "1")
		return "blu_plug";
	else if (io::read_file("/sys/module/msm_hotplug/msm_enabled") == "1")
		return "msm_hotplug";
	else if (io::read_file("/sys/kernel/intelli_plug/intelli_plug_active") == "1")
		return "intelli_plug";
	else if (io::read_file("/sys/module/lazyplug/parameters/lazyplug_active") =="1")
		return "lazyplug";
	else if (io::read_file("/sys/kernel/AiO_HotPlug/toggle") =="1")
		return "AiO_HotPlug";
	else if (io::read_file("/sys/kernel/alucard_hotplug/hotplug_enable") =="1")
		return "alucard_hotplug";
	else if (io::read_file("/sys/kernel/bricked_hotplug/conf/enabled") =="1")
		return "bricked_hotplug";
	else if (io::read_file("/sys/class/misc/mako_hotplug_control/enabled") =="1")
		return "mako_hotpkug_control";
	else if (io::read_file("/sys/module/lazyplug/parameters/lazyplug_active") =="1")
		return "zen_decision";

	return "";
}

std::vector<uint16_t> sysfs::gpu::freqs() {
	// read and split avail freqs
	std::string freq_list_str = io::read_file(PATH_GPU + "/gpu_freq_table");
	if (freq_list_str == "")
		return std::vector<uint16_t> {};
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

uint8_t sysfs::gpu::load() {
	std::string str = io::read_file(PATH_GPU + "/gpu_load");
	if (str == "")
		return 0;
	std::istringstream iss(str);
	std::vector<std::string> str_split((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
	return stoi(str_split[0]);
}

std::vector<std::string> sysfs::block::blkdevs() {
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

uint32_t sysfs::memory::ram_size() {
	std::string str = io::read_file(PATH_MEMINFO);
	if (str == "")
		return 0;
	std::istringstream iss(str);
	std::vector<std::string> str_split((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
	return stoi(str_split[1]);
}

uint32_t sysfs::memory::avail_ram() {
	std::string str = io::read_file(PATH_MEMINFO);
	if (str == "")
		return 0;
	std::istringstream iss(str);
	std::vector<std::string> str_split((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
	return stoi(str_split[4]);
}

bool sysfs::display::suspended() {
	if (io::path_exists(PATH_STATE_NOTIFIER)) {
		std::string str = io::read_file(PATH_STATE_NOTIFIER + "/state_suspended");
		return (str.find("Y") != std::string::npos);
	} else if (io::path_exists(PATH_POWER_SUSPEND)) {
		std::string str = io::read_file(PATH_POWER_SUSPEND + "/power_suspend_state");
		return (str.find("1") != std::string::npos);
	} else if (io::path_exists(PATH_FB0)) {
		std::string str = io::read_file(PATH_FB0 + "/idle_notify");
		return (str.find("yes") != std::string::npos);
	}

	// not accessible
	return false;
}

