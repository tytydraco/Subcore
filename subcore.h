#ifndef SUBCORE_H
#define SUBCORE_H

#include "sysfs.h"

class Subcore {
	private:
		enum State {
			state_level_0,
			state_level_1,
			state_level_2,
			state_level_3
		};

		struct sysfs_struct {
			State state;
			std::string iosched;
			std::string cpu_gov;
			std::vector<uint32_t> cpu_max_freqs;
			uint16_t gpu_max_freq;
			std::string lmk_minfree;
			uint16_t readahead;
			uint8_t swappiness;
			uint8_t cache_pressure;
			uint8_t dirty_ratio;
			uint8_t dirty_background_ratio;
			uint16_t entropy_read;
			uint16_t entropy_write;
			uint16_t subcore_scan_ms;
		};

		State current_state;

		SysFs::Cpu cpu;
		SysFs::Block block;
		SysFs::Gpu gpu;
		SysFs::Battery battery;
		SysFs::Display display;

		void setup_level_0();
		void setup_level_1();
		void setup_level_2();
		void setup_level_3();
	public:
		void algorithm();
		sysfs_struct level_0;
		sysfs_struct level_1;
		sysfs_struct level_2;
		sysfs_struct level_3;

		void setup_presets();
		void set_sysfs(sysfs_struct sysfs);
};

#endif

