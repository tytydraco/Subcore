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

		struct interactive_struct {
			uint8_t go_hispeed_load;
			std::string above_hispeed_delay;
			uint32_t timer_rate;
			uint32_t hispeed_freq;
			int8_t timer_slack;
			std::string target_loads;
			uint32_t min_sample_time;
			uint8_t ignore_hispeed_on_notif;
			uint8_t powersave_bias;
			uint8_t fast_ramp_down;
			uint8_t align_windows;
			uint8_t use_migration_notif;
			uint8_t enable_prediction;
			uint32_t screen_off_maxfreq;
			uint8_t use_sched_load;
			uint8_t max_freq_hysteresis;
			uint32_t boostpulse_duration;
		};

		struct sysfs_struct {
			std::vector<std::string> ioscheds;
			std::vector<std::string> cpu_govs;
			std::vector<interactive_struct> interactives;
			std::vector<uint32_t> cpu_max_freqs;
			uint16_t gpu_max_freq;
			std::string lmk_minfree;
			std::vector<uint16_t> readaheads;
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
			uint8_t ksm;
		};

		struct level_struct {
			State state;
			std::vector<std::string> gov_pref;
			uint8_t load_requirement;
			sysfs_struct level_data;
		};

		struct UserSettings {
			private:
				sysfs_struct backup_settings;
				std::string hotplug;
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
		SysFs::Memory memory;
		SysFs::Gpu gpu;
		SysFs::Battery battery;
		SysFs::Display display;	

		level_struct level_0;
		level_struct level_1;
		level_struct level_2;
		level_struct level_3;

		// the # of times the same level is selected
		uint16_t same_level_count = 0;

		// functions
		void set_sysfs(level_struct level);
		void set_interactive(uint8_t core, interactive_struct interactive);
		std::string preferred_gov(std::vector<std::string> pref_govs);
		uint32_t freq_from_percent(std::vector<uint32_t> avail_freqs, uint8_t percent);
		uint32_t freq_from_percent(std::vector<uint16_t> avail_freqs, uint8_t percent);
	public:
		bool debug = false;
		bool low_mem = false;
		bool power_aware = true;

		UserSettings user_settings = UserSettings(cpu, block, gpu);

		void algorithm();	
		void setup_levels();
};

#endif

