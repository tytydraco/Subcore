#include <iostream>
#include <string>
#include <vector>

#include "subcore.h"

void Subcore::UserSettings::save() {
	//cpu gov & max freq
	std::vector<uint32_t> freqs;
	uint8_t online = cpu.get_online();
	for (size_t i = 0; i < online; i++) {
		freqs.push_back(cpu.get_max_freq(i));
	}
	backup_settings.cpu_max_freqs = freqs;

	//TODO: add support for cluster-independant CPU governors
	backup_settings.cpu_gov = cpu.get_gov(0);

	// gpu max freq
	backup_settings.gpu_max_freq = gpu.get_max_freq();

	// iosched & readahead
	std::vector<std::string> blkdevs = block.get_blkdevs();
	for (std::string blkdev : blkdevs) {
		backup_settings.iosched = block.get_iosched(blkdev);
		backup_settings.readahead = block.get_read_ahead(blkdev);

		//TODO: add support for blkdev-independant ioscheds / readaheads
		break;
	}
	
	// low-memory devices will behave strangely with these tweaks
	if (!low_mem) {
		// swappiness
		backup_settings.swappiness = block.get_swappiness();

		// cache pressure
		backup_settings.cache_pressure = block.get_cache_pressure();
	
		// dirty ratios
		backup_settings.dirty_ratio = block.get_dirty_ratio();
		backup_settings.dirty_background_ratio = block.get_dirty_background_ratio();

		// lmk minfree
		backup_settings.lmk_minfree = block.get_lmk();

		// other vm tweaks
		backup_settings.laptop_mode = block.get_laptop_mode();
		backup_settings.oom_kill_allocating_task = block.get_oom_kill_allocating_task();
		backup_settings.overcommit_memory = block.get_overcommit_memory();
		backup_settings.page_cluster = block.get_page_cluster();
	}

	// entropy
	backup_settings.entropy_read = block.get_entropy_read();
	backup_settings.entropy_write = block.get_entropy_write();
}

void Subcore::UserSettings::load() {
	//cpu gov & max freq
	uint8_t online = cpu.get_online();
	for (size_t i = 0; i < online; i++) {
		cpu.set_gov(i, backup_settings.cpu_gov);
		cpu.set_max_freq(i, backup_settings.cpu_max_freqs[i]);
	}

	// gpu max freq
	gpu.set_max_freq(backup_settings.gpu_max_freq);

	// iosched & readahead
	std::vector<std::string> blkdevs = block.get_blkdevs();
	for (std::string blkdev : blkdevs) {
		block.set_iosched(blkdev, backup_settings.iosched);
		block.set_read_ahead(blkdev, backup_settings.readahead);
	}
	
	// low-memory devices will behave strangely with these tweaks
	if (!low_mem) {
		// swappiness
		block.set_swappiness(backup_settings.swappiness);

		// cache pressure
		block.set_cache_pressure(backup_settings.cache_pressure);
	
		// dirty ratios
		block.set_dirty_ratio(backup_settings.dirty_ratio);
		block.set_dirty_background_ratio(backup_settings.dirty_background_ratio);

		// lmk minfree
		block.set_lmk(backup_settings.lmk_minfree);

		// other vm tweaks
		block.set_laptop_mode(backup_settings.laptop_mode);
		block.set_oom_kill_allocating_task(backup_settings.oom_kill_allocating_task);
		block.set_overcommit_memory(backup_settings.overcommit_memory);
		block.set_page_cluster(backup_settings.page_cluster);
	}

	// entropy
	block.set_entropy_read(backup_settings.entropy_read);
	block.set_entropy_write(backup_settings.entropy_write);

}

