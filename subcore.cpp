#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <sys/types.h>
#include <signal.h>
#include <algorithm>

#include "subcore.h"

void subcore::user_settings::save() {
	hotplug = cpu.hotplug();
	uint8_t online = cpu.online();
	for (size_t i = 0; i < online; i++) {
		backup_settings.cpu_max_freqs.push_back(cpu.max_freq(i));
		backup_settings.cpu_min_freqs.push_back(cpu.min_freq(i));
		backup_settings.cpu_govs.push_back(cpu.gov(i));
	}
	backup_settings.gpu_max_freq = gpu.max_freq();
	backup_settings.gpu_min_freq = gpu.min_freq();

	std::vector<std::string> blkdevs = block.blkdevs();
	for (std::string blkdev : blkdevs) {
		backup_settings.ioscheds.push_back(block.iosched(blkdev));
		backup_settings.readaheads.push_back(block.read_ahead(blkdev));
	}
	
	if (!low_mem) {
		backup_settings.swappiness = block.swappiness();
		backup_settings.cache_pressure = block.cache_pressure();
		backup_settings.dirty_ratio = block.dirty_ratio();
		backup_settings.dirty_background_ratio = block.dirty_background_ratio();
		backup_settings.lmk_minfree = block.lmk();
		backup_settings.laptop_mode = block.laptop_mode();
		backup_settings.oom_kill_allocating_task = block.oom_kill_allocating_task();
		backup_settings.overcommit_memory = block.overcommit_memory();
		backup_settings.page_cluster = block.page_cluster();
		backup_settings.ksm = block.ksm();
	}
	backup_settings.entropy_read = block.entropy_read();
	backup_settings.entropy_write = block.entropy_write();

	for (size_t i = 0; i < online; i++) {
		interactive_struct interactive;
		std::string PATH_INTERACTIVE;
		if (io::path_exists("/sys/devices/system/cpu/cpufreq/interactive"))
			PATH_INTERACTIVE = "/sys/devices/system/cpu/cpufreq/interactive";
		else if (io::path_exists("/sys/devices/system/cpu/cpufreq/policy" + std::to_string(i) + "/interactive"))
			PATH_INTERACTIVE = "/sys/devices/system/cpu/cpufreq/policy" + std::to_string(i) + "/interactive";
		else {
			backup_settings.interactives.push_back(interactive);
			continue;
		}
		interactive.go_hispeed_load = (uint8_t) stoi(io::read_file(PATH_INTERACTIVE + "/go_hispeed_load"));
		interactive.above_hispeed_delay = io::read_file(PATH_INTERACTIVE + "/above_hispeed_delay");
		interactive.timer_rate = (uint32_t) stoi(io::read_file(PATH_INTERACTIVE + "/timer_rate"));
		interactive.timer_slack = (int32_t) stoi(io::read_file(PATH_INTERACTIVE + "/timer_slack"));
		interactive.min_sample_time = (uint32_t) stoi(io::read_file(PATH_INTERACTIVE + "/min_sample_time"));
		interactive.hispeed_freq = (uint32_t) stoi(io::read_file(PATH_INTERACTIVE + "/hispeed_freq"));
		interactive.target_loads = io::read_file(PATH_INTERACTIVE + "/target_loads");
		backup_settings.interactives.push_back(interactive);
	}
}

