#include <iostream>
#include <string>
#include <vector>

#include "subcore.h"

void Subcore::algorithm() {
	uint8_t load = cpu.get_loadavg();
	std::cout << "[*] Load: " << std::to_string(load) << "\t";
	
	//special cases
	uint8_t capacity = battery.capacity();
	bool charging = battery.charging();

	if (charging) {
		// alwats use highest load based algorithm
		load = 100;
	} else if (capacity <= 15) {
		// always use lowest load based algorithm
		load /= 2;
	}

	// load based algorithm
	if (load <= cpu.LOAD_POWERSAVE_THRESH) {
		set_sysfs(level_0);
		std::cout << "level_0" << std::endl;
	} else if (load <= cpu.LOAD_IDLE_THRESH) {
		set_sysfs(level_1);
		std::cout << "level_1" << std::endl;
	} else if (load <= cpu.LOAD_LOW_LAT_THRESH) {
		set_sysfs(level_2);
		std::cout << "level_2" << std::endl;
	} else if (load <= cpu.LOAD_PERFORMANCE_THRESH) {
		set_sysfs(level_3);
		std::cout << "level_3" << std::endl;
	} else {
		std::cout << "other" << std::endl;
	}
}

void Subcore::setup_level_0() {
	uint8_t present = cpu.get_present();
	std::vector<uint32_t> new_cpu_max_freqs;
	for (size_t i = 0; i <= present; i++) {
		std::vector<uint32_t> cpu_avail_freqs = cpu.get_freqs(i);
		if (cpu_avail_freqs.size() > 0) {
			new_cpu_max_freqs.push_back(cpu_avail_freqs[0]);
		} else {
			new_cpu_max_freqs.push_back(0);
		}
	}

	std::vector<uint16_t> gpu_avail_freqs = gpu.get_freqs();

	std::vector<std::string> cpu_avail_govs = cpu.get_govs();
	std::string new_cpu_pref_gov;
	for (std::string cpu_pref_gov : cpu.GOV_PREF_POWERSAVE) {
		for (std::string cpu_avail_gov : cpu_avail_govs) {
			if (cpu_avail_gov == cpu_pref_gov) {	
				new_cpu_pref_gov = cpu_pref_gov;
				goto exit_gov_loop;
			}	
		}
	}

exit_gov_loop:	
	level_0.state = state_level_0;
	level_0.iosched = "noop";
	level_0.cpu_gov = new_cpu_pref_gov;
	level_0.cpu_max_freqs = new_cpu_max_freqs;
	if (gpu_avail_freqs.size() == 0) 
		level_0.gpu_max_freq = 0;
	else 
		level_0.gpu_max_freq = gpu_avail_freqs[1];
	level_0.lmk_minfree = block.LMK_AGGRESSIVE;
	level_0.swappiness = 0;
	level_0.readahead = 128;
	level_0.cache_pressure = 10;
	level_0.dirty_ratio = 20;
	level_0.dirty_background_ratio = 5;
	level_0.subcore_scan_ms = 5000;
}

void Subcore::setup_level_1() {
	uint8_t present = cpu.get_present();
	std::vector<uint32_t> new_cpu_max_freqs;
	for (size_t i = 0; i <= present; i++) {
		std::vector<uint32_t> cpu_avail_freqs = cpu.get_freqs(i);
		if (cpu_avail_freqs.size() > 0) {
			new_cpu_max_freqs.push_back(cpu_avail_freqs[3]);
		} else {
			new_cpu_max_freqs.push_back(0);
		}
	}

	std::vector<uint16_t> gpu_avail_freqs = gpu.get_freqs();

	std::vector<std::string> cpu_avail_govs = cpu.get_govs();
	std::string new_cpu_pref_gov;
	for (std::string cpu_pref_gov : cpu.GOV_PREF_IDLE) {
		for (std::string cpu_avail_gov : cpu_avail_govs) {
			if (cpu_avail_gov == cpu_pref_gov) {
				new_cpu_pref_gov = cpu_pref_gov;
				goto exit_gov_loop;
			}	
		}
	}
	
exit_gov_loop:
	level_1.state = state_level_1;
	level_1.iosched = "noop";
	level_1.cpu_gov = new_cpu_pref_gov;
	level_1.cpu_max_freqs = new_cpu_max_freqs;
	if (gpu_avail_freqs.size() == 0) 
		level_1.gpu_max_freq = 0;
	else 
		level_1.gpu_max_freq = gpu_avail_freqs[2];
	level_1.lmk_minfree = block.LMK_AGGRESSIVE;
	level_1.swappiness = 0;
	level_1.readahead = 256;
	level_1.cache_pressure = 15;
	level_1.dirty_ratio = 30;
	level_1.dirty_background_ratio = 5;
	level_1.subcore_scan_ms = 4000;
}

