
#pragma once	

#include "master.h"

struct timer_;
struct archetype_data_;

struct systems_::item_ {

	static void process_triggers(void*, __int32);
	static void update_ammo(void*, __int32);
};

struct parameters_::item_ {

	struct process_triggers_ {

		const timer_* timer;
		archetype_data_* archetype_data;
	};
	struct update_ammo_ {

		const timer_* timer;
		archetype_data_* archetype_data;
	};

	process_triggers_ process_triggers;
	update_ammo_ update_ammo;
};
