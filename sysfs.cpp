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

std::vector<uint32_t> SysFs::Cpu::freqs(uint8_t core) {
	// read and split avail freqs
	std::string freq_list_str = IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_available_frequencies");
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

std::vector<std::string> SysFs::Cpu::govs() {
	// read and split avail freqs
	std::string freq_list_str = IO::read_file(PATH_CPU + "/cpu0" + "/cpufreq/scaling_available_governors");
	if (freq_list_str == "")
		return std::vector<std::string> {};
	std::istringstream iss(freq_list_str);
	return std::vector<std::string>((std::istream_iterator<std::string>(iss)),
			std::istream_iterator<std::string>());
}

uint8_t SysFs::Cpu::loadavg() {
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
		std::cout << "[!] Error: loadavg() failed. Device may be unsupported." << std::endl;
		exit(1);
	}

	// limit bounds
	if (loadavg > 100)
		return (uint8_t) 100;
	if (loadavg < 0)
		return (uint8_t) 0;

	return (uint8_t) (loadavg);
}

void SysFs::Cpu::hotplug(std::string name, bool state) {
	std::string new_state = state ? "1" : "0";
	if (name == "thunderplug")
		IO::write_file("/sys/kernel/thunderplug/hotplug_enabled", new_state);
	else if (name == "autosmp")
		IO::write_file("/sys/kernel/autosmp/conf/online", state ? "Y" : "N");	
	else if (name == "blu_plug")
		IO::write_file("/sys/module/blu_plug/parameters/enabled", new_state);	
	else if (name == "msm_hotplug")
		IO::write_file("/sys/module/msm_hotplug/msm_enabled", new_state);	
	else if (name == "intelli_plug")
		IO::write_file("/sys/kernel/intelli_plug/intelli_plug_active", new_state);	
	else if (name == "lazyplug")
		IO::write_file("/sys/module/lazyplug/parameters/lazyplug_active", new_state);
	else if (name == "AiO_HotPlug")
		IO::write_file("/sys/kernel/AiO_HotPlug/toggle", new_state);
	else if (name == "alucard_hotplug")
		IO::write_file("/sys/kernel/alucard_hotplug/hotplug_enable", new_state);
	else if (name == "bricked_hotplug")
		IO::write_file("/sys/kernel/bricked_hotplug/conf/enabled", new_state);
	else if (name == "mako_hotplug_control")
		IO::write_file("/sys/class/misc/mako_hotplug_control/enabled", new_state);
	else if (name == "zen_decision")
		IO::write_file("/sys/kernel/zen_decision/enabled", new_state);
}

std::string SysFs::Cpu::hotplug() {
	if (IO::read_file("/sys/kernel/thunderplug/hotplug_enabled") == "1")
		return "thunderplug";
	else if (IO::read_file("/sys/kernel/autosmp/conf/online") == "Y")
		return "autosmp";
	else if (IO::read_file("/sys/module/blu_plug/parameters/enabled") == "1")
		return "blu_plug";
	else if (IO::read_file("/sys/module/msm_hotplug/msm_enabled") == "1")
		return "msm_hotplug";
	else if (IO::read_file("/sys/kernel/intelli_plug/intelli_plug_active") == "1")
		return "intelli_plug";
	else if (IO::read_file("/sys/module/lazyplug/parameters/lazyplug_active") =="1")
		return "lazyplug";
	else if (IO::read_file("/sys/kernel/AiO_HotPlug/toggle") =="1")
		return "AiO_HotPlug";
	else if (IO::read_file("/sys/kernel/alucard_hotplug/hotplug_enable") =="1")
		return "alucard_hotplug";
	else if (IO::read_file("/sys/kernel/bricked_hotplug/conf/enabled") =="1")
		return "bricked_hotplug";
	else if (IO::read_file("/sys/class/misc/mako_hotplug_control/enabled") =="1")
		return "mako_hotpkug_control";
	else if (IO::read_file("/sys/module/lazyplug/parameters/lazyplug_active") =="1")
		return "zen_decision";

	return "";
}

std::vector<uint16_t> SysFs::Gpu::freqs() {
	// read and split avail freqs
	std::string freq_list_str = IO::read_file(PATH_GPU + "/gpu_freq_table");
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

uint8_t SysFs::Gpu::load() {
	std::string str = IO::read_file(PATH_GPU + "/gpu_load");
	if (str == "")
		return 0;
	std::istringstream iss(str);
	std::vector<std::string> str_split((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
	return stoi(str_split[0]);
}

std::vector<std::string> SysFs::Block::blkdevs() {
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

uint32_t SysFs::Memory::ram_size() {
	std::string str = IO::read_file(PATH_MEMINFO);
	if (str == "")
		return 0;
	std::istringstream iss(str);
	std::vector<std::string> str_split((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
	return stoi(str_split[1]);
}

uint32_t SysFs::Memory::avail_ram() {
	std::string str = IO::read_file(PATH_MEMINFO);
	if (str == "")
		return 0;
	std::istringstream iss(str);
	std::vector<std::string> str_split((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
	return stoi(str_split[4]);
}

bool SysFs::Display::suspended() {
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
}

