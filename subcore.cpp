#include <iostream>
#include <string>
#include <vector>

#include "subcore.h"

void Subcore::UserSettings::save() {
	//cpu gov & max freq
	std::vector<uint32_t> freqs;
	std::vector<std::string> govs;
	uint8_t online = cpu.online();
	for (size_t i = 0; i < online; i++) {
		freqs.push_back(cpu.max_freq(i));
		govs.push_back(cpu.gov(i));
	}

	backup_settings.cpu_max_freqs = freqs;
	backup_settings.cpu_govs = govs;	

	// gpu max freq
	backup_settings.gpu_max_freq = gpu.max_freq();

	// iosched & readahead
	std::vector<std::string> ioscheds;
	std::vector<uint16_t> readaheads;
	std::vector<std::string> blkdevs = block.blkdevs();
	for (std::string blkdev : blkdevs) {
		ioscheds.push_back(block.iosched(blkdev));
		readaheads.push_back(block.read_ahead(blkdev));
	}

	backup_settings.ioscheds = ioscheds;
	backup_settings.readaheads = readaheads;
	
	// low-memory devices will behave strangely with these tweaks
	if (!low_mem) {
		// swappiness
		backup_settings.swappiness = block.swappiness();

		// cache pressure
		backup_settings.cache_pressure = block.cache_pressure();
	
		// dirty ratios
		backup_settings.dirty_ratio = block.dirty_ratio();
		backup_settings.dirty_background_ratio = block.dirty_background_ratio();

		// lmk minfree
		backup_settings.lmk_minfree = block.lmk();

		// other vm tweaks
		backup_settings.laptop_mode = block.laptop_mode();
		backup_settings.oom_kill_allocating_task = block.oom_kill_allocating_task();
		backup_settings.overcommit_memory = block.overcommit_memory();
		backup_settings.page_cluster = block.page_cluster();
	}

	// entropy
	backup_settings.entropy_read = block.entropy_read();
	backup_settings.entropy_write = block.entropy_write();
}

void Subcore::UserSettings::load() {
	//cpu gov & max freq
	uint8_t online = cpu.online();
	for (size_t i = 0; i < online; i++) {
		cpu.gov(i, backup_settings.cpu_govs[i]);
		cpu.max_freq(i, backup_settings.cpu_max_freqs[i]);
	}

	// gpu max freq
	gpu.max_freq(backup_settings.gpu_max_freq);

	// iosched & readahead
	std::vector<std::string> blkdevs = block.blkdevs();
	for (size_t i = 0; i < blkdevs.size(); i++) {
		block.iosched(blkdevs[i], backup_settings.ioscheds[i]);
		block.read_ahead(blkdevs[i], backup_settings.readaheads[i]);
	}
	
	// low-memory devices will behave strangely with these tweaks
	if (!low_mem) {
		// swappiness
		block.swappiness(backup_settings.swappiness);

		// cache pressure
		block.cache_pressure(backup_settings.cache_pressure);
	
		// dirty ratios
		block.dirty_ratio(backup_settings.dirty_ratio);
		block.dirty_background_ratio(backup_settings.dirty_background_ratio);

		// lmk minfree
		block.lmk(backup_settings.lmk_minfree);

		// other vm tweaks
		block.laptop_mode(backup_settings.laptop_mode);
		block.oom_kill_allocating_task(backup_settings.oom_kill_allocating_task);
		block.overcommit_memory(backup_settings.overcommit_memory);
		block.page_cluster(backup_settings.page_cluster);
	}

	// entropy
	block.entropy_read(backup_settings.entropy_read);
	block.entropy_write(backup_settings.entropy_write);

}

