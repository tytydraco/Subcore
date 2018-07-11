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
				const int STAT_AVG_SLEEP_MS = 1000;
				const int STAT_AVG_IDLE_THRESH = 10;
			public:
				std::vector<int> get_freqs(int core);
				void set_max_freq(int core, int freq);
				int get_max_freq(int core);
				void set_min_freq(int core, int freq);
				int get_min_freq(int core);
				int get_present();
				void set_gov(int core, std::string gov);
				std::string get_gov(int core);
				int get_loadavg();
				bool is_idle();
		};

		struct Gpu {
			private:
				const std::string PATH_GPU = "/sys/kernel/gpu";
			public:
				std::vector<int> get_freqs();
				void set_max_freq(int freq);
				int get_max_freq();
				void set_min_freq(int freq);
				int get_min_freq();
				void set_gov(std::string gov);
				std::string get_gov();
		};

		struct Block {
			private:
				const std::string PATH_BLOCK = "/sys/block";
				const std::string PATH_MOUNTS = "/proc/mounts";
				const std::string PATH_LMK = "/sys/module/lowmemorykiller/parameters";
				const std::string PATH_SWAPPINESS = "/proc/sys/vm/swappiness";
				struct LMK_presets {
					const std::string VERY_LIGHT = "9169,18338,27507,36676,45845,55014";
					const std::string AGGRESSIVE = "18338,27507,55014,91690,128366,137535";
				};
			public:
				std::vector<std::string> get_blkdevs();
				void set_iosched(std::string blkdev, std::string iosched);
				std::string get_iosched(std::string blkdev);
				void set_lmk(std::string minfree);
				std::string get_lmk();
				void set_read_ahead(std::string blkdev, int read_ahead);
				int get_read_ahead(std::string blkdev);
				void set_swappiness(int swappiness);
				int get_swappiness();
		};

		struct Battery {
			private:
				const std::string PATH_BATTERY = "/sys/class/power_supply/battery";
			public:
				int capacity();
				bool charging();
		};
};
#endif

