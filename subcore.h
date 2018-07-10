#ifndef SUBCORE_H
#define SUBCORE_H

#include "sysfs.h"

class Subcore {
	private:
		struct alg_preset {
			std::string iosched;
			std::string cpu_gov;
			int cpu_max_freq;
			int gpu_max_freq;
			std::string lmk_adj;
			int new_subcore_scan;
			int readahead;
			int swappiness;
		};

		alg_preset idle;
		alg_preset awake;

		SysFs::Cpu cpu;
		SysFs::Block block;

	public:
		void algorithm(alg_preset new_preset);
};

#endif

