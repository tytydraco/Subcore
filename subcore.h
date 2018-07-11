#ifndef SUBCORE_H
#define SUBCORE_H

#include "sysfs.h"

class Subcore {
	private:
		struct sysfs_struct {
			std::string iosched;
			std::string cpu_gov;
			std::vector<int> cpu_max_freqs;
			int gpu_max_freq;
			std::string lmk_minfree;
			int new_subcore_scan;
			int readahead;
			int swappiness;
		};

		SysFs::Cpu cpu;
		SysFs::Block block;
		SysFs::Gpu gpu;

		void setup_powersave();
		void setup_idle();
		void setup_low_lat();
		void setup_performance();
	public:
		sysfs_struct powersave;
		sysfs_struct idle;
		sysfs_struct low_lat;
		sysfs_struct performance;

		void setup_presets();
		void set_sysfs(sysfs_struct sysfs);
};

#endif

