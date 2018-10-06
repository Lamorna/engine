#include "component.h"

component_::weapon_::projectile_data_ component_::weapon_::projectile_data[id_::COUNT]; // = { 1.0f, 2.0f, 2.0f, 3.0f, 3.0f };


/*
==================
==================
*/
void Toggle_Component_Bit(

	const __int32 i_archetype,
	const __int32 i_entity,
	const __int32 component_id,
	archetype_data_& archetype_data
) {

	archetype_& archetype = archetype_data.archetypes[i_archetype];
	__int32 group_id = component_id / component_data_::NUM_BITS_INTEGER;
	__int32 group_index = component_id - (group_id * component_data_::NUM_BITS_INTEGER);
	unsigned __int32 set_bit = archetype.component_masks[group_id] & (0x1 << group_index);
	archetype.entity_component_masks[i_entity][group_id] ^= set_bit;
}

/*
==================
==================
*/
void Set_Component_Bit(

	const __int32 i_archetype,
	const __int32 i_entity,
	const __int32 component_id,
	archetype_data_& archetype_data
) {

	archetype_& archetype = archetype_data.archetypes[i_archetype];
	__int32 group_id = component_id / component_data_::NUM_BITS_INTEGER;
	__int32 group_index = component_id - (group_id * component_data_::NUM_BITS_INTEGER);
	unsigned __int32 set_bit = archetype.component_masks[group_id] & (0x1 << group_index);
	archetype.entity_component_masks[i_entity][group_id] |= set_bit;
}

/*
==================
==================
*/
void Clear_Component_Bit(

	const __int32 i_archetype,
	const __int32 i_entity,
	const __int32 component_id,
	archetype_data_& archetype_data
) {

	archetype_& archetype = archetype_data.archetypes[i_archetype];
	__int32 group_id = component_id / component_data_::NUM_BITS_INTEGER;
	__int32 group_index = component_id - (group_id * component_data_::NUM_BITS_INTEGER);
	archetype.entity_component_masks[i_entity][group_id] &= (~(0x1 << group_index));
}

/*
==================
==================
*/
__int8* Find_Archetype_Component(

	const __int32 i_archetype,
	const __int32 component_id,
	const archetype_data_& archetype_data
) {

	const archetype_& archetype = archetype_data.archetypes[i_archetype];

	for (__int32 i_component = 0; i_component < archetype.n_components; i_component++) {

		if (component_id == archetype.component_ids[i_component]) {
			return archetype.mem_ptrs[i_component];
		}
	}

	return NULL;
}

/*
==================
==================
*/
bool Match_Entity_Mask(

	const __int32 i_entity,
	const unsigned __int32 component_masks[component_data_::NUM_MASKS],
	const archetype_& archetype
) {

	unsigned __int32 include_result = 0x1;
	for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
		include_result &= (archetype.entity_component_masks[i_entity][i_group] & component_masks[i_group]) == component_masks[i_group];
	}

	bool is_match = include_result != 0x0;
	return is_match;
}


/*
==================
==================
*/
void Populate_Fetch_Table(

	const archetype_data_& archetype_data,
	component_fetch_& component_fetch

) {

	Convert_Component_list_To_Mask(component_fetch.n_components, component_fetch.component_ids, component_fetch.component_masks);
	Convert_Component_list_To_Mask(component_fetch.n_excludes, component_fetch.exclude_ids, component_fetch.exclude_masks);

	component_fetch.n_archetypes = 0;
	component_fetch.n_entities = 0;

	for (__int32 i_archetype = 0; i_archetype < archetype_data.n_archetypes; i_archetype++) {

		const archetype_& archetype = archetype_data.archetypes[i_archetype];

		unsigned __int32 include_result = 0x1;
		unsigned __int32 exclude_result = 0x0;
		for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
			include_result &= (archetype.component_masks[i_group] & component_fetch.component_masks[i_group]) == component_fetch.component_masks[i_group];
			exclude_result |= archetype.component_masks[i_group] & component_fetch.exclude_masks[i_group];
		}

		bool is_archetype_match = (include_result != 0x0) && (exclude_result == 0x0);

		if (is_archetype_match) {

			for (__int32 i_component = 0; i_component < component_fetch.n_components; i_component++) {

				const __int32 component_id = component_fetch.component_ids[i_component];

				for (__int32 i_component_archetype = 0; i_component_archetype < archetype.n_components; i_component_archetype++) {

					bool is_component_match = component_id == archetype.component_ids[i_component_archetype];

					if (is_component_match) {
						component_fetch.table[i_component][component_fetch.n_archetypes] = archetype.mem_ptrs[i_component_archetype];
					}
				}
			}

			component_fetch.n_entities += archetype.n_entities;
			component_fetch.i_archetypes[component_fetch.n_archetypes] = i_archetype;
			component_fetch.n_archetypes++;
		}
	}
}

/*
==================
==================
*/
__int32 Return_Camera_Component(

	const archetype_data_& archetype_data,
	component_::camera_** camera

)
{
	component_fetch_ component_fetch;
	component_fetch.n_components = 1;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::CAMERA;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		*camera = (component_::camera_*)component_fetch.table[0][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) {
				return i_entity;
			};
		}
	}

	return -1;
}

/*
==================
==================
*/
void Convert_Component_list_To_Mask(

	const __int32 n_components,
	const __int32 components[],
	unsigned __int32 mask[component_data_::NUM_MASKS]
) {


	for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
		mask[i_group] = 0x0;
	}

	for (__int32 i_component = 0; i_component < n_components; i_component++) {

		const __int32 component_id = components[i_component];
		const __int32 group_id = component_id / component_data_::NUM_BITS_INTEGER;
		const __int32 group_index = component_id - (group_id * component_data_::NUM_BITS_INTEGER);
		mask[group_id] |= 1 << group_index;
	}
}