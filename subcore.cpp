#include <iostream>
#include <string>
#include <vector>

#include "subcore.h"

void Subcore::setup_powersave() {

	int present = cpu.get_present();
	std::vector<int> new_cpu_max_freqs;
	for (int i = 0; i <= present; i++) {
		std::vector<int> cpu_avail_freqs = cpu.get_freqs(i);
		new_cpu_max_freqs.push_back(cpu_avail_freqs[0]);
	}

	std::vector<int> gpu_avail_freqs = gpu.get_freqs();

	powersave.iosched = "noop";
	powersave.cpu_gov = "powersave";
	powersave.cpu_max_freqs = new_cpu_max_freqs;
	powersave.gpu_max_freq = gpu_avail_freqs[0];
	powersave.lmk_minfree = block.LMK_AGGRESSIVE;
	powersave.new_subcore_scan = 5000;
	powersave.swappiness = 0;
}

void Subcore::setup_idle() {

	int present = cpu.get_present();
	std::vector<int> new_cpu_max_freqs;
	for (int i = 0; i <= present; i++) {
		std::vector<int> cpu_avail_freqs = cpu.get_freqs(i);
		new_cpu_max_freqs.push_back(cpu_avail_freqs[1]);
	}

	std::vector<int> gpu_avail_freqs = gpu.get_freqs();

	powersave.iosched = "noop";
	powersave.cpu_gov = "conservative";
	powersave.cpu_max_freqs = new_cpu_max_freqs;
	powersave.gpu_max_freq = gpu_avail_freqs[2];
	powersave.lmk_minfree = block.LMK_AGGRESSIVE;
	powersave.new_subcore_scan = 2000;
	powersave.swappiness = 0;
}

void Subcore::setup_low_lat() {

	int present = cpu.get_present();
	std::vector<int> new_cpu_max_freqs;
	for (int i = 0; i <= present; i++) {
		std::vector<int> cpu_avail_freqs = cpu.get_freqs(i);
		new_cpu_max_freqs.push_back(cpu_avail_freqs[cpu_avail_freqs.size() - 3]);
	}

	std::vector<int> gpu_avail_freqs = gpu.get_freqs();

	powersave.iosched = "deadline";
	powersave.cpu_gov = "interactive";
	powersave.cpu_max_freqs = new_cpu_max_freqs;
	powersave.gpu_max_freq = gpu_avail_freqs[gpu_avail_freqs.size() - 2];
	powersave.lmk_minfree = block.LMK_VERY_LIGHT;
	powersave.new_subcore_scan = 1000;
	powersave.swappiness = 25;
}

void Subcore::setup_performance() {

	int present = cpu.get_present();
	std::vector<int> new_cpu_max_freqs;
	for (int i = 0; i <= present; i++) {
		std::vector<int> cpu_avail_freqs = cpu.get_freqs(i);
		new_cpu_max_freqs.push_back(cpu_avail_freqs[cpu_avail_freqs.size() - 1]);
	}

	std::vector<int> gpu_avail_freqs = gpu.get_freqs();

	powersave.iosched = "deadline";
	powersave.cpu_gov = "ondemand";
	powersave.cpu_max_freqs = new_cpu_max_freqs;
	powersave.gpu_max_freq = gpu_avail_freqs[gpu_avail_freqs.size() - 1];
	powersave.lmk_minfree = block.LMK_VERY_LIGHT;
	powersave.new_subcore_scan = 500;
	powersave.swappiness = 25;
}

void SubCore::setup_presets() {
	setup_powersave();
	setup_idle();
	setup_lowlat();
	setup_performance();
}

void Subcore::set_sysfs(sysfs_struct sysfs) {
	
}


