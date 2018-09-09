#pragma once

#include "sysfs.h"

class subcore {
	private:
		// objects
		enum state {
			state_init,
			state_level_sleep,
			state_level_idle,
			state_level_light,
			state_level_medium,
			state_level_aggressive
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

		struct schedutil_struct {
			uint16_t up_rate_limit_us;
			uint16_t down_rate_limit_us;
			uint16_t rate_limit_us;
		};

		struct sysfs_struct {
			std::vector<std::string> ioscheds;
			std::vector<std::string> cpu_govs;
			std::vector<interactive_struct> interactives;
			std::vector<schedutil_struct> schedutils;
			std::vector<uint32_t> cpu_max_freqs;
			std::vector<uint32_t> cpu_min_freqs;
			std::string cpu_boost;
			uint16_t gpu_max_freq;
			uint16_t gpu_min_freq;
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
			state state;
			std::vector<std::string> gov_pref;
			uint8_t load_requirement;
			sysfs_struct level_data;
		};

		// place to save user settings in mem
		sysfs_struct backup_settings;	

		// declarations
		state current_state = state_init;

		sysfs::cpu cpu;
		sysfs::block block;
		sysfs::memory memory;
		sysfs::gpu gpu;
		sysfs::battery battery;
		sysfs::display display;	

		level_struct level_sleep;
		level_struct level_idle;
		level_struct level_light;
		level_struct level_medium;
		level_struct level_aggressive;

		// the # of times the same level is selected
		uint16_t same_level_count = 0;
		std::string hotplug;

		// min freq offsets
		std::vector<uint8_t> cpu_freq_offsets;
		std::vector<uint8_t> gpu_freq_offsets;

		// functions
		void set_sysfs(level_struct level);
		void set_interactive(uint8_t core, interactive_struct interactive);
		void set_schedutil(uint8_t core, schedutil_struct schedutil);
		std::string preferred_gov(std::vector<std::string> pref_govs);
		uint32_t freq_from_percent(std::vector<uint32_t> avail_freqs, uint8_t percent, uint8_t offset = 0);
		uint32_t freq_from_percent(std::vector<uint16_t> avail_freqs, uint8_t percent, uint8_t offset = 0);
	public:
		bool debug = false;
		bool memory_aware = false;
		bool power_aware = true;
		bool sleep_aware = true;
		bool gpu_mode	= false;

		void settings_save();
		void settings_load();

		void algorithm();	
		void setup_levels();
};