void Subcore::algorithm() {
	uint8_t load = cpu.get_loadavg();
	
	if (debug)
		std::cout << "[*] Load: " << std::to_string(load) << "\t";
	
	//special cases
	bool charging = battery.charging();
	bool display_off = display.get_suspended();

	if (charging) {
		// always use highest load based algorithm
		load = 100;
	} else if (display_off) {
		// use minimum load to conserve power
		load = 0;
	}

	// load based algorithm
	if (load <= level_0.load_requirement) {
		set_sysfs(level_0);
		if (debug)
			std::cout << "level_0" << std::endl;
	} else if (load <= level_1.load_requirement) {
		set_sysfs(level_1);
		if (debug)
			std::cout << "level_1" << std::endl;
	} else if (load <= level_2.load_requirement) {
		set_sysfs(level_2);
		if (debug)
			std::cout << "level_2" << std::endl;
	} else if (load <= level_3.load_requirement) {
		set_sysfs(level_3);
		if (debug)
			std::cout << "level_3" << std::endl;
	} else {
		if (debug)
			std::cout << "other" << std::endl;
	}
}

void Subcore::setup_level_0() {
	uint8_t online = cpu.get_online();
	std::vector<uint32_t> new_cpu_max_freqs;
	for (size_t i = 0; i < online; i++) {
		std::vector<uint32_t> cpu_avail_freqs = cpu.get_freqs(i);
		if (cpu_avail_freqs.size() > 0) {
			new_cpu_max_freqs.push_back(cpu_avail_freqs[0]);
		} else {
			new_cpu_max_freqs.push_back(0);
		}
	}

	level_0.gov_pref = std::vector<std::string> {
		"powersave",
		"performance"
	};

	level_0.load_requirement = 20;
	level_0.state = state_level_0;
	level_0.level_data.iosched = "noop";
	level_0.level_data.cpu_gov = preferred_gov(level_0.gov_pref);
	level_0.level_data.cpu_max_freqs = new_cpu_max_freqs;

	std::vector<uint16_t> gpu_avail_freqs = gpu.get_freqs();
	if (gpu_avail_freqs.size() < 4) 
		level_0.level_data.gpu_max_freq = 0;
	else 
		level_0.level_data.gpu_max_freq = gpu_avail_freqs[3];

	level_0.level_data.lmk_minfree = block.LMK_AGGRESSIVE;
	level_0.level_data.swappiness = 10;
	level_0.level_data.readahead = 256;
	level_0.level_data.cache_pressure = 10;
	level_0.level_data.dirty_ratio = 90;
	level_0.level_data.dirty_background_ratio = 80;
	level_0.level_data.entropy_read = 64;
	level_0.level_data.entropy_write = 128;
	level_0.level_data.subcore_scan_ms = 3000;
	level_0.level_data.laptop_mode = 1;
	level_0.level_data.oom_kill_allocating_task = 0;
	level_0.level_data.overcommit_memory = 0;
	level_0.level_data.page_cluster = 0;
}

void Subcore::setup_level_1() {
	uint8_t online = cpu.get_online();
	std::vector<uint32_t> new_cpu_max_freqs;
	for (size_t i = 0; i < online; i++) {
		std::vector<uint32_t> cpu_avail_freqs = cpu.get_freqs(i);
		if (cpu_avail_freqs.size() > 0) {
			new_cpu_max_freqs.push_back(cpu_avail_freqs[3]);
		} else {
			new_cpu_max_freqs.push_back(0);
		}
	}

	level_1.gov_pref = std::vector<std::string> {
		"energy-dcfc",
		"schedutil",
		"darkness",
		"nightmare",
		"conservative",
		"performance"
	};

	level_1.load_requirement = 40;
	level_1.state = state_level_1;
	level_1.level_data.iosched = "deadline";
	level_1.level_data.cpu_gov = preferred_gov(level_1.gov_pref);
	level_1.level_data.cpu_max_freqs = new_cpu_max_freqs;

	std::vector<uint16_t> gpu_avail_freqs = gpu.get_freqs();
	if (gpu_avail_freqs.size() < 5) 
		level_1.level_data.gpu_max_freq = 0;
	else 
		level_1.level_data.gpu_max_freq = gpu_avail_freqs[4];

	level_1.level_data.lmk_minfree = block.LMK_AGGRESSIVE;
	level_1.level_data.swappiness = 20;
	level_1.level_data.readahead = 512;
	level_1.level_data.cache_pressure = 10;
	level_1.level_data.dirty_ratio = 90;
	level_1.level_data.dirty_background_ratio = 80;
	level_1.level_data.entropy_read = 128;
	level_1.level_data.entropy_write = 256;
	level_1.level_data.subcore_scan_ms = 2000;
	level_1.level_data.laptop_mode = 1;
	level_1.level_data.oom_kill_allocating_task = 0;
	level_1.level_data.overcommit_memory = 0;
	level_1.level_data.page_cluster = 0;
}

