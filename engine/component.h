#pragma once

#include "vector.h"

//======================================================================

struct behaviour_node_ {

	__int32 behaviour_id;
	__int32 i_next_node;
	float timer;
};

struct component_id_ {

	enum {

		BASE,
		MOVE,
		ANIMATION,
		BEHAVIOUR,
		PROJECTILE,
		PROJECTILE_ID,
		COLLIDER,
		SMALL_MODEL_ID,
		COLLIDEE_STATIC,
		COLLIDEE_MOVING,
		COLLIDEE_ROTATED,
		VERTEX_LIGHT_SOURCE,
		LIGHTMAP_LIGHT_SOURCE,
		PATROL_POINT,
		PATROL,
		TRIGGER,
		SPAWN,
		MODEL_SPACE,
		MODEL_SPACE_UPDATE,
		COLOUR,
		COLOUR_SPACE,
		TEXTURE_SPACE,
		DRAW,
		ITEM,
		CAMERA,
		TELEPORTER,
		LAVA,
		DOOR,
		CLOUD,
		PLATFORM,
		MAP_COLLIDE,
		MAP_RENDER,
		PARTICLE,
		ANIMATION_DRIVER,
		EFFECT,
		FLY,
		TEXTURE_BLEND,
		TRAP_DOOR_ID,
		PLATE,
		BUTTON,
		SWITCH,
		SOUND_TRIGGER,
		POWER,
		ATTACHED,
		MASS,
		HEALTH,
		WEAPON,
		BOUNDING_BOX,
		PROP,
		MOB,
		MAP_BVH,
		MODEL,

		COUNT,
	};
};


enum {

	NUM_BITS_INTEGER_RAW = 32,
	NUM_MASKS_RAW = (component_id_::COUNT / NUM_BITS_INTEGER_RAW) + 1,
};


struct component_ {

	struct small_model_id_;
	struct collidee_static_;
	struct collidee_moving_;
	struct collidee_rotated_;
	struct vertex_light_source_;
	struct lightmap_light_source_;
	struct patrol_point_;
	struct patrol_;
	struct teleporter_;
	struct trigger_;
	struct door_;
	struct item_;
	struct cloud_;
	struct spawn_;
	struct colour_;
	struct colour_space_;
	struct draw_;
	struct model_space_;
	struct model_space_update_;
	struct lava_;
	struct platform_;
	struct texture_space_;
	struct map_collide_;
	struct map_render_;
	struct particle_;
	struct animation_;
	struct base_;
	struct move_;
	struct behaviour_;
	struct projectile_;
	struct collider_;
	struct camera_;
	struct projectile_id_;
	struct animation_driver_;
	struct effect_;
	struct fly_;
	struct texture_blend_;
	struct trap_door_id_;
	struct plate_;
	struct button_;
	struct switch_;
	struct sound_trigger_;
	struct power_;
	struct attached_;
	struct mass_;
	struct health_;
	struct weapon_;
	struct bounding_box_;
	struct prop_;
	struct mob_;
	struct map_bvh_;
	struct model_;
};

struct component_::small_model_id_ {

	__int32 id;
};
struct component_::collidee_static_ {

	__int32 entity_type;
	__int32 entity_id;
	int3_ extent;
};
struct component_::collidee_moving_ {

	__int32 entity_type;
	__int32 entity_id;
	int3_ extent;
};
struct component_::collidee_rotated_ {

	__int32 entity_type;
	__int32 entity_id;
	int3_ extent;
	int3_ normals[3];
};
struct component_::vertex_light_source_ {

	__int32 id;
	float intensity;
};
struct component_::lightmap_light_source_ {

	__int32 id;
	float intensity;
};
struct component_::patrol_point_ {

	__int32 id;
};
struct component_::patrol_ {

	enum {
		MAX_POINTS = 8,
	};

	__int32 n_points;
	__int32 i_current;
	__int32 point_ids[MAX_POINTS];
};
struct component_::teleporter_ {

	int3_ destination;
};
struct component_::trigger_ {

	bool is_triggered;
	int3_ position_fixed;
	int3_ extent_fixed;
};
struct component_::door_ {

	struct state_ {

