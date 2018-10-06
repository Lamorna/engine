
#include "master.h"
#include "vector.h"


struct timer_;
struct archetype_data_;

struct systems_::environment_ {

	static void update_clouds(void*, __int32);
	static void update_lava(void*, __int32);
	static void update_doors(void*, __int32);
	static void update_platforms(void*, __int32);
	static void update_switches(void*, __int32);
	static void update_buttons(void*, __int32);

};

struct parameters_::environment_ {

	struct update_clouds_ {

		const matrix* m_rotate;
		archetype_data_* archetype_data;
	};
	struct update_lava_ {

		const matrix* m_rotate;
		archetype_data_* archetype_data;
	};
	struct update_doors_ {

		const timer_* timer;
		archetype_data_* archetype_data;
	};
	struct update_platforms_ {

		const timer_* timer;
		archetype_data_* archetype_data;
	};
	struct update_switches_ {

		const timer_* timer;
		archetype_data_* archetype_data;
	};
	struct update_buttons_ {

		const timer_* timer;
		archetype_data_* archetype_data;
	};

	update_clouds_ update_clouds;
	update_lava_ update_lava;
	update_doors_ update_doors;
	update_platforms_ update_platforms;
	update_switches_ update_switches;
	update_buttons_ update_buttons;

};