void subcore::user_settings::load() {
	cpu.hotplug(hotplug, true);
	uint8_t online = cpu.online();
	for (size_t i = 0; i < online; i++) {
		cpu.max_freq(i, backup_settings.cpu_max_freqs[i]);
		cpu.min_freq(i, backup_settings.cpu_min_freqs[i]);
	}
	gpu.max_freq(backup_settings.gpu_max_freq);
	gpu.min_freq(backup_settings.gpu_min_freq);

	std::vector<std::string> blkdevs = block.blkdevs();
	for (size_t i = 0; i < blkdevs.size(); i++) {
		block.iosched(blkdevs[i], backup_settings.ioscheds[i]);
		block.read_ahead(blkdevs[i], backup_settings.readaheads[i]);
	}

	if (!low_mem) {
		block.swappiness(backup_settings.swappiness);
		block.cache_pressure(backup_settings.cache_pressure);
		block.dirty_ratio(backup_settings.dirty_ratio);
		block.dirty_background_ratio(backup_settings.dirty_background_ratio);
		block.lmk(backup_settings.lmk_minfree);
		block.laptop_mode(backup_settings.laptop_mode);
		block.oom_kill_allocating_task(backup_settings.oom_kill_allocating_task);
		block.overcommit_memory(backup_settings.overcommit_memory);
		block.page_cluster(backup_settings.page_cluster);
		block.ksm(backup_settings.ksm);
	}
	block.entropy_read(backup_settings.entropy_read);
	block.entropy_write(backup_settings.entropy_write);

	for (uint8_t i = 0; i < online; i++) {
		std::string PATH_INTERACTIVE;
		if (io::path_exists("/sys/devices/system/cpu/cpufreq/interactive"))
			PATH_INTERACTIVE = "/sys/devices/system/cpu/cpufreq/interactive";
		else if (io::path_exists("/sys/devices/system/cpu/cpufreq/policy" + std::to_string(i) + "/interactive"))
			PATH_INTERACTIVE = "/sys/devices/system/cpu/cpufreq/policy" + std::to_string(i) + "/interactive";
		else
			continue;
		io::write_file(PATH_INTERACTIVE + "/go_hispeed_load", std::to_string(backup_settings.interactives[i].go_hispeed_load));
		io::write_file(PATH_INTERACTIVE + "/above_hispeed_delay", backup_settings.interactives[i].above_hispeed_delay);
		io::write_file(PATH_INTERACTIVE + "/timer_rate", std::to_string(backup_settings.interactives[i].timer_rate));
		io::write_file(PATH_INTERACTIVE + "/timer_slack", std::to_string(backup_settings.interactives[i].timer_slack));
		io::write_file(PATH_INTERACTIVE + "/min_sample_time", std::to_string(backup_settings.interactives[i].min_sample_time));
		io::write_file(PATH_INTERACTIVE + "/hispeed_freq", std::to_string(backup_settings.interactives[i].hispeed_freq));
		io::write_file(PATH_INTERACTIVE + "/target_loads", backup_settings.interactives[i].target_loads);
		cpu.gov(i, backup_settings.cpu_govs[i]);
	}
}

