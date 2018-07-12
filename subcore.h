#ifndef SUBCORE_H
#define SUBCORE_H

#include "sysfs.h"

class Subcore {
	private:
		enum State {
			state_powersave,
			state_idle,
			state_low_lat,
			state_performance
		};

		struct sysfs_struct {
			State state;
			std::string iosched;
			std::string cpu_gov;
			std::vector<int> cpu_max_freqs;
			int gpu_max_freq;
			std::string lmk_minfree;
			int new_subcore_scan;
			int readahead;
			int swappiness;
			int cache_pressure;
			int dirty_ratio;
			int dirty_background_ratio;
		};

		State current_state;

		SysFs::Cpu cpu;
		SysFs::Block block;
		SysFs::Gpu gpu;
		SysFs::Battery battery;

		void setup_powersave();
		void setup_idle();
		void setup_low_lat();
		void setup_performance();
	public:
		void algorithm();
		sysfs_struct powersave;
		sysfs_struct idle;
		sysfs_struct low_lat;
		sysfs_struct performance;

		void setup_presets();
		void set_sysfs(sysfs_struct sysfs);
};

#endif