void Subcore::setup_level_2() {
	uint8_t online = cpu.get_online();
	std::vector<uint32_t> new_cpu_max_freqs;
	for (size_t i = 0; i < online; i++) {
		std::vector<uint32_t> cpu_avail_freqs = cpu.get_freqs(i);
		if (cpu_avail_freqs.size() > 0) {
			new_cpu_max_freqs.push_back(cpu_avail_freqs[cpu_avail_freqs.size() - 3]);
		} else {
			new_cpu_max_freqs.push_back(0);
		}
	}

	level_2.gov_pref = std::vector<std::string> {
		"relaxed",
		"chill",
		"interactive",
		"helix_schedutil",
		"schedutil",
		"interactive",
		"performance"
	};

	level_2.load_requirement = 60;
	level_2.state = state_level_2;
	level_2.level_data.iosched = "deadline";
	level_2.level_data.cpu_gov = preferred_gov(level_2.gov_pref);
	level_2.level_data.cpu_max_freqs = new_cpu_max_freqs;

	std::vector<uint16_t> gpu_avail_freqs = gpu.get_freqs();
	if (gpu_avail_freqs.size() == 0) 
		level_2.level_data.gpu_max_freq = 0;
	else 
		level_2.level_data.gpu_max_freq = gpu_avail_freqs[gpu_avail_freqs.size() - 2];

	level_2.level_data.lmk_minfree = block.LMK_VERY_LIGHT;
	level_2.level_data.swappiness = 30;
	level_2.level_data.readahead = 1024;
	level_2.level_data.cache_pressure = 10;
	level_2.level_data.dirty_ratio = 90;
	level_2.level_data.dirty_background_ratio = 80;
	level_2.level_data.entropy_read = 512;
	level_2.level_data.entropy_write = 2048;
	level_2.level_data.subcore_scan_ms = 1000;
	level_2.level_data.laptop_mode = 1;
	level_2.level_data.oom_kill_allocating_task = 0;
	level_2.level_data.overcommit_memory = 1;
	level_2.level_data.page_cluster = 3;
}

void Subcore::setup_level_3() {
	uint8_t online = cpu.get_online();
	std::vector<uint32_t> new_cpu_max_freqs;
	for (size_t i = 0; i < online; i++) {
		std::vector<uint32_t> cpu_avail_freqs = cpu.get_freqs(i);
		if (cpu_avail_freqs.size() > 0) {
			new_cpu_max_freqs.push_back(cpu_avail_freqs[cpu_avail_freqs.size() - 1]);
		} else {
			new_cpu_max_freqs.push_back(0);
		}
	}

	level_3.gov_pref = std::vector<std::string> {
		"conservative",
		"ondemand",
		"performance"
	};

	level_3.load_requirement = 100;
	level_3.state = state_level_3;
	level_3.level_data.iosched = "deadline";
	level_3.level_data.cpu_gov = preferred_gov(level_3.gov_pref);
	level_3.level_data.cpu_max_freqs = new_cpu_max_freqs;

	std::vector<uint16_t> gpu_avail_freqs = gpu.get_freqs();
	if (gpu_avail_freqs.size() == 0) 
		level_3.level_data.gpu_max_freq = 0;
	else 
		level_3.level_data.gpu_max_freq = gpu_avail_freqs[gpu_avail_freqs.size() - 1];

	level_3.level_data.lmk_minfree = block.LMK_VERY_LIGHT;
	level_3.level_data.swappiness = 40;
	level_3.level_data.readahead = 2048;
	level_3.level_data.cache_pressure = 10;
	level_3.level_data.dirty_ratio = 90;
	level_3.level_data.dirty_background_ratio = 80;
	level_3.level_data.entropy_read = 1024;
	level_3.level_data.entropy_write = 2048;
	level_3.level_data.subcore_scan_ms = 500;
	level_3.level_data.laptop_mode = 1;
	level_3.level_data.oom_kill_allocating_task = 0;
	level_3.level_data.overcommit_memory = 1;
	level_3.level_data.page_cluster = 3;
}