void subcore::algorithm() {
	uint8_t load = cpu.loadavg();
	if (cpu.online() <= 4)
		load *= 1.5;
	
	//special cases
	if (display.suspended()) {
		load = 0;
	} else if (power_aware) {
		uint8_t capacity = battery.capacity();
		bool charging = battery.charging();
		if (charging)
			load = 100;
		else if (capacity <= 5)
			load = 0;
		else if (capacity <= 15)
			load /= 2;
	}

	if (debug)
		std::cout << "[*] Load: " << std::to_string(load) << "\t";

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

void subcore::setup_levels() {
	std::string hotplug = cpu.hotplug();
	while (hotplug != "") {
		cpu.hotplug(hotplug, false);
		hotplug = cpu.hotplug();
	}
	
	uint16_t mp_pid = io::get_pid("mpdecision");
	if (mp_pid != 0)
		kill(mp_pid, SIGTERM);

	level_0.load_requirement = 10;
	level_0.state = state_level_0;
	level_1.load_requirement = 40;
	level_1.state = state_level_1;
	level_2.load_requirement = 70;
	level_2.state = state_level_2;
	level_3.load_requirement = 100;
	level_3.state = state_level_3;
	
	level_0.gov_pref = std::vector<std::string> {
		"interactive",
		"schedutil",
		"performance"
	};
	level_1.gov_pref = std::vector<std::string> {
		"interactive",
		"schedutil",
		"performance"
	};
	level_2.gov_pref = std::vector<std::string> {
		"interactive",
		"schedutil",
		"performance"
	};
	level_3.gov_pref = std::vector<std::string> {
		"interactive",
		"schedutil",
		"performance"
	};
		
	uint8_t online = cpu.online();
	for (size_t i = 0; i < online; i++) {
		cpu.online(i, true);
		std::vector<uint32_t> cpu_avail_freqs = cpu.freqs(i);
		if (cpu_avail_freqs.size() > 0) {					
			std::vector<uint32_t>::iterator itr = std::find(cpu_avail_freqs.begin(), cpu_avail_freqs.end(), cpu.min_freq(i));
			uint8_t offset = std::distance(cpu_avail_freqs.begin(), itr);
			cpu_freq_offsets.push_back(offset);

			level_0.level_data.cpu_max_freqs.push_back(freq_from_percent(cpu_avail_freqs, 20, offset));
			level_1.level_data.cpu_max_freqs.push_back(freq_from_percent(cpu_avail_freqs, 30, offset));
			level_2.level_data.cpu_max_freqs.push_back(freq_from_percent(cpu_avail_freqs, 45, offset));
			level_3.level_data.cpu_max_freqs.push_back(cpu_avail_freqs[cpu_avail_freqs.size() - 1]);

			level_0.level_data.cpu_min_freqs.push_back(cpu_avail_freqs[0]);
			level_1.level_data.cpu_min_freqs.push_back(cpu_avail_freqs[0]);
			level_2.level_data.cpu_min_freqs.push_back(cpu_avail_freqs[0]);
			level_3.level_data.cpu_min_freqs.push_back(cpu_avail_freqs[0]);
		} else {
			level_0.level_data.cpu_max_freqs.push_back(0);
			level_1.level_data.cpu_max_freqs.push_back(0);
			level_2.level_data.cpu_max_freqs.push_back(0);
			level_3.level_data.cpu_max_freqs.push_back(0);

			level_0.level_data.cpu_min_freqs.push_back(0);
			level_1.level_data.cpu_min_freqs.push_back(0);
			level_2.level_data.cpu_min_freqs.push_back(0);
			level_3.level_data.cpu_min_freqs.push_back(0);
		}
		level_0.level_data.cpu_govs.push_back(preferred_gov(level_0.gov_pref));
		level_1.level_data.cpu_govs.push_back(preferred_gov(level_1.gov_pref));
		level_2.level_data.cpu_govs.push_back(preferred_gov(level_2.gov_pref));
		level_3.level_data.cpu_govs.push_back(preferred_gov(level_3.gov_pref));
	}

	std::vector<uint16_t> gpu_avail_freqs = gpu.freqs();
	if (gpu_avail_freqs.size() > 1) {
		std::vector<uint16_t>::iterator itr = std::find(gpu_avail_freqs.begin(), gpu_avail_freqs.end(), gpu.min_freq());
		uint8_t offset = std::distance(gpu_avail_freqs.begin(), itr);
		gpu_freq_offsets.push_back(offset);

		level_0.level_data.gpu_max_freq = freq_from_percent(gpu_avail_freqs, 50, offset);
		level_1.level_data.gpu_max_freq = freq_from_percent(gpu_avail_freqs, 70, offset);
		level_2.level_data.gpu_max_freq = gpu_avail_freqs[gpu_avail_freqs.size() - 1];
		level_3.level_data.gpu_max_freq = gpu_avail_freqs[gpu_avail_freqs.size() - 1];

		level_0.level_data.gpu_min_freq = gpu_avail_freqs[0];
		level_1.level_data.gpu_min_freq = gpu_avail_freqs[0];
		level_2.level_data.gpu_min_freq = gpu_avail_freqs[0];
		level_3.level_data.gpu_min_freq = gpu_avail_freqs[0];
	} else {
		level_0.level_data.gpu_max_freq = 0;
		level_1.level_data.gpu_max_freq = 0;
		level_2.level_data.gpu_max_freq = 0;
		level_3.level_data.gpu_max_freq = 0;
		
		level_0.level_data.gpu_min_freq = 0;
		level_1.level_data.gpu_min_freq = 0;
		level_2.level_data.gpu_min_freq = 0;
		level_3.level_data.gpu_min_freq = 0;
	}

	std::vector<std::string> blkdevs = block.blkdevs();
	for (size_t i = 0; i < blkdevs.size(); i++) {
		level_0.level_data.readaheads.push_back(2048);
		level_0.level_data.ioscheds.push_back("noop");
		level_1.level_data.readaheads.push_back(2048);
		level_1.level_data.ioscheds.push_back("deadline");
		level_2.level_data.readaheads.push_back(2048);
		level_2.level_data.ioscheds.push_back("deadline");
		level_3.level_data.readaheads.push_back(2048);
		level_3.level_data.ioscheds.push_back("deadline");
	}

	level_0.level_data.lmk_minfree = block.LMK_LIGHT;
	level_0.level_data.swappiness = 0;
	level_0.level_data.cache_pressure = 30;
	level_0.level_data.dirty_ratio = 90;
	level_0.level_data.dirty_background_ratio = 80;
	level_0.level_data.entropy_read = 1024;
	level_0.level_data.entropy_write = 2048;
	level_0.level_data.subcore_scan_ms = 500;
	level_0.level_data.laptop_mode = 1;
	level_0.level_data.oom_kill_allocating_task = 0;
	level_0.level_data.overcommit_memory = 0;
	level_0.level_data.page_cluster = 0;
	level_0.level_data.ksm = 0;
	level_1.level_data.lmk_minfree = block.LMK_MEDIUM;
	level_1.level_data.swappiness = 20;
	level_1.level_data.cache_pressure = 60;
	level_1.level_data.dirty_ratio = 90;
	level_1.level_data.dirty_background_ratio = 80;
	level_1.level_data.entropy_read = 1024;
	level_1.level_data.entropy_write = 2048;
	level_1.level_data.subcore_scan_ms = 500;
	level_1.level_data.laptop_mode = 1;
	level_1.level_data.oom_kill_allocating_task = 0;
	level_1.level_data.overcommit_memory = 0;
	level_1.level_data.page_cluster = 0;
	level_1.level_data.ksm = 0;
	level_2.level_data.lmk_minfree = block.LMK_MEDIUM;
	level_2.level_data.swappiness = 30;
	level_2.level_data.cache_pressure = 70;
	level_2.level_data.dirty_ratio = 90;
	level_2.level_data.dirty_background_ratio = 80;
	level_2.level_data.entropy_read = 1024;
	level_2.level_data.entropy_write = 2048;
	level_2.level_data.subcore_scan_ms = 500;
	level_2.level_data.laptop_mode = 1;
	level_2.level_data.oom_kill_allocating_task = 0;
	level_2.level_data.overcommit_memory = 1;
	level_2.level_data.page_cluster = 3;
	level_2.level_data.ksm = 0;
	level_3.level_data.lmk_minfree = block.LMK_AGGRESSIVE;
	level_3.level_data.swappiness = 40;
	level_3.level_data.cache_pressure = 80;
	level_3.level_data.dirty_ratio = 90;
	level_3.level_data.dirty_background_ratio = 80;
	level_3.level_data.entropy_read = 1024;
	level_3.level_data.entropy_write = 2048;
	level_3.level_data.subcore_scan_ms = 500;
	level_3.level_data.laptop_mode = 1;
	level_3.level_data.oom_kill_allocating_task = 0;
	level_3.level_data.overcommit_memory = 1;
	level_3.level_data.page_cluster = 3;
	level_3.level_data.ksm = 0;
	for (size_t i = 0; i < online; i++) {
		interactive_struct interactive_0;
		interactive_struct interactive_1;
		interactive_struct interactive_2;
		interactive_struct interactive_3;
		// universal
		interactive_0.go_hispeed_load = 99;
		interactive_0.above_hispeed_delay = "100000";
		interactive_0.timer_rate = 40000;
		interactive_0.timer_slack = -1;
		interactive_0.min_sample_time = 100000;
		interactive_1.go_hispeed_load = 99;
		interactive_1.above_hispeed_delay = "80000";
		interactive_1.timer_rate = 40000;
		interactive_1.timer_slack = -1;
		interactive_1.min_sample_time = 100000;
		interactive_2.go_hispeed_load = 80;
		interactive_2.above_hispeed_delay = "80000";
		interactive_2.timer_rate = 20000;
		interactive_2.timer_slack = -1;
		interactive_2.min_sample_time = 80000;
		interactive_3.go_hispeed_load = 75;
		interactive_3.above_hispeed_delay = "40000";
		interactive_3.timer_rate = 20000;
		interactive_3.timer_slack = -1;
		interactive_3.min_sample_time = 80000;

		// per cpu
		std::vector<uint32_t> cpu_avail_freqs = cpu.freqs(i);
		if (cpu_avail_freqs.size() >= 4) {
			interactive_0.hispeed_freq = cpu_avail_freqs[cpu_freq_offsets[i]];
			interactive_0.target_loads = ((std::ostringstream&) (std::ostringstream("") << "97 " << freq_from_percent(cpu_avail_freqs, 20, cpu_freq_offsets[i]) << ":99")).str();
			interactive_1.hispeed_freq = freq_from_percent(cpu_avail_freqs, 20, cpu_freq_offsets[i]);
			interactive_1.target_loads = ((std::ostringstream&) (std::ostringstream("") << "95 " << freq_from_percent(cpu_avail_freqs, 20, cpu_freq_offsets[i]) << ":97 " << freq_from_percent(cpu_avail_freqs, 30, cpu_freq_offsets[i]) << ":99")).str();
			interactive_2.hispeed_freq = freq_from_percent(cpu_avail_freqs, 30, cpu_freq_offsets[i]);
			interactive_2.target_loads = ((std::ostringstream&) (std::ostringstream("") << "75 " << freq_from_percent(cpu_avail_freqs, 20, cpu_freq_offsets[i]) << ":80 " << freq_from_percent(cpu_avail_freqs, 30, cpu_freq_offsets[i]) << ":85 " << freq_from_percent(cpu_avail_freqs, 45, cpu_freq_offsets[i]) << ":99")).str();
			interactive_3.hispeed_freq = freq_from_percent(cpu_avail_freqs, 45);
			interactive_3.target_loads = ((std::ostringstream&) (std::ostringstream("") << "70 " << freq_from_percent(cpu_avail_freqs, 20, cpu_freq_offsets[i]) << ":75 " << freq_from_percent(cpu_avail_freqs, 45, cpu_freq_offsets[i]) << ":80 " << cpu_avail_freqs[cpu_avail_freqs.size() - 1] << ":85")).str();
		} else {
			interactive_0.hispeed_freq = 0;
			interactive_0.target_loads = "";
			interactive_1.hispeed_freq = 0;
			interactive_1.target_loads = "";
			interactive_2.hispeed_freq = 0;
			interactive_2.target_loads = "";
			interactive_3.hispeed_freq = 0;
			interactive_3.target_loads = "";
		}
		level_0.level_data.interactives.push_back(interactive_0);
		level_1.level_data.interactives.push_back(interactive_1);
		level_2.level_data.interactives.push_back(interactive_2);
		level_3.level_data.interactives.push_back(interactive_3);
	}
}

void subcore::set_sysfs(level_struct level) {
	if (current_state == level.state) {
		if (display.suspended()) {
			same_level_count = 0;
			return;
		}
		same_level_count++;
		cpu.STAT_AVG_SLEEP_MS = level.level_data.subcore_scan_ms;
		if (same_level_count >= 10) {
			cpu.STAT_AVG_SLEEP_MS += 3000;
			if (debug)
				std::cout << "+3000ms\t";
		} else if (same_level_count >= 5) {
			cpu.STAT_AVG_SLEEP_MS += 1500;
			if (debug)
				std::cout << "+1500ms\t";
			}	
		return;
	}
	uint8_t online = cpu.online();
	for (size_t i = 0; i < online; i++) {
		cpu.gov(i, level.level_data.cpu_govs[i]);
		if (power_aware && display.suspended())
			cpu.max_freq(i, cpu.freqs(i)[0]);
		else
			cpu.max_freq(i, level.level_data.cpu_max_freqs[i]);	
		cpu.min_freq(i, level.level_data.cpu_min_freqs[i]);
		if (level.level_data.cpu_govs[i] == "interactive")
			set_interactive(i, level.level_data.interactives[i]);
	}
	gpu.max_freq(level.level_data.gpu_max_freq);
	gpu.min_freq(level.level_data.gpu_min_freq);
	std::vector<std::string> blkdevs = block.blkdevs();
	for (size_t i = 0; i < blkdevs.size(); i++) {
		block.iosched(blkdevs[i], level.level_data.ioscheds[i]);
		block.read_ahead(blkdevs[i], level.level_data.readaheads[i]);
	}
	if (!low_mem) {
		block.swappiness(level.level_data.swappiness);
		block.cache_pressure(level.level_data.cache_pressure);
		block.dirty_ratio(level.level_data.dirty_ratio);
		block.dirty_background_ratio(level.level_data.dirty_background_ratio);
		uint32_t ram_size = memory.ram_size() / 400;
		std::stringstream lmk_minfree_stream(level.level_data.lmk_minfree);
		std::string new_lmk_minfree;
		int offset;
		while (lmk_minfree_stream >> offset) {
			new_lmk_minfree += std::to_string(offset * ram_size) + ",";
			if (lmk_minfree_stream.peek() == ',')
				lmk_minfree_stream.ignore();
		}
		new_lmk_minfree.erase(new_lmk_minfree.size() - 1);
		block.lmk(new_lmk_minfree);
		block.laptop_mode(level.level_data.laptop_mode);
		block.oom_kill_allocating_task(level.level_data.oom_kill_allocating_task);
		block.overcommit_memory(level.level_data.overcommit_memory);
		block.page_cluster(level.level_data.page_cluster);
		block.ksm(level.level_data.ksm);
	}
	block.entropy_read(level.level_data.entropy_read);
	block.entropy_write(level.level_data.entropy_write);
	cpu.STAT_AVG_SLEEP_MS = level.level_data.subcore_scan_ms;
	current_state = level.state;
	same_level_count = 0;
}

void subcore::set_interactive(uint8_t core, interactive_struct interactive) {
	std::string PATH_INTERACTIVE;
	
	if (io::path_exists("/sys/devices/system/cpu/cpufreq/interactive"))
		PATH_INTERACTIVE = "/sys/devices/system/cpu/cpufreq/interactive";
	else if (io::path_exists("/sys/devices/system/cpu/cpufreq/policy" + std::to_string(core) + "/interactive"))
		PATH_INTERACTIVE = "/sys/devices/system/cpu/cpufreq/policy" + std::to_string(core) + "/interactive";
	else
		return;

	io::write_file(PATH_INTERACTIVE + "/go_hispeed_load", std::to_string(interactive.go_hispeed_load));
	io::write_file(PATH_INTERACTIVE + "/above_hispeed_delay", interactive.above_hispeed_delay);
	io::write_file(PATH_INTERACTIVE + "/timer_rate", std::to_string(interactive.timer_rate));
	io::write_file(PATH_INTERACTIVE + "/timer_slack", std::to_string(interactive.timer_slack));
	io::write_file(PATH_INTERACTIVE + "/min_sample_time", std::to_string(interactive.min_sample_time));
	io::write_file(PATH_INTERACTIVE + "/hispeed_freq", std::to_string(interactive.hispeed_freq));
	io::write_file(PATH_INTERACTIVE + "/target_loads", interactive.target_loads);
}

std::string subcore::preferred_gov(std::vector<std::string> pref_govs) {
	std::vector<std::string> cpu_avail_govs = cpu.govs();
	std::string new_cpu_pref_gov;
	for (std::string pref_gov : pref_govs)
		for (std::string cpu_avail_gov : cpu_avail_govs)
			if (cpu_avail_gov == pref_gov)
				return pref_gov;

	// double failsafe
	return "performance";
}

uint32_t subcore::freq_from_percent(std::vector<uint32_t> avail_freqs, uint8_t percent, uint8_t offset) {
	int16_t index = round(avail_freqs.size() * ((float) percent / 100)) - 1 + offset;
	if (index < 0)
		index = 0;
	if (index > (uint16_t) avail_freqs.size() - 1)
		index = avail_freqs.size() - 1;
	return avail_freqs[index];
}

uint32_t subcore::freq_from_percent(std::vector<uint16_t> avail_freqs, uint8_t percent, uint8_t offset) {
	int16_t index = round(avail_freqs.size() * ((float) percent / 100)) - 1 + offset;
	if (index < 0)
		index = 0;
	if (index > (uint16_t) avail_freqs.size() - 1)
		index = avail_freqs.size() - 1;
	return avail_freqs[index];
}