		enum {

			NULL_ = -1,
			MOVE,
			STATIC,
		};
	};

	__int32 state;
	__int32 state_trigger;
	float interval;
	int3_ position_default;
	float3_ move;
};
struct component_::item_ {

	struct type_ {

		enum {

			NULL_ = -1,
			KILL,
			SHRINK,
			PETRIFY,
			PUSH,
			PULL,

			COUNT,
		};
	};

	struct state_ {

		enum {

			NULL_ = -1,
			PICKED_UP,
			SPAWNED,
		};
	};

	__int32 state;
	__int32 state_trigger;
	__int32 id;
	float timer;
	float spawn_time;
	int3_ spawn_position;
	int3_ despawn_position;
	float3_ default_scale;
};
struct component_::cloud_ {

	float3_ base_scale;
	float3_ base_colour;
	float3_ seed_scale;
};
struct component_::spawn_ {

	int3_ position;
};
struct component_::colour_ {

	float3_ colour;
};
struct component_::colour_space_ {

	float4_ q_add;
	float4_ q_rotate;
	matrix_ m_rotate;
};
struct component_::draw_ {

	__int32 draw_id;
	//__int32 model_id;
};
struct component_::model_space_ {

	matrix_ m_rotate;
};
struct component_::model_space_update_ {

	float4_ q_add;
	float4_ q_rotate;
};
struct component_::lava_ {

	float3_ base_distance;
	float3_ base_colour;
	int3_ origin;
};
struct component_::platform_ {

	__int32 axis_travel;
	float contact_mass;
	float3_ intervals;
	float3_ speeds;
	int3_ prev_position;
	int3_ endpoints[2];

	struct blend_ {

		float t_interval;
		float blend_speed;
	};
	blend_ blend;
};
struct component_::texture_space_ {

	float4_ q_add;
	float4_ q_rotate;
	matrix_ m_rotate;
};
struct component_::map_collide_ {

	__int32 entity_type;
	__int32 entity_id;
	int3_ extent;
};
struct component_::map_render_ {

	__int32 id;
};
struct component_::particle_ {

	__int32 id;
	float3_ base_velocity;
};
struct component_::animation_ {

	enum {

		CURRENT_FRAME_LOW,
		CURRENT_FRAME_HI,
		NEXT_FRAME,

	};

	bool is_frame_change;
	bool is_end_animation;
	__int32 model_id;
	__int32 i_current;
	__int32 trigger_id;
	__int32 i_frames[3];
	float frame_speed;
	float frame_speed_modifier;
	float frame_interval;
	float3_ origin;
};
struct component_::animation_driver_ {

	__int32 i_current_animation;
	__int32 trigger_id;
};

struct component_::base_ {

	int3_ position_fixed;
	float3_ scale;
};
struct component_::move_ {

	float max_speed;
	float3_ velocity;
	float3_ displacement;

};
struct component_::behaviour_ {

	struct id_ {

		enum {

			NULL_ = -1,
			STAND,
			MOVE_PATROL,
			CHARGE_PLAYER,
			ATTACK,
			DEATH,
			DEAD,
			SHRINK,
			PAIN,

			COUNT,
		};
	};

	struct target_id_ {

		enum {
			NULL_ = -1,
			PATROL_POINT,
			PLAYER,
		};
	};

	enum {
		MAX_BEHAVIOUR_NODES = 16,
	};

	__int32 state;
	__int32 state_trigger;
	behaviour_node_ behaviour_nodes[MAX_BEHAVIOUR_NODES];
	__int32 i_node;
	__int32 n_nodes;
	__int32 trigger_id;
};

struct component_::projectile_ {

	__int32 i_projectile;
	__int32 i_begin;
	__int32 n_projectiles;
	__int32 type_id;
	float timer;
	bool is_fired;

	struct model_data_ {

		__int32 i_firing_frame;
		__int32 i_vertex_left;
		__int32 i_vertex_right;
		float3_ displacement;
	};

	model_data_ model_data;
};

struct component_::collider_ {

	bool is_ground_contact;
	__int32 entity_type;
	__int32 entity_id;
	int3_ extent;
};
struct component_::camera_ {

