#include <iostream>
#include <string>
#include <vector>

#include "subcore.h"

void Subcore::setup_powersave() {

	int present = cpu.get_present();
	std::vector<int> new_cpu_max_freqs;
	for (int i = 0; i <= present; i++) {
		std::vector<int> avail_freqs = cpu.get_freqs(i);
		new_cpu_max_freqs.push_back(avail_freqs[0]);
	}

	powersave.iosched = "noop";
	powersave.cpu_gov = "powersave";
	powersave.cpu_max_freqs = new_cpu_max_freqs;

}

void Subcore::set_sysfs(sysfs_struct sysfs) {

}


