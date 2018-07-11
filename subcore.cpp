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
}

void Subcore::set_sysfs(sysfs_struct sysfs) {

}