void Subcore::algorithm() {
	uint8_t load = cpu.loadavg();
	
	if (debug)
		std::cout << "[*] Load: " << std::to_string(load) << "\t";
	
	//special cases
	bool charging = battery.charging();
	bool display_off = display.suspended();

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
	level_0.load_requirement = 10;
	level_0.state = state_level_0;

	level_0.gov_pref = std::vector<std::string> {
		"powersave",
		"performance"
	};
		
	uint8_t online = cpu.online();
	std::vector<uint32_t> new_cpu_max_freqs;
	std::string pref_gov = preferred_gov(level_0.gov_pref);
	for (size_t i = 0; i < online; i++) {
		std::vector<uint32_t> cpu_avail_freqs = cpu.freqs(i);
		if (cpu_avail_freqs.size() > 0) 
			new_cpu_max_freqs.push_back(cpu_avail_freqs[0]);
		else
			new_cpu_max_freqs.push_back(0);
		level_0.level_data.cpu_govs.push_back(pref_gov);
	}	
	level_0.level_data.cpu_max_freqs = new_cpu_max_freqs;

	std::vector<uint16_t> gpu_avail_freqs = gpu.freqs();
	if (gpu_avail_freqs.size() < 4) 
		level_0.level_data.gpu_max_freq = 0;
	else 
		level_0.level_data.gpu_max_freq = gpu_avail_freqs[3];

	std::vector<std::string> blkdevs = block.blkdevs();
	for (size_t i = 0; i < blkdevs.size(); i++) {
		level_0.level_data.readaheads.push_back(1024);
		level_0.level_data.ioscheds.push_back("noop");
	}

	level_0.level_data.lmk_minfree = block.LMK_AGGRESSIVE;
	level_0.level_data.swappiness = 0;
	level_0.level_data.cache_pressure = 50;
	level_0.level_data.dirty_ratio = 90;
	level_0.level_data.dirty_background_ratio = 80;
	level_0.level_data.entropy_read = 1024;
	level_0.level_data.entropy_write = 2048;
	level_0.level_data.subcore_scan_ms = 1000;
	level_0.level_data.laptop_mode = 1;
	level_0.level_data.oom_kill_allocating_task = 0;
	level_0.level_data.overcommit_memory = 0;
	level_0.level_data.page_cluster = 0;
}

void Subcore::setup_level_1() {	
	level_1.load_requirement = 20;
	level_1.state = state_level_1;

	level_1.gov_pref = std::vector<std::string> {
		"energy-dcfc",
		"schedutil",
		"darkness",
		"nightmare",
		"conservative",
		"performance"
	};

	uint8_t online = cpu.online();
	std::vector<uint32_t> new_cpu_max_freqs;
	std::string pref_gov = preferred_gov(level_1.gov_pref);
	for (size_t i = 0; i < online; i++) {
		std::vector<uint32_t> cpu_avail_freqs = cpu.freqs(i);
		if (cpu_avail_freqs.size() > 4) 
			new_cpu_max_freqs.push_back(cpu_avail_freqs[3]);
		else
			new_cpu_max_freqs.push_back(0);
		level_1.level_data.cpu_govs.push_back(pref_gov);
	}
	level_1.level_data.cpu_max_freqs = new_cpu_max_freqs;

	std::vector<uint16_t> gpu_avail_freqs = gpu.freqs();
	if (gpu_avail_freqs.size() < 5) 
		level_1.level_data.gpu_max_freq = 0;
	else 
		level_1.level_data.gpu_max_freq = gpu_avail_freqs[4];

	std::vector<std::string> blkdevs = block.blkdevs();
	for (size_t i = 0; i < blkdevs.size(); i++) {
		level_1.level_data.readaheads.push_back(2048);
		level_1.level_data.ioscheds.push_back("dealine");
	}

	level_1.level_data.lmk_minfree = block.LMK_AGGRESSIVE;
	level_1.level_data.swappiness = 20;
	level_1.level_data.cache_pressure = 60;
	level_1.level_data.dirty_ratio = 90;
	level_1.level_data.dirty_background_ratio = 80;
	level_1.level_data.entropy_read = 1024;
	level_1.level_data.entropy_write = 2048;
	level_1.level_data.subcore_scan_ms = 1500;
	level_1.level_data.laptop_mode = 1;
	level_1.level_data.oom_kill_allocating_task = 0;
	level_1.level_data.overcommit_memory = 0;
	level_1.level_data.page_cluster = 0;
}

void Subcore::setup_level_2() {	
	level_2.load_requirement = 40;
	level_2.state = state_level_2;

	level_2.gov_pref = std::vector<std::string> {
		"relaxed",
		"chill",
		"interactive",
		"helix_schedutil",
		"schedutil",
		"interactive",
		"performance"
	};

	uint8_t online = cpu.online();
	std::vector<uint32_t> new_cpu_max_freqs;
	std::string pref_gov = preferred_gov(level_2.gov_pref);
	for (size_t i = 0; i < online; i++) {
		std::vector<uint32_t> cpu_avail_freqs = cpu.freqs(i);
		if (cpu_avail_freqs.size() > 2) 
			new_cpu_max_freqs.push_back(cpu_avail_freqs[cpu_avail_freqs.size() - 3]);
		else
			new_cpu_max_freqs.push_back(0);
		level_2.level_data.cpu_govs.push_back(pref_gov);
	}
	level_2.level_data.cpu_max_freqs = new_cpu_max_freqs;

	std::vector<uint16_t> gpu_avail_freqs = gpu.freqs();
	if (gpu_avail_freqs.size() == 0) 
		level_2.level_data.gpu_max_freq = 0;
	else 
		level_2.level_data.gpu_max_freq = gpu_avail_freqs[gpu_avail_freqs.size() - 2];

	std::vector<std::string> blkdevs = block.blkdevs();
	for (size_t i = 0; i < blkdevs.size(); i++) {
		level_2.level_data.readaheads.push_back(2048);
		level_2.level_data.ioscheds.push_back("dealine");
	}

	level_2.level_data.lmk_minfree = block.LMK_VERY_LIGHT;
	level_2.level_data.swappiness = 30;
	level_2.level_data.cache_pressure = 70;
	level_2.level_data.dirty_ratio = 90;
	level_2.level_data.dirty_background_ratio = 80;
	level_2.level_data.entropy_read = 512;
	level_2.level_data.entropy_write = 2048;
	level_2.level_data.subcore_scan_ms = 2000;
	level_2.level_data.laptop_mode = 1;
	level_2.level_data.oom_kill_allocating_task = 0;
	level_2.level_data.overcommit_memory = 1;
	level_2.level_data.page_cluster = 3;
}