void Subcore::setup_level_2() {
	uint8_t present = cpu.get_present();
	std::vector<uint32_t> new_cpu_max_freqs;
	for (size_t i = 0; i <= present; i++) {
		std::vector<uint32_t> cpu_avail_freqs = cpu.get_freqs(i);
		if (cpu_avail_freqs.size() > 0) {
			new_cpu_max_freqs.push_back(cpu_avail_freqs[cpu_avail_freqs.size() - 3]);
		} else {
			new_cpu_max_freqs.push_back(0);
		}
	}

	std::vector<uint16_t> gpu_avail_freqs = gpu.get_freqs();

	std::vector<std::string> cpu_avail_govs = cpu.get_govs();
	std::string new_cpu_pref_gov;
	for (std::string cpu_pref_gov : cpu.GOV_PREF_LOW_LAT) {
		for (std::string cpu_avail_gov : cpu_avail_govs) {
			if (cpu_avail_gov == cpu_pref_gov) {
				new_cpu_pref_gov = cpu_pref_gov;
				goto exit_gov_loop;
			}	
		}
	}

exit_gov_loop:
	level_2.state = state_level_2;
	level_2.iosched = "deadline";
	level_2.cpu_gov = new_cpu_pref_gov;
	level_2.cpu_max_freqs = new_cpu_max_freqs;
	if (gpu_avail_freqs.size() == 0) 
		level_2.gpu_max_freq = 0;
	else 
		level_2.gpu_max_freq = gpu_avail_freqs[gpu_avail_freqs.size() - 2];
	level_2.lmk_minfree = block.LMK_VERY_LIGHT;
	level_2.swappiness = 25;
	level_2.readahead = 512;
	level_2.cache_pressure = 40;
	level_2.dirty_ratio = 40;
	level_2.dirty_background_ratio = 5;
	level_2.subcore_scan_ms = 3000;
}

void Subcore::setup_level_3() {
	uint8_t present = cpu.get_present();
	std::vector<uint32_t> new_cpu_max_freqs;
	for (size_t i = 0; i <= present; i++) {
		std::vector<uint32_t> cpu_avail_freqs = cpu.get_freqs(i);
		if (cpu_avail_freqs.size() > 0) {
			new_cpu_max_freqs.push_back(cpu_avail_freqs[cpu_avail_freqs.size() - 1]);
		} else {
			new_cpu_max_freqs.push_back(0);
		}
	}

	std::vector<uint16_t> gpu_avail_freqs = gpu.get_freqs();

	std::vector<std::string> cpu_avail_govs = cpu.get_govs();
	std::string new_cpu_pref_gov;
	for (std::string cpu_pref_gov : cpu.GOV_PREF_PERFORMANCE) {
		for (std::string cpu_avail_gov : cpu_avail_govs) {
			if (cpu_avail_gov == cpu_pref_gov) {
				new_cpu_pref_gov = cpu_pref_gov;
				goto exit_gov_loop;
			}	
		}
	}
	
exit_gov_loop:
	level_3.state = state_level_3;
	level_3.iosched = "deadline";
	level_3.cpu_gov = new_cpu_pref_gov;
	level_3.cpu_max_freqs = new_cpu_max_freqs;
	if (gpu_avail_freqs.size() == 0) 
		level_3.gpu_max_freq = 0;
	else 
		level_3.gpu_max_freq = gpu_avail_freqs[gpu_avail_freqs.size() - 1];
	level_3.lmk_minfree = block.LMK_VERY_LIGHT;
	level_3.swappiness = 50;
	level_3.readahead = 1024;
	level_3.cache_pressure = 50;
	level_3.dirty_ratio = 75;
	level_3.dirty_background_ratio = 5;
	level_3.subcore_scan_ms = 2000;
}

void Subcore::setup_presets() {	
	setup_level_0();	
	setup_level_1();
	setup_level_2();
	setup_level_3();	
}

void Subcore::set_sysfs(sysfs_struct sysfs) {
	// if we are already on this state, do nothing
	if (current_state == sysfs.state)
		return;

	// iosched & readahead
	std::vector<std::string> blkdevs = block.get_blkdevs();
	for (std::string blkdev : blkdevs) {
		block.set_iosched(blkdev, sysfs.iosched);
		block.set_read_ahead(blkdev, sysfs.readahead);
	}

	// swappiness
	block.set_swappiness(sysfs.swappiness);

	// cache pressure
	block.set_cache_pressure(sysfs.cache_pressure);
	
	// dirty ratios
	block.set_dirty_ratio(sysfs.dirty_ratio);
	block.set_dirty_background_ratio(sysfs.dirty_background_ratio);

	//cpu gov & max freq
	uint8_t present = cpu.get_present();
	for (size_t i = 0; i <= present; i++) {
		cpu.set_gov(i, sysfs.cpu_gov);
		cpu.set_max_freq(i, sysfs.cpu_max_freqs[i]);
	}
	
	// gpu max freq
	gpu.set_max_freq(sysfs.gpu_max_freq);
	
	// lmk minfree
	block.set_lmk(sysfs.lmk_minfree);
	
	// subcore scan ms
	cpu.STAT_AVG_SLEEP_MS = sysfs.subcore_scan_ms;

	// set the current state
	current_state = sysfs.state;
}



