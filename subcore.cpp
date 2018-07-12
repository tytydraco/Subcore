#include <iostream>
#include <string>
#include <vector>

#include "subcore.h"

void Subcore::setup_powersave() {
	int present = cpu.get_present();
	std::vector<int> new_cpu_max_freqs;
	for (int i = 0; i <= present; i++) {
		std::vector<int> cpu_avail_freqs = cpu.get_freqs(i);
		if (cpu_avail_freqs.size() > 0) {
			new_cpu_max_freqs.push_back(cpu_avail_freqs[0]);
		} else {
			new_cpu_max_freqs.push_back(0);
		}
	}

	std::vector<int> gpu_avail_freqs = gpu.get_freqs();

	powersave.state = state_powersave;
	powersave.iosched = "noop";
	powersave.cpu_gov = "powersave";
	powersave.cpu_max_freqs = new_cpu_max_freqs;
	powersave.gpu_max_freq = gpu_avail_freqs[2];
	powersave.lmk_minfree = block.LMK_AGGRESSIVE;
	powersave.swappiness = 0;
	powersave.readahead = 128;
}

void Subcore::setup_idle() {

	int present = cpu.get_present();
	std::vector<int> new_cpu_max_freqs;
	for (int i = 0; i <= present; i++) {
		std::vector<int> cpu_avail_freqs = cpu.get_freqs(i);
		if (cpu_avail_freqs.size() > 0) {
			new_cpu_max_freqs.push_back(cpu_avail_freqs[1]);
		} else {
			new_cpu_max_freqs.push_back(0);
		}
	}

	std::vector<int> gpu_avail_freqs = gpu.get_freqs();

	idle.state = state_idle;
	idle.iosched = "noop";
	idle.cpu_gov = "conservative";
	idle.cpu_max_freqs = new_cpu_max_freqs;
	idle.gpu_max_freq = gpu_avail_freqs[2];
	idle.lmk_minfree = block.LMK_AGGRESSIVE;
	idle.swappiness = 0;
	idle.readahead = 128;
}

void Subcore::setup_low_lat() {

	int present = cpu.get_present();
	std::vector<int> new_cpu_max_freqs;
	for (int i = 0; i <= present; i++) {
		std::vector<int> cpu_avail_freqs = cpu.get_freqs(i);
		if (cpu_avail_freqs.size() > 0) {
			new_cpu_max_freqs.push_back(cpu_avail_freqs[cpu_avail_freqs.size() - 3]);
		} else {
			new_cpu_max_freqs.push_back(0);
		}
	}

	std::vector<int> gpu_avail_freqs = gpu.get_freqs();

	low_lat.state = state_low_lat;
	low_lat.iosched = "deadline";
	low_lat.cpu_gov = "interactive";
	low_lat.cpu_max_freqs = new_cpu_max_freqs;
	low_lat.gpu_max_freq = gpu_avail_freqs[gpu_avail_freqs.size() - 3];
	low_lat.lmk_minfree = block.LMK_VERY_LIGHT;
	low_lat.swappiness = 25;
	low_lat.readahead = 512;
}

void Subcore::setup_performance() {

	int present = cpu.get_present();
	std::vector<int> new_cpu_max_freqs;
	for (int i = 0; i <= present; i++) {
		std::vector<int> cpu_avail_freqs = cpu.get_freqs(i);
		if (cpu_avail_freqs.size() > 0) {
			new_cpu_max_freqs.push_back(cpu_avail_freqs[cpu_avail_freqs.size() - 1]);
		} else {
			new_cpu_max_freqs.push_back(0);
		}
	}

	std::vector<int> gpu_avail_freqs = gpu.get_freqs();

	performance.state = state_performance;
	performance.iosched = "deadline";
	performance.cpu_gov = "ondemand";
	performance.cpu_max_freqs = new_cpu_max_freqs;
	performance.gpu_max_freq = gpu_avail_freqs[gpu_avail_freqs.size() - 1];
	performance.lmk_minfree = block.LMK_VERY_LIGHT;
	performance.swappiness = 25;
	performance.readahead = 2048;
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

	//cpu gov & max freq
	int present = cpu.get_present();
	for (int i = 0; i <= present; i++) {
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