void Subcore::setup_level_3() {
	level_3.load_requirement = 100;
	level_3.state = state_level_3;

	level_3.gov_pref = std::vector<std::string> {
		"conservative",
		"ondemand",
		"performance"
	};

	uint8_t online = cpu.online();
	std::vector<uint32_t> new_cpu_max_freqs;
	std::string pref_gov = preferred_gov(level_3.gov_pref);
	for (size_t i = 0; i < online; i++) {
		std::vector<uint32_t> cpu_avail_freqs = cpu.freqs(i);
		if (cpu_avail_freqs.size() > 0) 
			new_cpu_max_freqs.push_back(cpu_avail_freqs[cpu_avail_freqs.size() - 1]);
		else
			new_cpu_max_freqs.push_back(0);
		level_3.level_data.cpu_govs.push_back(pref_gov);
	}
	level_3.level_data.cpu_max_freqs = new_cpu_max_freqs;

	std::vector<uint16_t> gpu_avail_freqs = gpu.freqs();
	if (gpu_avail_freqs.size() == 0) 
		level_3.level_data.gpu_max_freq = 0;
	else 
		level_3.level_data.gpu_max_freq = gpu_avail_freqs[gpu_avail_freqs.size() - 1];

	std::vector<std::string> blkdevs = block.blkdevs();
	for (size_t i = 0; i < blkdevs.size(); i++) {
		level_3.level_data.readaheads.push_back(2048);
		level_3.level_data.ioscheds.push_back("dealine");
	}

	level_3.level_data.lmk_minfree = block.LMK_VERY_LIGHT;
	level_3.level_data.swappiness = 40;
	level_3.level_data.cache_pressure = 80;
	level_3.level_data.dirty_ratio = 90;
	level_3.level_data.dirty_background_ratio = 80;
	level_3.level_data.entropy_read = 1024;
	level_3.level_data.entropy_write = 2048;
	level_3.level_data.subcore_scan_ms = 2500;
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

		bool display_off = display.suspended();
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
	uint8_t online = cpu.online();
	for (size_t i = 0; i < online; i++) {
		cpu.gov(i, level.level_data.cpu_govs[i]);
		cpu.max_freq(i, level.level_data.cpu_max_freqs[i]);
	}

	// gpu max freq
	gpu.max_freq(level.level_data.gpu_max_freq);

	// iosched & readahead
	std::vector<std::string> blkdevs = block.blkdevs();
	for (size_t i = 0; i < blkdevs.size(); i++) {
		block.iosched(blkdevs[i], level.level_data.ioscheds[i]);
		block.read_ahead(blkdevs[i], level.level_data.readaheads[i]);
	}
	
	// low-memory devices will behave strangely with these tweaks
	if (!low_mem) {
		// swappiness
		block.swappiness(level.level_data.swappiness);

		// cache pressure
		block.cache_pressure(level.level_data.cache_pressure);
	
		// dirty ratios
		block.dirty_ratio(level.level_data.dirty_ratio);
		block.dirty_background_ratio(level.level_data.dirty_background_ratio);

		// lmk minfree
		block.lmk(level.level_data.lmk_minfree);

		// other vm tweaks
		block.laptop_mode(level.level_data.laptop_mode);
		block.oom_kill_allocating_task(level.level_data.oom_kill_allocating_task);
		block.overcommit_memory(level.level_data.overcommit_memory);
		block.page_cluster(level.level_data.page_cluster);
	}

	// entropy
	block.entropy_read(level.level_data.entropy_read);
	block.entropy_write(level.level_data.entropy_write);
	
	// subcore scan ms
	// special cases for extended periods
	cpu.STAT_AVG_SLEEP_MS = level.level_data.subcore_scan_ms;

	// set the current state
	current_state = level.state;

	// reset count
	same_level_count = 0;
}

std::string Subcore::preferred_gov(std::vector<std::string> pref_govs) {
	std::vector<std::string> cpu_avail_govs = cpu.govs();
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

