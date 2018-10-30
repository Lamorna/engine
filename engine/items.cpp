#include "items.h"
#include "vector.h"
#include "component.h"
#include "input.h"


//======================================================================

static const float3_ scale_velocity = { 1.0f, 1.0f, 1.0f };
static const float3_ scale_start = { 0.25f, 0.25f, 0.25f};
   
//======================================================================
/*
==================
==================
*/
void systems_::item_::update_ammo(

	void* parameters, __int32 i_thread

	)
{
	parameters_::item_::update_ammo_* func_parameters = (parameters_::item_::update_ammo_*)parameters;
	const timer_& timer = *func_parameters->timer;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	__int32 i_entity_camera;
	component_::component_::colour_* colour_player;
	component_::component_::weapon_* weapon_player;
	{
		component_fetch_ fetch;
		fetch.n_components = 3;
		fetch.component_ids[0] = component_id_::COLOUR;
		fetch.component_ids[1] = component_id_::WEAPON;
		fetch.component_ids[2] = component_id_::CAMERA;
		fetch.n_excludes = 0;

		Populate_Fetch_Table(archetype_data, fetch);

		const __int32 i_archetype_index = 0;
		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];

		colour_player = (component_::colour_*)fetch.table[0][i_archetype_index];
		weapon_player = (component_::weapon_*)fetch.table[1][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < archetype.n_entities; i_entity++) {

			if (Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) {
				i_entity_camera = i_entity;
			}
		}
	}

	component_fetch_ fetch;
	fetch.n_components = 4;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::COLOUR;
	fetch.component_ids[2] = component_id_::ITEM;
	fetch.component_ids[3] = component_id_::TRIGGER;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::colour_* colour = (component_::colour_*)fetch.table[1][i_archetype_index];
		component_::item_* item = (component_::item_*)fetch.table[2][i_archetype_index];
		component_::trigger_* trigger = (component_::trigger_*)fetch.table[3][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			__int32 new_state = item[i_entity].state;
			bool is_picked_up = item[i_entity].state == component_::item_::state_::PICKED_UP;
			//item[i_entity].timer += blend(timer.delta_time, 0.0f, is_picked_up);
			item[i_entity].timer += is_picked_up ? timer.delta_time : 0.0f;

			bool is_timer = item[i_entity].timer > item[i_entity].spawn_time;
			//new_state = blend_int(component_::item_::state_::SPAWNED, new_state, is_timer);
			new_state = is_timer ? component_::item_::state_::SPAWNED : new_state;
			//item[i_entity].timer = blend(0.0f, item[i_entity].timer, is_timer);
			item[i_entity].timer = is_timer ? 0.0f : item[i_entity].timer;

			//new_state = blend_int(component_::item_::state_::PICKED_UP, new_state, trigger[i_entity].is_triggered);
			new_state = trigger[i_entity].is_triggered ? component_::item_::state_::PICKED_UP : new_state;

			bool is_spawned = new_state == component_::item_::state_::SPAWNED;

			for (__int32 i = X; i < W; i++) {

				//base[i_entity].position_fixed.i[i] = blend_int(item[i_entity].spawn_position.i[i], item[i_entity].despawn_position.i[i], is_spawned);
				base[i_entity].position_fixed.i[i] = is_spawned ? item[i_entity].spawn_position.i[i] : item[i_entity].despawn_position.i[i];
				//base[i_entity].scale.f[i] = blend(base[i_entity].scale.f[i], scale_start.f[i], is_spawned);
				base[i_entity].scale.f[i] = is_spawned ? base[i_entity].scale.f[i] : scale_start.f[i];
				float delta_scale = scale_velocity.f[i] * timer.delta_time;
				//base[i_entity].scale.f[i] += blend(delta_scale, 0.0f, is_spawned);
				base[i_entity].scale.f[i] += is_spawned ? delta_scale : 0.0f;
				base[i_entity].scale.f[i] = min(base[i_entity].scale.f[i], item[i_entity].default_scale.f[i]);
			}

			bool is_state_change = new_state != item[i_entity].state;
			//item[i_entity].state_trigger = blend_int(new_state, component_::item_::state_::NULL_, is_state_change);
			item[i_entity].state_trigger = is_state_change ? new_state : component_::item_::state_::NULL_;
			item[i_entity].state = new_state;

			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				//colour_player[i_entity_camera].colour.f[i_axis] = blend(colour[i_entity].colour.f[i_axis], colour_player[i_entity_camera].colour.f[i_axis], trigger[i_entity].is_triggered);
				colour_player[i_entity_camera].colour.f[i_axis] = trigger[i_entity].is_triggered ? colour[i_entity].colour.f[i_axis] : colour_player[i_entity_camera].colour.f[i_axis];
			}
			//weapon_player[i_entity_camera].projectile_id = blend_int(item[i_entity].id, weapon_player[i_entity_camera].projectile_id, trigger[i_entity].is_triggered);
			weapon_player[i_entity_camera].projectile_id = trigger[i_entity].is_triggered ? item[i_entity].id : weapon_player[i_entity_camera].projectile_id;
			weapon_player[i_entity_camera].ammo_count += item[i_entity].state_trigger == component_::item_::state_::PICKED_UP ? component_::weapon_::ammo_::LOAD : 0;
			weapon_player[i_entity_camera].ammo_count = min(weapon_player[i_entity_camera].ammo_count, component_::weapon_::ammo_::MAX);
		}
	}
}