	__int32 id;
	float angle_pitch;
	float angle_yaw;
	float t_smooth_step;
	float acceleration;
	int3_ offset;
	int3_ position_fixed;
	int3_ previous_position;
	float4_ q_yaw;
	matrix_ m_camera_space;
	matrix_ m_projection;
	matrix_ m_clip_space;
	bool is_jumped;
	bool is_stepped;
};
struct component_::projectile_id_ {

	__int32 id;
	__int32 type_id;
};
struct component_::effect_ {

	__int32 trigger_id;

	struct shrink_ {

		float timer;
		float duration;
		float t_interval;
		float3_ base_scale;
		float3_ shrunk_scale;
	};

	struct petrify_ {

		bool is_running;
		float timer;
		float time_limit;
		float t_interval;
		float begin_time_modifier;
		float end_time_modifier;
	};

	petrify_ petrify;
	shrink_ shrink;
};

struct component_::fly_ {

	float acceleration_rise;
	float max_air_speed;
	float min_air_speed;
	float gravity_modifier_max;
	float gravity_modifier_min;
	__int32 ceiling_fixed;
	__int32 floor_fixed;
};

struct component_::texture_blend_ {

	float interval;
};
struct component_::trap_door_id_ {

	__int32 id;
};
struct component_::plate_ {

	__int32 sound_trigger;
	float t_interval;
	float3_ colours[2];
	int3_ positions[2];
};
struct component_::button_ {

	__int32 id;
	__int32 sound_trigger;
	float3_ colours[2];
};
struct component_::switch_ {

	bool is_on;
	bool was_on;
	bool is_hold;
	__int32 id;
	__int32 i_archetype_target;
	__int32 i_entity_target;
};
struct component_::sound_trigger_ {

	__int32 source_id;
};
struct component_::power_ {

	bool is_on;
	float4_ q_null;
	float4_ q_add_colour;
	float4_ q_add_texture;
};
struct component_::attached_ {

	bool is_attachment;
	__int32 i_archetype;
	__int32 i_entity;
};
struct component_::mass_ {

	float value;
	float default_value;
};
struct component_::health_ {

	__int32 hp;
};
struct component_::weapon_ {

	struct ammo_ {

		enum {

			LOAD = 16,
			MAX = 99,
		};
	};

	struct id_ {

		enum {

			KILL,
			SHRINK,
			PETRIFY,
			PUSH,
			PULL,

			COUNT,
		};
	};
	struct projectile_data_ {

		float reload_time;
		float speed;
		float3_ scale;
		float3_ colour;
	};

	static projectile_data_ projectile_data[id_::COUNT];
	//static float reload_timers[id_::COUNT];

	__int32 projectile_id;
	__int32 i_projectile;
	__int32 i_begin;
	__int32 n_projectiles;
	__int32 ammo_count;
	float timer;
	bool is_fired;
};
struct component_::bounding_box_ {

	int3_ centre;
	int3_ extent;
};

struct component_::prop_ {

	__int32 id;
};

struct component_::mob_ {

	__int32 id;
};
struct component_::map_bvh_ {

	__int32 i_start;
	__int32 n_models;
	int3_ extent;
};
struct component_::model_ {

	__int32 id;
};


struct component_fetch_ {

	enum {

		MAX_COMPONENTS = 32,
		MAX_ARCHETYPES = 64,
	};

	__int32 n_components;
	__int32 n_excludes;
	__int32 n_archetypes;
	__int32 n_entities;
	unsigned __int32 component_masks[NUM_MASKS_RAW];
	unsigned __int32 exclude_masks[NUM_MASKS_RAW];
	__int32 component_ids[MAX_COMPONENTS];
	__int32 exclude_ids[MAX_COMPONENTS];
	__int32 i_archetypes[MAX_ARCHETYPES];
	__int8* table[MAX_COMPONENTS][MAX_ARCHETYPES];
};

struct archetype_ {

	__int32 n_entities;
	__int32 n_components;
	unsigned __int32 component_masks[NUM_MASKS_RAW];
	__int32* component_ids;
	unsigned __int32(*entity_component_masks)[NUM_MASKS_RAW];
	__int8** mem_ptrs;
};

