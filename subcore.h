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

		struct level_struct {
			State state;
			std::vector<std::string> gov_pref;
			uint8_t load_requirement;
			sysfs_struct level_data;
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

		std::string preferred_gov(std::vector<std::string> pref_govs);
	public:
		bool debug = false;

		void algorithm();

		level_struct level_0;
		level_struct level_1;
		level_struct level_2;
		level_struct level_3;

		void setup_presets();
		void set_sysfs(level_struct level);
};

#endif