/*
==================
==================
*/
void systems_::item_::process_triggers(

	void* parameters, __int32 i_thread

)
{
	parameters_::item_::process_triggers_* func_parameters = (parameters_::item_::process_triggers_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	int3_ position_player;
	int3_ extent_player;
	{
		component_fetch_ fetch;
		fetch.n_components = 3;
		fetch.component_ids[0] = component_id_::BASE;
		fetch.component_ids[1] = component_id_::CAMERA;
		fetch.component_ids[2] = component_id_::BOUNDING_BOX;
		fetch.n_excludes = 0;

		Populate_Fetch_Table(archetype_data, fetch);

		const __int32 i_archetype_index = 0;
		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::bounding_box_* bounding_box = (component_::bounding_box_*)fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < archetype.n_entities; i_entity++) {

			if (Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) {
				position_player = base[i_entity].position_fixed;
				extent_player = bounding_box[i_entity].extent;
			}
		}
	}

	component_fetch_ fetch;
	fetch.n_components = 1;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::TRIGGER;

	Populate_Fetch_Table(archetype_data, fetch);

	//enum {

	//	PLAYER,
	//	PICKUP,
	//	MAX_ENTRIES = 64,
	//};

	//struct pair_ {

	//	__int32 value;
	//	__int32 i_entity;
	//	__int32 id;
	//};

	//__int32 n_entries = 0;
	//pair_ sap[3][MAX_ENTRIES];

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::trigger_* trigger = (component_::trigger_*)fetch.table[0][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			{
				//sap[X][n_entries].value = trigger[i_entity].position_fixed.x + trigger[i_entity].extent_fixed.x;
				//sap[Y][n_entries].value = trigger[i_entity].position_fixed.y + trigger[i_entity].extent_fixed.y;
				//sap[Z][n_entries].value = trigger[i_entity].position_fixed.z + trigger[i_entity].extent_fixed.z;

				//sap[X][n_entries].i_entity = i_entity;
				//sap[Y][n_entries].i_entity = i_entity;
				//sap[Z][n_entries].i_entity = i_entity;

				//sap[X][n_entries].id = PICKUP;
				//sap[Y][n_entries].id = PICKUP;
				//sap[Z][n_entries].id = PICKUP;

				//n_entries++;

				//sap[X][n_entries].value = trigger[i_entity].position_fixed.x - trigger[i_entity].extent_fixed.x;
				//sap[Y][n_entries].value = trigger[i_entity].position_fixed.y - trigger[i_entity].extent_fixed.y;
				//sap[Z][n_entries].value = trigger[i_entity].position_fixed.z - trigger[i_entity].extent_fixed.z;

				//sap[X][n_entries].i_entity = i_entity;
				//sap[Y][n_entries].i_entity = i_entity;
				//sap[Z][n_entries].i_entity = i_entity;

				//sap[X][n_entries].id = PICKUP;
				//sap[Y][n_entries].id = PICKUP;
				//sap[Z][n_entries].id = PICKUP;

				//n_entries++;
			}

			__int32 bit_set = 0x1;
			bit_set &= abs(position_player.x - trigger[i_entity].position_fixed.x) < (trigger[i_entity].extent_fixed.x + extent_player.x);
			bit_set &= abs(position_player.y - trigger[i_entity].position_fixed.y) < (trigger[i_entity].extent_fixed.y + extent_player.y);
			bit_set &= abs(position_player.z - trigger[i_entity].position_fixed.z) < (trigger[i_entity].extent_fixed.z + extent_player.z);

			trigger[i_entity].is_triggered = bit_set != 0x0;
		}
	}

	//// sort
	//pair_ temp_sort[MAX_ENTRIES];

	//for (__int32 i_axis = X; i_axis < W; i_axis++) {

	//	for (__int32 i_sorted = 0; i_sorted < n_entries; i_sorted++) {

	//		__int32 find_min = INT_MAX;
	//		__int32 i_min = INVALID_RESULT;
	//		for (__int32 i_entry = 0; i_entry < n_entries; i_entry++) {
	//			bool is_less = sap[i_axis][i_entry].value < find_min;
	//			i_min = is_less ? i_entry : i_min;
	//			find_min = is_less ? sap[i_axis][i_entry].value : find_min;
	//		}
	//		temp_sort[i_sorted] = sap[i_axis][i_min];
	//		sap[i_axis][i_min].value = INT_MAX;
	//	}

	//	for (__int32 i_sorted = 0; i_sorted < n_entries; i_sorted++) {
	//		sap[i_axis][i_sorted] = temp_sort[i_sorted];
	//	}
	//}

	//for (__int32 i_axis = X; i_axis < W; i_axis++) {

	//	for (__int32 i_sorted = 0; i_sorted < n_entries; i_sorted++) {
	//		printf_s("%i ,", sap[i_axis][i_sorted].value);
	//	}
	//	printf_s("\n");
	//	for (__int32 i_sorted = 0; i_sorted < n_entries; i_sorted++) {
	//		printf_s("%i ,", sap[i_axis][i_sorted].i_entity);
	//	}
	//	printf_s("\n");
	//}
	//printf_s("\n");
}

