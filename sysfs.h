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
			public:
				uint16_t STAT_AVG_SLEEP_MS = 3000;

				std::vector<uint32_t> get_freqs(uint16_t core);
				std::vector<std::string> get_govs();
				uint8_t get_loadavg();

				inline void set_max_freq(uint16_t core, uint32_t freq) {
					IO::write_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_max_freq", std::to_string(freq));
				}

				inline uint32_t get_max_freq(uint16_t core) {
					std::string str = IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_max_freq");
					try {
						return (uint32_t) atoi(str.c_str());
					} catch (...) {
						return 0;
					}
				}

				inline void set_min_freq(uint16_t core, uint32_t freq) {
					IO::write_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_min_freq", std::to_string(freq));
				}

				inline uint32_t get_min_freq(uint16_t core) {
					std::string str = IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_min_freq");
					try {
						return (uint32_t) atoi(str.c_str());
					} catch (...) {
						return 0;
					}
				}

				inline uint8_t get_present() {
					std::string present_str = IO::read_file(PATH_CPU + "/present");
					try {
						return ((uint8_t) present_str.at(2) - '0') + 1;
					} catch (...) {
						return 0;
					}
				}

				inline void set_gov(uint16_t core, std::string gov) {
					IO::write_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_governor", gov);
				}

				inline std::string get_gov(uint16_t core) {
					try {
						return IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_governor");
					} catch (...) {
						return "";
					}
				}	
		};

		struct Gpu {
			private:
				const std::string PATH_GPU = "/sys/kernel/gpu";
			public:
				std::vector<uint16_t> get_freqs();
				uint8_t get_load();	

				inline void set_max_freq(uint16_t freq) {
					IO::write_file(PATH_GPU + "/gpu_max_clock", std::to_string(freq));
				}

				inline uint16_t get_max_freq() {
					std::string str = IO::read_file(PATH_GPU + "/gpu_max_clock");
					try {
						return (uint16_t) stoi(str);
					} catch (...) {
						return 0;
					}
				}

				inline void set_min_freq(uint16_t freq) {
					IO::write_file(PATH_GPU + "/gpu_min_clock", std::to_string(freq));
				}

				inline uint16_t get_min_freq() {
					std::string str = IO::read_file(PATH_GPU + "/gpu_min_clock");
					try {
						return (uint16_t) stoi(str);
					} catch (...) {
						return 0;
					}
				}

				inline void set_gov(std::string gov) {
					IO::write_file(PATH_GPU + "/gpu_governor", gov);
				}

				inline std::string get_gov() {
					std::string str = IO::read_file(PATH_GPU + "/gpu_governor");
					try {
						return str;
					} catch (...) {
						return "";
					}
				}
		};

		struct Block {
			private:
				const std::string PATH_BLOCK = "/sys/block";
				const std::string PATH_MOUNTS = "/proc/mounts";
				const std::string PATH_LMK = "/sys/module/lowmemorykiller/parameters";
				const std::string PATH_VM = "/proc/sys/vm";
				const std::string PATH_ENTROPY = "/proc/sys/kernel/random";
			public:
				const std::string LMK_VERY_LIGHT = "9169,18338,27507,36676,45845,55014";
				const std::string LMK_AGGRESSIVE = "18338,27507,55014,91690,128366,137535";

				std::vector<std::string> get_blkdevs();

				inline void set_iosched(std::string blkdev, std::string iosched) {
					IO::write_file(PATH_BLOCK + "/" + blkdev + "/queue/scheduler", iosched);
				}

				inline std::string get_iosched(std::string blkdev) {
					try {
						return IO::read_file(PATH_BLOCK + "/" + blkdev + "/queue/scheduler");
					} catch (...) {
						return "";
					}
				}

				inline void set_lmk(std::string minfree) {
					IO::write_file(PATH_LMK + "/minfree", minfree);
				}

				inline std::string get_lmk() {
					try {
						return IO::read_file(PATH_LMK + "/minfree");
					} catch (...) {
						return "";
					}
				}

				inline void set_read_ahead(std::string blkdev, uint16_t read_ahead) {
					IO::write_file(PATH_BLOCK + "/" + blkdev + "/queue/read_ahead_kb", std::to_string(read_ahead));
				}

				inline uint16_t get_read_ahead(std::string blkdev) {
					std::string str = IO::read_file(PATH_BLOCK + "/" + blkdev + "/queue/read_ahead_kb");
					try {
						return (uint16_t) stoi(str);
					} catch (...) {
						return 0;
					}
				}

				inline void set_swappiness(uint8_t swappiness) {
					IO::write_file(PATH_VM + "/swappiness", std::to_string(swappiness));
				}

				inline uint8_t get_swappiness() {
					std::string str = IO::read_file(PATH_VM + "/swappiness");
					try {
						return (uint8_t) stoi(str);
					} catch (...) {
						return 0;
					}
				}

				inline void set_cache_pressure(uint8_t pressure) {
					IO::write_file(PATH_VM + "/vfs_cache_pressure", std::to_string(pressure));
				}

				inline uint8_t get_cache_pressure() {
					std::string str = IO::read_file(PATH_VM + "/vfs_cache_pressure");
					try {
						return (uint8_t) stoi(str);
					} catch (...) {
						return 0;
					}
				}

				inline void set_dirty_ratio(uint8_t ratio) {
					IO::write_file(PATH_VM + "/dirty_ratio", std::to_string(ratio));
				}

				inline uint8_t get_dirty_ratio() {
					std::string str = IO::read_file(PATH_VM + "/dirty_ratio");
					try {
						return (uint8_t) stoi(str);
					} catch (...) {
						return 0;
					}
				}

				inline void set_dirty_background_ratio(uint8_t ratio) {
					IO::write_file(PATH_VM + "/dirty_background_ratio", std::to_string(ratio));
				}

				inline uint8_t get_dirty_background_ratio() {
					std::string str = IO::read_file(PATH_VM + "/dirty_background_ratio");
					try {
						return (uint8_t) stoi(str);
					} catch (...) {
						return 0;
					}
				}

				inline void set_entropy_read(uint16_t entropy) {
					IO::write_file(PATH_ENTROPY + "/read_wakeup_threshold", std::to_string(entropy));
				}

				inline uint16_t get_entropy_read() {
					std::string str = IO::read_file(PATH_ENTROPY + "/read_wakeup_threshold");
					try {
						return (uint16_t) stoi(str);
				 	} catch (...) {
						return 0;
					}
				}

				inline void set_entropy_write(uint16_t entropy) {
					IO::write_file(PATH_ENTROPY + "/write_wakeup_threshold", std::to_string(entropy));
				}

				inline uint16_t get_entropy_write() {
					std::string str = IO::read_file(PATH_ENTROPY + "/write_wakeup_threshold");
					try {
						return (uint16_t) stoi(str);
					} catch (...) {
						return 0;
					}
				}

				inline void set_laptop_mode(uint8_t state) {
					IO::write_file(PATH_VM + "/laptop_mode", std::to_string(state));
				}

				inline uint8_t get_latptop_mode() {
					std::string str = IO::read_file(PATH_VM + "/laptop_mode");
					try {
						return (uint8_t) stoi(str);
					} catch (...) {
						return 0;
					}	
				}

				inline void set_oom_kill_allocating_task(uint8_t state) {
					IO::write_file(PATH_VM + "/oom_kill_allocating_task", std::to_string(state));
				}

				inline uint8_t get_oom_kill_allocating_task() {
					std::string str = IO::read_file(PATH_VM + "/oom_kill_allocating_task");
					try {
						return (uint8_t) stoi(str);
					} catch (...) {
						return 0;
					}
				}

				inline void set_overcommit_memory(uint8_t state) {
					IO::write_file(PATH_VM + "/overcommit_memory", std::to_string(state));
				}

				inline uint8_t get_overcommit_memory() {
					std::string str = IO::read_file(PATH_VM + "/overcommit_memory");
					try {
						return (uint8_t) stoi(str);
					} catch (...) {
						return 0;
					}
				}

				inline void set_page_cluster(uint8_t size) {
					IO::write_file(PATH_VM + "/page-cluster", std::to_string(size));
				}

				inline uint8_t get_page_cluster() {
					std::string str = IO::read_file(PATH_VM + "/page-cluster");
					try {
						return (uint8_t) stoi(str);
					} catch (...) {
						return 0;
					}
				}

				// dirty * centisecs are set by laptop mode
				inline void set_dirty_expire_centisecs(uint16_t secs) {
					IO::write_file(PATH_VM + "/dirty_expire_centisecs", std::to_string(secs));
				}

				inline uint16_t get_dirty_expire_centisecs() {
					std::string str = IO::read_file(PATH_VM + "/dirty_expire_centisecs");
					try {
						return (uint16_t) stoi(str);
					} catch (...) {
						return 0;
					}
				}

				inline void set_dirty_writeback_centisecs(uint16_t size) {
					IO::write_file(PATH_VM + "/dirty_writeback_centisecs", std::to_string(size));
				}

				inline uint16_t get_dirty_writeback_centisecs() {
					std::string str = IO::read_file(PATH_VM + "/dirty_writeback_centisecs");
					try {
						return (uint16_t) stoi(str);
					} catch (...) {
						return 0;
					}
				}

		};

		struct Battery {
			private:
				const std::string PATH_BATTERY = "/sys/class/power_supply/battery";
			public:
				inline uint8_t capacity() {
					std::string capacity_str = IO::read_file(PATH_BATTERY + "/capacity");
					// return 100 if there is no valid capacity

					try {
						return (uint8_t) stoi(capacity_str);
					} catch (...) {
						return 100;
					}
				}

				inline bool charging() {
					std::string status_str = IO::read_file(PATH_BATTERY + "/status");
					try {
						return (status_str.find("Charging") != std::string::npos);
					} catch (...) {
						return false;
					}
				}
		};

		struct Memory {
			private:
				const std::string PATH_MEMINFO = "/proc/meminfo";
			public:
				uint32_t get_ram_size();
				uint32_t get_avail_ram();	
		};

		struct Display {
			private:
				const std::string PATH_STATE_NOTIFIER = "/sys/module/state_notifier/parameters";				
				const std::string PATH_POWER_SUSPEND = "/sys/kernel/power_suspend";
				const std::string PATH_FB0 = "/sys/devices/virtual/graphics/fb0";
			public:
				bool get_suspended();
		};
};
#endif

