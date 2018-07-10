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

		struct Block {
			private:
				const std::string PATH_BLOCK = "/sys/block";
				const std::string PATH_MOUNTS = "/proc/mounts";
				const std::string PATH_LMK = "/sys/module/lowmemorykiller/parameters";
			public:
				std::vector<std::string> get_blkdevs();
				void set_iosched(std::string blkdev, std::string iosched);
				std::string get_iosched(std::string blkdev);
				void set_lmk_adj(std::string adj);
				std::string get_lmk_adj();
				void set_read_ahead(std::string blkdev, int read_ahead);
				int get_read_ahead(std::string blkdev);
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