void Subcore::setup_presets() {	
	setup_level_0();	
	setup_level_1();
	setup_level_2();
	setup_level_3();	
}

void Subcore::set_sysfs(level_struct level) {
	// if we are already on this state, do nothing
	if (current_state == level.state) {

		bool display_off = display.get_suspended();
		if (display_off) {
			// do not count when sleeping to avoid
			// stutter on wakeup (after +5000ms)
			same_level_count = 0;

			// we can return now, as thats all we need to do
			return;
		}

		// if we're awake, add to the counter
		same_level_count++;

		// depending on how constant the load is,
		// add a minor delay to keep it from dipping
		// too soon (IE loading scene / video pause)
		cpu.STAT_AVG_SLEEP_MS = level.level_data.subcore_scan_ms;
		if (same_level_count >= 5) {
			cpu.STAT_AVG_SLEEP_MS += 3000;
			if (debug)
				std::cout << "+3000ms\t";
		} else if (same_level_count >= 3) {
			cpu.STAT_AVG_SLEEP_MS += 1500;
			if (debug)
				std::cout << "+1500ms\t";
			}	

		return;
	}

	//cpu gov & max freq
	uint8_t online = cpu.get_online();
	for (size_t i = 0; i < online; i++) {
		cpu.set_gov(i, level.level_data.cpu_gov);
		cpu.set_max_freq(i, level.level_data.cpu_max_freqs[i]);
	}

	// gpu max freq
	gpu.set_max_freq(level.level_data.gpu_max_freq);

	// iosched & readahead
	std::vector<std::string> blkdevs = block.get_blkdevs();
	for (std::string blkdev : blkdevs) {
		block.set_iosched(blkdev, level.level_data.iosched);
		block.set_read_ahead(blkdev, level.level_data.readahead);
	}
	
	// low-memory devices will behave strangely with these tweaks
	if (!low_mem) {
		// swappiness
		block.set_swappiness(level.level_data.swappiness);

		// cache pressure
		block.set_cache_pressure(level.level_data.cache_pressure);
	
		// dirty ratios
		block.set_dirty_ratio(level.level_data.dirty_ratio);
		block.set_dirty_background_ratio(level.level_data.dirty_background_ratio);

		// lmk minfree
		block.set_lmk(level.level_data.lmk_minfree);

		// other vm tweaks
		block.set_laptop_mode(level.level_data.laptop_mode);
		block.set_oom_kill_allocating_task(level.level_data.oom_kill_allocating_task);
		block.set_overcommit_memory(level.level_data.overcommit_memory);
		block.set_page_cluster(level.level_data.page_cluster);
	}

	// entropy
	block.set_entropy_read(level.level_data.entropy_read);
	block.set_entropy_write(level.level_data.entropy_write);
	
	// subcore scan ms
	// special cases for extended periods
	cpu.STAT_AVG_SLEEP_MS = level.level_data.subcore_scan_ms;

	// set the current state
	current_state = level.state;

	// reset count
	same_level_count = 0;
}

std::string Subcore::preferred_gov(std::vector<std::string> pref_govs) {
	std::vector<std::string> cpu_avail_govs = cpu.get_govs();
	std::string new_cpu_pref_gov;
	for (std::string pref_gov : pref_govs) {
		for (std::string cpu_avail_gov : cpu_avail_govs) {
			if (cpu_avail_gov == pref_gov) {
				return pref_gov;
			}	
		}
	}

	// double failsafe
	return "performance";
}

