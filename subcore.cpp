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
		load = 0;
	} else if (capacity <= 25) {
		// always use idle or lower
		load = (load > cpu.LOAD_IDLE_THRESH) ? cpu.LOAD_IDLE_THRESH : load;
	}

	// load based algorithm
	if (load <= cpu.LOAD_POWERSAVE_THRESH) {
		set_sysfs(powersave);
		std::cout << "powersave" << std::endl;
	} else if (load <= cpu.LOAD_IDLE_THRESH) {
		set_sysfs(idle);
		std::cout << "idle" << std::endl;
	} else if (load <= cpu.LOAD_LOW_LAT_THRESH) {
		set_sysfs(low_lat);
		std::cout << "low_lat" << std::endl;
	} else if (load <= cpu.LOAD_PERFORMANCE_THRESH) {
		set_sysfs(performance);
		std::cout << "performance" << std::endl;
	} else {
		std::cout << "other" << std::endl;
	}
}

void Subcore::setup_powersave() {
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
	powersave.state = state_powersave;
	powersave.iosched = "noop";
	powersave.cpu_gov = new_cpu_pref_gov;
	powersave.cpu_max_freqs = new_cpu_max_freqs;
	powersave.gpu_max_freq = gpu_avail_freqs[2];
	powersave.lmk_minfree = block.LMK_AGGRESSIVE;
	powersave.swappiness = 0;
	powersave.readahead = 128;
	powersave.cache_pressure = 25;
	powersave.dirty_ratio = 20;
	powersave.dirty_background_ratio = 5;
}

void Subcore::setup_idle() {
	uint8_t present = cpu.get_present();
	std::vector<uint32_t> new_cpu_max_freqs;
	for (size_t i = 0; i <= present; i++) {
		std::vector<uint32_t> cpu_avail_freqs = cpu.get_freqs(i);
		if (cpu_avail_freqs.size() > 0) {
			new_cpu_max_freqs.push_back(cpu_avail_freqs[1]);
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
	idle.state = state_idle;
	idle.iosched = "noop";
	idle.cpu_gov = new_cpu_pref_gov;
	idle.cpu_max_freqs = new_cpu_max_freqs;
	idle.gpu_max_freq = gpu_avail_freqs[2];
	idle.lmk_minfree = block.LMK_AGGRESSIVE;
	idle.swappiness = 0;
	idle.readahead = 128;
	idle.cache_pressure = 50;
	idle.dirty_ratio = 40;
	idle.dirty_background_ratio = 5;
}

void Subcore::setup_low_lat() {
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
	low_lat.state = state_low_lat;
	low_lat.iosched = "deadline";
	low_lat.cpu_gov = new_cpu_pref_gov;
	low_lat.cpu_max_freqs = new_cpu_max_freqs;
	low_lat.gpu_max_freq = gpu_avail_freqs[gpu_avail_freqs.size() - 2];
	low_lat.lmk_minfree = block.LMK_VERY_LIGHT;
	low_lat.swappiness = 25;
	low_lat.readahead = 512;
	low_lat.cache_pressure = 75;
	low_lat.dirty_ratio = 60;
	low_lat.dirty_background_ratio = 5;
}

void Subcore::setup_performance() {
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
	performance.state = state_performance;
	performance.iosched = "deadline";
	performance.cpu_gov = new_cpu_pref_gov;
	performance.cpu_max_freqs = new_cpu_max_freqs;
	performance.gpu_max_freq = gpu_avail_freqs[gpu_avail_freqs.size() - 1];
	performance.lmk_minfree = block.LMK_VERY_LIGHT;
	performance.swappiness = 75;
	performance.readahead = 2048;
	performance.cache_pressure = 100;
	performance.dirty_ratio = 90;
	performance.dirty_background_ratio = 5;
}

void Subcore::setup_presets() {
	setup_powersave();
	setup_idle();
	setup_low_lat();
	setup_performance();
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

	// set the current state
	current_state = sysfs.state;
}