struct archetype_data_ {

	__int32 n_archetypes;
	archetype_ archetypes[component_fetch_::MAX_ARCHETYPES];
};

//struct component_buffer_ {
//
//	enum {
//		NUM_BYTES = memory_::___1_MiB,
//	};
//
//	void* mem_ptr;
//	__int8 mem_chunk[NUM_BYTES];
//};

struct component_data_ {

	enum {

		NUM_BITS_INTEGER = NUM_BITS_INTEGER_RAW,
		NUM_MASKS = NUM_MASKS_RAW,
	};

	const __int32 size_byte[component_id_::COUNT] = {

		sizeof(component_::base_),							
		sizeof(component_::move_),
		sizeof(component_::animation_),
		sizeof(component_::behaviour_),
		sizeof(component_::projectile_),
		sizeof(component_::projectile_id_),
		sizeof(component_::collider_),
		sizeof(component_::small_model_id_),
		sizeof(component_::collidee_static_),
		sizeof(component_::collidee_moving_),
		sizeof(component_::collidee_rotated_),
		sizeof(component_::vertex_light_source_),
		sizeof(component_::lightmap_light_source_),
		sizeof(component_::patrol_point_),
		sizeof(component_::patrol_),
		sizeof(component_::trigger_),
		sizeof(component_::spawn_),
		sizeof(component_::model_space_),
		sizeof(component_::model_space_update_),
		sizeof(component_::colour_),
		sizeof(component_::colour_space_),
		sizeof(component_::texture_space_),
		sizeof(component_::draw_),
		sizeof(component_::item_),
		sizeof(component_::camera_),
		sizeof(component_::teleporter_),
		sizeof(component_::lava_),
		sizeof(component_::door_),
		sizeof(component_::cloud_),
		sizeof(component_::platform_),
		sizeof(component_::map_collide_),
		sizeof(component_::map_render_),
		sizeof(component_::particle_),
		sizeof(component_::animation_driver_),
		sizeof(component_::effect_),
		sizeof(component_::fly_),
		sizeof(component_::texture_blend_),
		sizeof(component_::trap_door_id_),
		sizeof(component_::plate_),
		sizeof(component_::button_),
		sizeof(component_::switch_),
		sizeof(component_::sound_trigger_),
		sizeof(component_::power_),
		sizeof(component_::attached_),
		sizeof(component_::mass_),
		sizeof(component_::health_),
		sizeof(component_::weapon_),
		sizeof(component_::bounding_box_),
		sizeof(component_::prop_),
		sizeof(component_::mob_),
		sizeof(component_::map_bvh_),
		sizeof(component_::model_),

	};

	archetype_data_ archetype_data;

};

struct behaviour_data_ {


	struct component_toggle_ {

		enum {
			MAX_COMPONENTS = 4,
		};

		__int32 n_components;
		__int32 component_ids[MAX_COMPONENTS];
	};

	__int32 animation_id;
	__int32 target_id;
	float max_speed;
	float move_acceleration;
	float time_duration;
	component_toggle_ component_toggle;
};

struct behaviour_manager_ {

	behaviour_data_ behaviour_data[component_::behaviour_::id_::COUNT];
};


//======================================================================

void Convert_Component_list_To_Mask(const __int32, const __int32[], unsigned __int32[component_data_::NUM_MASKS]);

bool Match_Entity_Mask(

	const __int32 i_entity,
	const unsigned __int32 component_masks[component_data_::NUM_MASKS],
	const archetype_& archetype
);

__int32 Return_Camera_Component(

	const archetype_data_&,
	component_::camera_**
);

void Populate_Fetch_Table(

	const archetype_data_&,
	component_fetch_&

);

__int8* Find_Archetype_Component(

	const __int32,
	const __int32,
	const archetype_data_&
);

void Clear_Component_Bit(

	const __int32,
	const __int32,
	const __int32,
	archetype_data_&
);

void Set_Component_Bit(

	const __int32,
	const __int32,
	const __int32,
	archetype_data_&
);

void Toggle_Component_Bit(

	const __int32,
	const __int32,
	const __int32,
	archetype_data_&
);

