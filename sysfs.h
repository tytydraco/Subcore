#ifndef STATS_H
#define STATS_H

#include <string>
#include <vector>

class SysFs {
	public:
		struct Cpu {
			private:
				const std::string PATH_CPU = "/sys/devices/system/cpu";
				const std::string PATH_STAT = "/proc/stat";
				const uint16_t STAT_AVG_SLEEP_MS = 3000;
			public:
				const uint8_t LOAD_POWERSAVE_THRESH = 10;
				const uint8_t LOAD_IDLE_THRESH = 20;
				const uint8_t LOAD_LOW_LAT_THRESH = 40;
				const uint8_t LOAD_PERFORMANCE_THRESH = 100;

				std::vector<uint32_t> get_freqs(uint8_t core);
				void set_max_freq(uint8_t core, uint32_t freq);
				uint32_t get_max_freq(uint8_t core);
				void set_min_freq(uint8_t core, uint32_t freq);
				uint32_t get_min_freq(uint8_t core);
				uint8_t get_present();
				void set_gov(uint8_t core, std::string gov);
				std::string get_gov(uint8_t core);
				uint8_t get_loadavg();
		};

		struct Gpu {
			private:
				const std::string PATH_GPU = "/sys/kernel/gpu";
			public:
				std::vector<uint16_t> get_freqs();
				void set_max_freq(uint16_t freq);
				uint16_t get_max_freq();
				void set_min_freq(uint16_t freq);
				uint16_t get_min_freq();
				void set_gov(std::string gov);
				std::string get_gov();
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
				void set_iosched(std::string blkdev, std::string iosched);
				std::string get_iosched(std::string blkdev);
				void set_lmk(std::string minfree);
				std::string get_lmk();
				void set_read_ahead(std::string blkdev, uint16_t read_ahead);
				uint16_t get_read_ahead(std::string blkdev);
				void set_swappiness(uint8_t swappiness);
				uint8_t get_swappiness();
				void set_cache_pressure(uint8_t pressure);
				uint8_t get_cache_pressure();
				void set_dirty_ratio(uint8_t ratio);
				uint8_t get_dirty_ratio();
				void set_dirty_background_ratio(uint8_t ratio);
				uint8_t get_dirty_background_ratio();
		};

		struct Battery {
			private:
				const std::string PATH_BATTERY = "/sys/class/power_supply/battery";
			public:
				uint8_t capacity();
				bool charging();
		};
};
#endif

