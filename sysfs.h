#ifndef STATS_H
#define STATS_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>

#include "misc.h"

class SysFs {
	public:
		struct Cpu {
			private:
				const std::string PATH_CPU = "/sys/devices/system/cpu";
				const std::string PATH_STAT = "/proc/stat";
				const uint16_t STAT_AVG_SLEEP_MS = 3000;	
			public:
				const std::string GOV_PREF_POWERSAVE[10] = {
					"powersave"
				};
				const std::string GOV_PREF_IDLE[10] = {
					"energy-dcfc",
					"schedutil",
					"darkness",
					"nightmare",
					"conservative"
				};
				const std::string GOV_PREF_LOW_LAT[10] = {
					"relaxed",
					"chill",
					"interactive",
					"helix_schedutil",
					"schedutil",
					"interactive"
				};
				const std::string GOV_PREF_PERFORMANCE[10] = {
					"ondemand"
				};
				const uint8_t LOAD_POWERSAVE_THRESH = 20;
				const uint8_t LOAD_IDLE_THRESH = 40;
				const uint8_t LOAD_LOW_LAT_THRESH = 60;
				const uint8_t LOAD_PERFORMANCE_THRESH = 100;

				std::vector<uint32_t> get_freqs(uint16_t core);
				std::vector<std::string> get_govs();
				uint8_t get_loadavg();

				inline void set_max_freq(uint16_t core, uint32_t freq) {
					IO::write_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_max_freq", std::to_string(freq));
				}

				inline uint32_t get_max_freq(uint16_t core) {
					std::string str = IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_max_freq");
					return (uint32_t) atoi(str.c_str());
				}

				inline void set_min_freq(uint16_t core, uint32_t freq) {
					IO::write_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_min_freq", std::to_string(freq));
				}

				inline uint32_t get_min_freq(uint16_t core) {
					std::string str = IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_min_freq");
					return (uint32_t) atoi(str.c_str());
				}

				inline uint8_t get_present() {
					std::string present_str = IO::read_file(PATH_CPU + "/present");
					return ((uint8_t) present_str.at(2) - '0') + 1;
				}

				inline void set_gov(uint16_t core, std::string gov) {
					IO::write_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_governor", gov);
				}

				inline std::string get_gov(uint16_t core) {
					return IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_governor");
				}	
		};

		struct Gpu {
			private:
				const std::string PATH_GPU = "/sys/kernel/gpu";
			public:
				std::vector<uint16_t> get_freqs();

				inline void set_max_freq(uint16_t freq) {
					IO::write_file(PATH_GPU + "/gpu_max_clock", std::to_string(freq));
				}

				inline uint16_t get_max_freq() {
					std::string str = IO::read_file(PATH_GPU + "/gpu_max_clock");
					return (uint16_t) stoi(str);
				}

				inline void set_min_freq(uint16_t freq) {
					IO::write_file(PATH_GPU + "/gpu_min_clock", std::to_string(freq));
				}

				inline uint16_t get_min_freq() {
					std::string str = IO::read_file(PATH_GPU + "/gpu_min_clock");
					return (uint16_t) stoi(str);
				}

				inline void set_gov(std::string gov) {
					IO::write_file(PATH_GPU + "/gpu_governor", gov);
				}

				inline std::string get_gov() {
					std::string str = IO::read_file(PATH_GPU + "/gpu_governor");
					return str;
				}
		};

		struct Block {
			private:
				const std::string PATH_BLOCK = "/sys/block";
				const std::string PATH_MOUNTS = "/proc/mounts";
				const std::string PATH_LMK = "/sys/module/lowmemorykiller/parameters";
				const std::string PATH_VM = "/proc/sys/vm";
			public:
				const std::string LMK_VERY_LIGHT = "9169,18338,27507,36676,45845,55014";
				const std::string LMK_AGGRESSIVE = "18338,27507,55014,91690,128366,137535";

				std::vector<std::string> get_blkdevs();

				inline void set_iosched(std::string blkdev, std::string iosched) {
					IO::write_file(PATH_BLOCK + "/" + blkdev + "/queue/scheduler", iosched);
				}

				inline std::string get_iosched(std::string blkdev) {
					return IO::read_file(PATH_BLOCK + "/" + blkdev + "/queue/scheduler");
				}

				inline void set_lmk(std::string minfree) {
					IO::write_file(PATH_LMK + "/minfree", minfree);
				}

				inline std::string get_lmk() {
					return IO::read_file(PATH_LMK + "/minfree");
				}

				inline void set_read_ahead(std::string blkdev, uint16_t read_ahead) {
					IO::write_file(PATH_BLOCK + "/" + blkdev + "/queue/read_ahead_kb", std::to_string(read_ahead));
				}

				inline uint16_t get_read_ahead(std::string blkdev) {
					std::string str = IO::read_file(PATH_BLOCK + "/" + blkdev + "/queue/read_ahead_kb");
					return (uint16_t) stoi(str);
				}

				inline void set_swappiness(uint8_t swappiness) {
					IO::write_file(PATH_VM + "/swappiness", std::to_string(swappiness));
				}

				inline uint8_t get_swappiness() {
					std::string str = IO::read_file(PATH_VM + "/swappiness");
					return (uint8_t) stoi(str);
				}

				inline void set_cache_pressure(uint8_t pressure) {
						IO::write_file(PATH_VM + "/vfs_cache_pressure", std::to_string(pressure));
				}

				inline uint8_t get_cache_pressure() {
					std::string str = IO::read_file(PATH_VM + "/vfs_cache_pressure");
					return (uint8_t) stoi(str);
				}

				inline void set_dirty_ratio(uint8_t ratio) {
						IO::write_file(PATH_VM + "/dirty_ratio", std::to_string(ratio));
				}

				inline uint8_t get_dirty_ratio() {
					std::string str = IO::read_file(PATH_VM + "/dirty_ratio");
					return (uint8_t) stoi(str);
				}

				inline void set_dirty_background_ratio(uint8_t ratio) {
						IO::write_file(PATH_VM + "/dirty_background_ratio", std::to_string(ratio));
				}

				inline uint8_t get_dirty_background_ratio() {
					std::string str = IO::read_file(PATH_VM + "/dirty_background_ratio");
					return (uint8_t) stoi(str);
				}

		};

		struct Battery {
			private:
				const std::string PATH_BATTERY = "/sys/class/power_supply/battery";
			public:
				inline uint8_t capacity() {
					std::string capacity_str = IO::read_file(PATH_BATTERY + "/capacity");
					return (uint8_t) stoi(capacity_str);
				}

				inline bool charging() {
					std::string status_str = IO::read_file(PATH_BATTERY + "/status");
					return (status_str.find("Charging") != std::string::npos);
				}
		};

		struct Memory {
			private:
				const std::string PATH_MEMINFO = "/proc/meminfo";
			public:
				inline long get_ram_size() {
					std::string str = IO::read_file(PATH_MEMINFO);
					std::istringstream iss(str);
					std::vector<std::string> str_split((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
					return stol(str_split[1]);
				}
		};
};
#endif

