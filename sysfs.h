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
				uint16_t STAT_AVG_SLEEP_MS = 1000;

				std::vector<uint32_t> freqs(uint16_t core);
				std::vector<std::string> govs();
				uint8_t loadavg();
				void hotplug(std::string name, bool state);
				std::string hotplug();

				inline void max_freq(uint16_t core, uint32_t freq) {
					IO::write_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_max_freq", std::to_string(freq));
				}

				inline uint32_t max_freq(uint16_t core) {
					std::string str = IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_max_freq");
					if (str == "")
						return 0;
					return (uint32_t) atoi(str.c_str());
				}

				inline void min_freq(uint16_t core, uint32_t freq) {
					IO::write_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_min_freq", std::to_string(freq));
				}

				inline uint32_t min_freq(uint16_t core) {
					std::string str = IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_min_freq");
					if (str == "")
						return 0;
					return (uint32_t) atoi(str.c_str());
				}

				inline uint8_t online() {
					std::string str = IO::read_file(PATH_CPU + "/online");
					if (str == "")
						return 0;
					return ((uint8_t) str.at(2) - '0') + 1;
				}

				inline void gov(uint16_t core, std::string gov) {
					IO::write_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_governor", gov);
				}

				inline std::string gov(uint16_t core) {
					return IO::read_file(PATH_CPU + "/cpu" + std::to_string(core) + "/cpufreq/scaling_governor");
				}
		};

		struct Gpu {
			private:
				const std::string PATH_GPU = "/sys/kernel/gpu";
			public:
				std::vector<uint16_t> freqs();
				uint8_t load();	

				inline void max_freq(uint16_t freq) {
					IO::write_file(PATH_GPU + "/gpu_max_clock", std::to_string(freq));
				}

				inline uint16_t max_freq() {
					std::string str = IO::read_file(PATH_GPU + "/gpu_max_clock");
					if (str == "")
						return 0;
					return (uint16_t) stoi(str);
				}

				inline void min_freq(uint16_t freq) {
					IO::write_file(PATH_GPU + "/gpu_min_clock", std::to_string(freq));
				}

				inline uint16_t min_freq() {
					std::string str = IO::read_file(PATH_GPU + "/gpu_min_clock");
					if (str == "")
						return 0;
					return (uint16_t) stoi(str);
				}

				inline void gov(std::string gov) {
					IO::write_file(PATH_GPU + "/gpu_governor", gov);
				}

				inline std::string gov() {
					return IO::read_file(PATH_GPU + "/gpu_governor");
				}
		};

		struct Block {
			private:
				const std::string PATH_BLOCK = "/sys/block";
				const std::string PATH_MOUNTS = "/proc/mounts";
				const std::string PATH_LMK = "/sys/module/lowmemorykiller/parameters";
				const std::string PATH_VM = "/proc/sys/vm";
				const std::string PATH_ENTROPY = "/proc/sys/kernel/random";
				const std::string PATH_MM = "/sys/kernel/mm";
			public:
				const std::string LMK_VERY_LIGHT = "1,2,3,4,5,6";
				const std::string LMK_LIGHT = "2,3,4,5,6,7";
				const std::string LMK_MEDIUM = "3,4,5,6,7,9";
				const std::string LMK_AGGRESSIVE = "2,3,6,10,14,15";
				const std::string LMK_VERY_AGGRESSIVE = "3,4,5,11,15,16";

				std::vector<std::string> blkdevs();

				inline void iosched(std::string blkdev, std::string iosched) {
					IO::write_file(PATH_BLOCK + "/" + blkdev + "/queue/scheduler", iosched);
				}

				inline std::string iosched(std::string blkdev) {
					return IO::read_file(PATH_BLOCK + "/" + blkdev + "/queue/scheduler");
				}

				inline void lmk(std::string minfree) {
					IO::write_file(PATH_LMK + "/minfree", minfree);
				}

				inline std::string lmk() {
					return IO::read_file(PATH_LMK + "/minfree");
				}

				inline void read_ahead(std::string blkdev, uint16_t read_ahead) {
					IO::write_file(PATH_BLOCK + "/" + blkdev + "/queue/read_ahead_kb", std::to_string(read_ahead));
				}

				inline uint16_t read_ahead(std::string blkdev) {
					std::string str = IO::read_file(PATH_BLOCK + "/" + blkdev + "/queue/read_ahead_kb");
					if (str == "")
						return 0;
					return (uint16_t) stoi(str);
				}

				inline void swappiness(uint8_t swappiness) {
					IO::write_file(PATH_VM + "/swappiness", std::to_string(swappiness));
				}

				inline uint8_t swappiness() {
					std::string str = IO::read_file(PATH_VM + "/swappiness");
					if (str == "")
						return 0;
					return (uint8_t) stoi(str);
				}

				inline void cache_pressure(uint8_t pressure) {
					IO::write_file(PATH_VM + "/vfs_cache_pressure", std::to_string(pressure));
				}

				inline uint8_t cache_pressure() {
					std::string str = IO::read_file(PATH_VM + "/vfs_cache_pressure");
					if (str == "")
						return 0;
					return (uint8_t) stoi(str);
				}

				inline void dirty_ratio(uint8_t ratio) {
					IO::write_file(PATH_VM + "/dirty_ratio", std::to_string(ratio));
				}

				inline uint8_t dirty_ratio() {
					std::string str = IO::read_file(PATH_VM + "/dirty_ratio");
					if (str == "")
						return 0;
					return (uint8_t) stoi(str);
				}

				inline void dirty_background_ratio(uint8_t ratio) {
					IO::write_file(PATH_VM + "/dirty_background_ratio", std::to_string(ratio));
				}

				inline uint8_t dirty_background_ratio() {
					std::string str = IO::read_file(PATH_VM + "/dirty_background_ratio");
					if (str == "")
						return 0;
					return (uint8_t) stoi(str);
				}

				inline void entropy_read(uint16_t entropy) {
					IO::write_file(PATH_ENTROPY + "/read_wakeup_threshold", std::to_string(entropy));
				}

				inline uint16_t entropy_read() {
					std::string str = IO::read_file(PATH_ENTROPY + "/read_wakeup_threshold");
					if (str == "")
						return 0;
					return (uint16_t) stoi(str);
				}

				inline void entropy_write(uint16_t entropy) {
					IO::write_file(PATH_ENTROPY + "/write_wakeup_threshold", std::to_string(entropy));
				}

				inline uint16_t entropy_write() {
					std::string str = IO::read_file(PATH_ENTROPY + "/write_wakeup_threshold");
					if (str == "")
						return 0;
					return (uint16_t) stoi(str);
				}

				inline void laptop_mode(uint8_t state) {
					IO::write_file(PATH_VM + "/laptop_mode", std::to_string(state));
				}

				inline uint8_t laptop_mode() {
					std::string str = IO::read_file(PATH_VM + "/laptop_mode");
					if (str == "")
						return 0;
					return (uint8_t) stoi(str);	
				}

				inline void oom_kill_allocating_task(uint8_t state) {
					IO::write_file(PATH_VM + "/oom_kill_allocating_task", std::to_string(state));
				}

				inline uint8_t oom_kill_allocating_task() {
					std::string str = IO::read_file(PATH_VM + "/oom_kill_allocating_task");
					if (str == "")
						return 0;
					return (uint8_t) stoi(str);
				}

				inline void overcommit_memory(uint8_t state) {
					IO::write_file(PATH_VM + "/overcommit_memory", std::to_string(state));
				}

				inline uint8_t overcommit_memory() {
					std::string str = IO::read_file(PATH_VM + "/overcommit_memory");
					if (str == "")
						return 0;
					return (uint8_t) stoi(str);
				}

				inline void page_cluster(uint8_t size) {
					IO::write_file(PATH_VM + "/page-cluster", std::to_string(size));
				}

				inline uint8_t page_cluster() {
					std::string str = IO::read_file(PATH_VM + "/page-cluster");
					if (str == "")
						return 0;
					return (uint8_t) stoi(str);
				}

				// dirty * centisecs are set by laptop mode
				inline void dirty_expire_centisecs(uint16_t secs) {
					IO::write_file(PATH_VM + "/dirty_expire_centisecs", std::to_string(secs));
				}

				inline uint16_t dirty_expire_centisecs() {
					std::string str = IO::read_file(PATH_VM + "/dirty_expire_centisecs");
					if (str == "")
						return 0;
					return (uint16_t) stoi(str);
				}

				inline void dirty_writeback_centisecs(uint16_t size) {
					IO::write_file(PATH_VM + "/dirty_writeback_centisecs", std::to_string(size));
				}

				inline uint16_t dirty_writeback_centisecs() {
					std::string str = IO::read_file(PATH_VM + "/dirty_writeback_centisecs");
					if (str == "")
						return 0;
					return (uint16_t) stoi(str);
				}

				inline void ksm(uint8_t state) {
					IO::write_file(PATH_MM + "/ksm/run", std::to_string(state));
				}

				inline uint8_t ksm() {
					std::string str = IO::read_file(PATH_MM + "/ksm/run");
					if (str == "")
						return 0;
					return (uint8_t) stoi(str);
				}

		};

		struct Battery {
			private:
				const std::string PATH_BATTERY = "/sys/class/power_supply/battery";
			public:
				inline uint8_t capacity() {
					std::string str = IO::read_file(PATH_BATTERY + "/capacity");
					if (str == "")
						return 100;
					return (uint8_t) stoi(str);
				}

				inline bool charging() {
					std::string str = IO::read_file(PATH_BATTERY + "/status");
					return (str.find("Charging") != std::string::npos);
				}
		};

		struct Memory {
			private:
				const std::string PATH_MEMINFO = "/proc/meminfo";
			public:
				uint32_t ram_size();
				uint32_t avail_ram();	
		};

		struct Display {
			private:
				const std::string PATH_STATE_NOTIFIER = "/sys/module/state_notifier/parameters";				
				const std::string PATH_POWER_SUSPEND = "/sys/kernel/power_suspend";
				const std::string PATH_FB0 = "/sys/devices/virtual/graphics/fb0";
			public:
				bool suspended();
		};
};
#endif

