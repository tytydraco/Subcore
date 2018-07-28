#ifndef SUBCORE_H
#define SUBCORE_H

#include "sysfs.h"

class Subcore {
	private:
		// objects
		enum State {
			state_level_0,
			state_level_1,
			state_level_2,
			state_level_3,
			state_init
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
			uint8_t laptop_mode;
			uint8_t oom_kill_allocating_task;
			uint8_t overcommit_memory;
			uint8_t page_cluster;
		};

		struct level_struct {
			State state;
			std::vector<std::string> gov_pref;
			uint8_t load_requirement;
			sysfs_struct level_data;
		};

		class UserSettings {
			private:
				sysfs_struct backup_settings;
				SysFs::Cpu &cpu;
				SysFs::Block &block;
				SysFs::Gpu &gpu;
				bool low_mem;
			public:
				void save();
				void load();

				UserSettings(SysFs::Cpu &_cpu, SysFs::Block &_block, SysFs::Gpu &_gpu)
					: cpu(_cpu), block(_block), gpu(_gpu) {}
		};

		// declarations
		State current_state = state_init;

		SysFs::Cpu cpu;
		SysFs::Block block;
		SysFs::Gpu gpu;
		SysFs::Battery battery;
		SysFs::Display display;
		
		UserSettings user_settings = UserSettings(cpu, block, gpu);

		level_struct level_0;
		level_struct level_1;
		level_struct level_2;
		level_struct level_3;

		// the # of times the same level is selected
		uint16_t same_level_count = 0;

		// functions
		void setup_level_0();
		void setup_level_1();
		void setup_level_2();
		void setup_level_3();
		std::string preferred_gov(std::vector<std::string> pref_govs);
		void set_sysfs(level_struct level);
	public:
		bool debug = false;
		bool low_mem = false;

		void algorithm();	
		void setup_presets();	
};

#endif

