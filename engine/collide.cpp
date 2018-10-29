#include "collide.h"
#include "component.h"
#include "vector_math.h"
#include "model.h"

struct collider_data_ {

	int3_ position;
	int3_ extent;
	int3_ direction;
	__int32 magnitude;

	bool is_player;
};

struct collidee_data_ {

	enum {

		MAX_POTENTIAL_COLLIDEES = 16,
	};

	__int32 n_collidees;
	__int32 i_start[grid_::type_::id_::COUNT];
	__int32 n_entries[grid_::type_::id_::COUNT];
	__int32 indices[MAX_POTENTIAL_COLLIDEES];
};

void Collision_Routine_STATIC(

	const collider_data_&,
	const collidee_data_&,
	collision_output_&,
	grid_&
);

void Collision_Routine_MOVING(

	const collider_data_&,
	const collidee_data_&,
	collision_output_&,
	grid_&
);

void Collision_Routine_ROTATED(

	const collider_data_&,
	const collidee_data_&,
	collision_output_&,
	grid_&
);

void(*Collision_Routine[])(

	const collider_data_&,
	const collidee_data_&,
	collision_output_&,
	grid_&

	) = {

		Collision_Routine_STATIC,
		Collision_Routine_MOVING,
		Collision_Routine_ROTATED,
};

void Collision_Response_DEFLECT(const collision_output_&, collider_data_&);
void Collision_Response_REFLECT(const collision_output_&, collider_data_&);
void Collision_Response_STOP(const collision_output_&, collider_data_&);
void Collision_Response_PASS(const collision_output_&, collider_data_&);
void Collision_Response_REPULSE(const collision_output_&, collider_data_&);

void(*Collision_Response[])(const collision_output_&, collider_data_&) = {

	Collision_Response_DEFLECT,
	Collision_Response_REFLECT,
	Collision_Response_STOP,
	Collision_Response_PASS,
	Collision_Response_REPULSE,
};


//======================================================================

static const __int32 epsilon_fixed = 1;
static const __int32 epsilon_sub = 1 * (fixed_scale / 2);


/*
==================
==================
*/
void Collision_Response_PASS(

	const collision_output_& collision_data,
	collider_data_& collider_data

) {

	// empty
}

/*
==================
==================
*/
void Collision_Response_REPULSE(

	const collision_output_& collision_data,
	collider_data_& collider_data

) {

	collider_data.position = collision_data.collision_point;
	collider_data.direction = collision_data.collidee.normal;

	collider_data.position.x += collision_data.collidee.normal.x;
	collider_data.position.y += collision_data.collidee.normal.y;
	collider_data.position.z += collision_data.collidee.normal.z;
}


/*
==================
==================
*/
void Collision_Response_REFLECT(

	const collision_output_& collision_data,
	collider_data_& collider_data

) {

	collider_data.position = collision_data.collision_point;
	const __int32 i_axis_collide = collision_data.collidee.i_axis;
	__int32 plane_clamp = (collision_data.collidee.total_extent.i[i_axis_collide] + epsilon_fixed) * collision_data.collidee.normal.i[i_axis_collide];
	collider_data.position.i[i_axis_collide] = collision_data.collidee.position.i[i_axis_collide] + plane_clamp;
	collider_data.direction.i[collision_data.collidee.i_axis] *= -1;
}

/*
==================
==================
*/
void Collision_Response_STOP(

	const collision_output_& collision_data,
	collider_data_& collider_data

) {
	collider_data.position = collision_data.collision_point;
	const __int32 i_axis_collide = collision_data.collidee.i_axis;
	__int32 plane_clamp = (collision_data.collidee.total_extent.i[i_axis_collide] + epsilon_fixed) * collision_data.collidee.normal.i[i_axis_collide];
	collider_data.position.i[i_axis_collide] = collision_data.collidee.position.i[i_axis_collide] + plane_clamp;
	collider_data.magnitude = 0;
	collider_data.direction.x = 0;
	collider_data.direction.y = 0;
	collider_data.direction.z = 0;
}


/*
==================
==================
*/
void Collision_Response_DEFLECT(

	const collision_output_& collision_data,
	collider_data_& collider_data

) {
	collider_data.position = collision_data.collision_point;
	const __int32 i_axis_collide = collision_data.collidee.i_axis;
	__int32 plane_clamp = (collision_data.collidee.total_extent.i[i_axis_collide] + epsilon_fixed) * collision_data.collidee.normal.i[i_axis_collide];
	collider_data.position.i[i_axis_collide] = collision_data.collidee.position.i[i_axis_collide] + plane_clamp;
	collider_data.direction.i[collision_data.collidee.i_axis] = 0;
}

/*
==================
==================
*/
void Collision_Routine_ROTATED(

	const collider_data_& collider_data,
	const collidee_data_& collidee_data,
	collision_output_& collision_data,
	grid_& grid
)
{

	const int3_ zero = { 0, 0, 0 };
	const __int32 i_start = collidee_data.i_start[grid_::type_::id_::ROTATED];
	const __int32 i_end = collidee_data.i_start[grid_::type_::id_::ROTATED] + collidee_data.n_entries[grid_::type_::id_::ROTATED];

	for (__int32 i_index = i_start; i_index < i_end; i_index++) {

		__int32 i_collidee = collidee_data.indices[i_index];

		int3_ positions_local[2];
		int3_ sum_extent;
		for (__int32 i_axis = X; i_axis < W; i_axis++) {
			__int64 direction = collider_data.direction.i[i_axis];
			__int64 magnitude = collider_data.magnitude;
			__int64 displacement = (direction * magnitude) / fixed_scale;
			positions_local[0].i[i_axis] = collider_data.position.i[i_axis] - grid.centre[i_collidee].i[i_axis];
			positions_local[1].i[i_axis] = positions_local[0].i[i_axis] + (__int32)displacement;
			sum_extent.i[i_axis] = grid.extent[i_collidee].i[i_axis] + collider_data.extent.i[i_axis];
		}

		unsigned __int32 out_mask = 0x0;
		__int32 sign_modifier[2] = { -1, 1 };
		for (__int32 i_normal = X; i_normal < W; i_normal++) {

			for (__int32 i_side = 0; i_side < 2; i_side++) {

				__int64 dot[2] = { 0,0 };
				for (__int32 i_axis = X; i_axis < W; i_axis++) {

					__int64 normal = grid.normals[i_collidee][i_normal].i[i_axis] * sign_modifier[i_side];
					__int64 extent = sum_extent.i[i_normal];
					__int64 plane_d = (normal * extent) / fixed_scale;
					__int64 vector_a = positions_local[0].i[i_axis] - plane_d;
					__int64 vector_b = positions_local[1].i[i_axis] - plane_d;
					dot[0] += vector_a * normal;
					dot[1] += vector_b * normal;
				}

				dot[0] /= fixed_scale;
				dot[1] /= fixed_scale;
				out_mask |= (dot[0] > 0) && (dot[1] > 0);
			}
		}

		bool is_not_culled = (out_mask == 0x0);

		if (is_not_culled) {

			if (collider_data.is_player) {
				printf_s(" not culled ");
			}

			const __int64 t_begin = 0;
			const __int64 t_end = collision_data.t_interval;
			const __int64 t_default = -1;
			__int64 t_hit = t_default;
			__int32 i_axis_out = X;
			__int32 plane_signs[2][3];

			for (__int32 i_normal = X; i_normal < W; i_normal++) {

				__int64 lead_denominator = 0;
				__int64 t_temp[2];
				for (__int32 i_side = 0; i_side < 2; i_side++) {

					__int64 numerator = 0;
					__int64 denominator = 0;
					for (__int32 i_axis = X; i_axis < W; i_axis++) {

						__int64 normal = grid.normals[i_collidee][i_normal].i[i_axis] * sign_modifier[i_side];
						__int64 extent = sum_extent.i[i_normal];
						__int64 plane_d = (normal * extent) / fixed_scale;
						__int64 vector_plane = plane_d - positions_local[0].i[i_axis];
						__int64 vector_line = positions_local[1].i[i_axis] - positions_local[0].i[i_axis];
						numerator += vector_plane * normal;
						denominator += vector_line * normal;
					}
					numerator /= fixed_scale;
					denominator /= fixed_scale;

					//lead_denominator = blend_int(denominator, lead_denominator, (__int64)(i_side == 0));
					lead_denominator = i_side == 0 ? denominator : lead_denominator;
					//denominator = blend_int((__int64)(1 * fixed_scale), denominator, (__int64)(denominator == 0));
					denominator = denominator == 0 ? (__int64)(1 * fixed_scale) : denominator;
					t_temp[i_side] = (numerator * fixed_scale) / denominator;
				}

				t_temp[0] = max(t_temp[0], INT_MIN);
				t_temp[1] = min(t_temp[1], INT_MAX);

				bool is_entering = lead_denominator < 0;
				bool is_zero = lead_denominator == 0;
				//__int32 index = blend_int(0, 1, is_entering);
				__int32 index = is_entering ? 0 : 1;
				__int64 t_enter = (__int32)t_temp[index];
				__int64 t_exit = (__int32)t_temp[index ^ 1];
				plane_signs[0][i_normal] = sign_modifier[index];
				plane_signs[1][i_normal] = sign_modifier[index ^ 1];

				//t_enter = blend_int((__int64)INT_MIN, t_enter, (__int64)is_zero);
				//t_exit = blend_int((__int64)INT_MAX, t_exit, (__int64)is_zero);
				t_enter = is_zero ? (__int64)INT_MIN : t_enter;
				t_exit = is_zero ? (__int64)INT_MAX : t_exit;

				bool is_not_valid = (t_exit < t_enter) || (t_enter > t_end) || (t_exit < t_begin);
				bool is_valid = (t_enter > t_hit) && (!is_not_valid);
				//t_hit = blend_int(t_enter, t_hit, (__int64)is_valid);
				t_hit = is_valid ? t_enter :  t_hit;
				//i_axis_out = blend_int(i_normal, i_axis_out, is_valid);
				i_axis_out = is_valid ? i_normal : i_axis_out;
			}

			if (t_hit > t_default) {

				collision_data.collidee.entity_type = grid.data[i_collidee].entity_type;
				collision_data.collidee.entity_id = grid.data[i_collidee].entity_id;
				collision_data.collidee.i_archetype = grid.data[i_collidee].i_archetype;
				collision_data.collidee.i_entity = grid.data[i_collidee].i_entity;
				collision_data.collidee.i_axis = i_axis_out;
				collision_data.collidee.normal.x = grid.normals[i_collidee][i_axis_out].x * plane_signs[0][i_axis_out];
				collision_data.collidee.normal.y = grid.normals[i_collidee][i_axis_out].y * plane_signs[0][i_axis_out];
				collision_data.collidee.normal.z = grid.normals[i_collidee][i_axis_out].z * plane_signs[0][i_axis_out];
				collision_data.collidee.position = grid.centre[i_collidee];
				collision_data.collidee.total_extent = sum_extent;
				collision_data.t_interval = (__int32)t_hit;
			}
		}
	}
}

/*
==================
==================
*/
void Collision_Routine_MOVING(

	const collider_data_& collider_data,
	const collidee_data_& collidee_data,
	collision_output_& collision_data,
	grid_& grid
)
{
	const int3_ zero = { 0, 0, 0 };

	const __int32 i_start = collidee_data.i_start[grid_::type_::id_::MOVING];
	const __int32 i_end = collidee_data.i_start[grid_::type_::id_::MOVING] + collidee_data.n_entries[grid_::type_::id_::MOVING];

	for (__int32 i_index = i_start; i_index < i_end; i_index++) {

		__int32 i_collidee = collidee_data.indices[i_index];

		int3_ position_local;
		int3_ extent;
		for (__int32 i_axis = X; i_axis < W; i_axis++) {

			position_local.i[i_axis] = collider_data.position.i[i_axis] - grid.centre[i_collidee].i[i_axis];
			extent.i[i_axis] = collider_data.extent.i[i_axis] + grid.extent[i_collidee].i[i_axis];
		}

		unsigned __int32 out_mask = 0x0;
		for (__int32 i_axis = X; i_axis < W; i_axis++) {

			__int64 delta_0 = abs(position_local.i[i_axis]);
			__int64 direction = collider_data.direction.i[i_axis];
			__int64 magnitude = collider_data.magnitude;
			__int64 displacement = (direction * magnitude) / fixed_scale;
			__int64 delta_1 = abs(position_local.i[i_axis] + (__int32)displacement);
			out_mask |= (delta_0 > extent.i[i_axis]) && (delta_1 > extent.i[i_axis]);
		}

		bool is_not_culled = (out_mask == 0x0);

		if (is_not_culled) {

			const __int64 t_begin = 0;
			const __int64 t_end = collision_data.t_interval;
			const __int64 t_default = INT_MIN;
			__int64 t_hit = t_default;
			__int32 i_axis_out = X;

			for (__int32 i_axis = X; i_axis < W; i_axis++) {

				bool is_entering = collider_data.direction.i[i_axis] < 0;
				bool is_zero = collider_data.direction.i[i_axis] == 0;

				__int64 bounds[2];
				//bounds[0] = blend_int(extent.i[i_axis], -extent.i[i_axis], is_entering);
				bounds[0] = is_entering ? extent.i[i_axis] : -extent.i[i_axis];
				//bounds[1] = blend_int(-extent.i[i_axis], extent.i[i_axis], is_entering);
				bounds[1] = is_entering ? -extent.i[i_axis] : extent.i[i_axis];

				//__int64 denominator = blend_int(1 * fixed_scale, collider_data.direction.i[i_axis], is_zero);
				__int64 denominator = is_zero? 1 * fixed_scale : collider_data.direction.i[i_axis];

				__int64 t_enter = ((bounds[0] - position_local.i[i_axis]) * fixed_scale) / denominator;
				__int64 t_exit = ((bounds[1] - position_local.i[i_axis]) * fixed_scale) / denominator;

				t_enter = max(t_enter, INT_MIN);
				t_exit = min(t_exit, INT_MAX);

				//t_enter = blend_int((__int64)INT_MIN, t_enter, (__int64)is_zero);
				t_enter = is_zero ? (__int64)INT_MIN : t_enter;
				//t_exit = blend_int((__int64)INT_MAX, t_exit, (__int64)is_zero);
				t_exit = is_zero? (__int64)INT_MAX : t_exit;

				bool is_not_valid = (t_exit < t_enter) || (t_enter > t_end) || (t_exit < t_begin);
				bool is_valid = (t_enter > t_hit) && (!is_not_valid);
				//t_hit = blend_int(t_enter, t_hit, (__int64)is_valid);
				t_hit = is_valid ? t_enter : t_hit;
				//i_axis_out = blend_int(i_axis, i_axis_out, is_valid);
				i_axis_out = is_valid? i_axis : i_axis_out;
			}

			if (t_hit > t_default) {

				collision_data.collidee.entity_type = grid.data[i_collidee].entity_type;
				collision_data.collidee.entity_id = grid.data[i_collidee].entity_id;
				collision_data.collidee.i_archetype = grid.data[i_collidee].i_archetype;
				collision_data.collidee.i_entity = grid.data[i_collidee].i_entity;
				collision_data.collidee.i_axis = i_axis_out;
				collision_data.collidee.normal = zero;
				collision_data.collidee.normal.i[i_axis_out] = blend_int(1, -1, collider_data.direction.i[i_axis_out] < 0);
				collision_data.collidee.position = grid.centre[i_collidee];
				collision_data.collidee.total_extent = extent;
				collision_data.t_interval = (__int32)t_hit;
			}
		}
	}
}

/*
==================
==================
*/
void Collision_Routine_STATIC(

	const collider_data_& collider_data,
	const collidee_data_& collidee_data,
	collision_output_& collision_data,
	grid_& grid
)
{
	
	const int3_ zero = { 0, 0, 0 };

	const __int32 i_start = collidee_data.i_start[grid_::type_::id_::STATIC];
	const __int32 i_end = collidee_data.i_start[grid_::type_::id_::STATIC] + collidee_data.n_entries[grid_::type_::id_::STATIC];

	for (__int32 i_index = i_start; i_index < i_end; i_index++) {

		__int32 i_collidee = collidee_data.indices[i_index];

		int3_ position_local;
		int3_ extent;
		for (__int32 i_axis = X; i_axis < W; i_axis++) {

			position_local.i[i_axis] = collider_data.position.i[i_axis] - grid.centre[i_collidee].i[i_axis];
			extent.i[i_axis] = collider_data.extent.i[i_axis] + grid.extent[i_collidee].i[i_axis];
		}

		unsigned __int32 out_mask = 0x0;
		for (__int32 i_axis = X; i_axis < W; i_axis++) {

			__int64 delta_0 = abs(position_local.i[i_axis]);
			__int64 direction = collider_data.direction.i[i_axis];
			__int64 magnitude = collider_data.magnitude;
			__int64 displacement = (direction * magnitude) / fixed_scale;
			__int64 delta_1 = abs(position_local.i[i_axis] + (__int32)displacement);
			out_mask |= (delta_0 > extent.i[i_axis]) && (delta_1 > extent.i[i_axis]);
		}

		bool is_not_culled = (out_mask == 0x0);

		if (is_not_culled) {

			const __int64 t_begin = 0;
			const __int64 t_end = collision_data.t_interval;
			const __int64 t_default = -1;
			__int64 t_hit = t_default;
			__int32 i_axis_out = X;

			for (__int32 i_axis = X; i_axis < W; i_axis++) {

				bool is_entering = collider_data.direction.i[i_axis] < 0;
				bool is_zero = collider_data.direction.i[i_axis] == 0;

				__int64 bounds[2];
				//bounds[0] = blend_int(extent.i[i_axis], -extent.i[i_axis], is_entering);
				bounds[0] = is_entering ? extent.i[i_axis] : -extent.i[i_axis];
				//bounds[1] = blend_int(-extent.i[i_axis], extent.i[i_axis], is_entering);
				bounds[1] = is_entering? -extent.i[i_axis] : extent.i[i_axis];

				//__int64 denominator = blend_int(1 * fixed_scale, collider_data.direction.i[i_axis], is_zero);
				__int64 denominator = is_zero? 1 * fixed_scale : collider_data.direction.i[i_axis];

				__int64 t_enter = ((bounds[0] - position_local.i[i_axis]) * fixed_scale) / denominator;
				__int64 t_exit = ((bounds[1] - position_local.i[i_axis]) * fixed_scale) / denominator;

				t_enter = max(t_enter, INT_MIN);
				t_exit = min(t_exit, INT_MAX);

				//t_enter = blend_int((__int64)INT_MIN, t_enter, (__int64)is_zero);
				t_enter = is_zero ? (__int64)INT_MIN : t_enter;
				//t_exit = blend_int((__int64)INT_MAX, t_exit, (__int64)is_zero);
				t_exit = is_zero ? (__int64)INT_MAX : t_exit;

				bool is_not_valid = (t_exit < t_enter) || (t_enter > t_end) || (t_exit < t_begin);
				bool is_valid = (t_enter > t_hit) && (!is_not_valid);
				//t_hit = blend_int(t_enter, t_hit, (__int64)is_valid);
				t_hit = is_valid ? t_enter : t_hit;;
				//i_axis_out = blend_int(i_axis, i_axis_out, is_valid);
				i_axis_out = is_valid ? i_axis : i_axis_out;
			}

			if (t_hit > t_default) {

				collision_data.collidee.entity_type = grid.data[i_collidee].entity_type;
				collision_data.collidee.entity_id = grid.data[i_collidee].entity_id;
				collision_data.collidee.i_archetype = grid.data[i_collidee].i_archetype;
				collision_data.collidee.i_entity = grid.data[i_collidee].i_entity;
				collision_data.collidee.i_axis = i_axis_out;
				collision_data.collidee.normal = zero;
				collision_data.collidee.normal.i[i_axis_out] = blend_int(1, -1, collider_data.direction.i[i_axis_out] < 0);
				collision_data.collidee.position = grid.centre[i_collidee];
				collision_data.collidee.total_extent = extent;
				collision_data.t_interval = (__int32)t_hit;
			}
		}
	}
}


/*
==================
==================
*/
void Collision_Routine_PARSE_BVH(

	const collider_data_& collider_data,
	const bool allow_table[colliding_type_::COUNT],
	collidee_data_& collidee_data,
	grid_& grid
)
{

	//const __int32 epsilon = 10 * fixed_scale;
	const __int32 epsilon = 0;

	__m128i collider_position[3];
	__m128i collider_extent[3];
	for (__int32 i_axis = X; i_axis < W; i_axis++) {
		collider_position[i_axis] = set_all(collider_data.position.i[i_axis]);
		collider_extent[i_axis] = set_all(collider_data.extent.i[i_axis]);
	}
	__m128i collider_magnitude = set_all(collider_data.magnitude + epsilon);


	__int32 n_valid_nodes = 0;
	__int32 i_valid_nodes[grid_::NUM_NODES];
	__m128i indices = set(0, 1, 2, 3);
	__m128i indices_inc = set_all(4);

	for (__int32 i_node_4 = 0; i_node_4 < grid_::NUM_NODES; i_node_4 += 4) {

		__int32 n = min(grid_::NUM_NODES - i_node_4, 4);

		__m128i node_position[4];
		__m128i node_extent[4];
		unsigned __int32 valid_mask = 0x0;
		for (__int32 i_node = 0; i_node < n; i_node++) {

			valid_mask |= 0x1 << i_node;
			node_position[i_node] = load_u(grid.nodes[i_node_4 + i_node].collide.centre.i);
			node_extent[i_node] = load_u(grid.nodes[i_node_4 + i_node].collide.extent.i);
		}
		Transpose(node_position);
		Transpose(node_extent);

		__m128i result = set_all(-1);
		for (__int32 i_axis = X; i_axis < W; i_axis++) {

			__m128i delta = abs(collider_position[i_axis] - node_position[i_axis]);
			__m128i extent = collider_extent[i_axis] + node_extent[i_axis] + collider_magnitude;
			result &= delta < extent;
		}

		unsigned __int32 pack_mask = store_mask(result) & valid_mask;
		__m128i packed_indices = Pack_Vector4(indices, pack_mask);
		store_u(packed_indices, i_valid_nodes + n_valid_nodes);
		n_valid_nodes += __popcnt(pack_mask);
		indices += indices_inc;
	}

	__int32 temp_index[collidee_data_::MAX_POTENTIAL_COLLIDEES];

	for (__int32 i_valid_node = 0; i_valid_node < n_valid_nodes; i_valid_node++) {

		const __int32 i_node = i_valid_nodes[i_valid_node];
		const __int32 n_entities = grid.nodes[i_node].collide.n_entries;

		for (__int32 i_entity_4 = 0; i_entity_4 < n_entities; i_entity_4 += 4) {

			__int32 n = min(n_entities - i_entity_4, 4);

			__m128i collidee_position[4];
			__m128i collidee_extent[4];
			unsigned __int32 valid_mask = 0x0;
			for (__int32 i_entity = 0; i_entity < n; i_entity++) {

				__int32 index = grid.nodes[i_node].indices[i_entity_4 + i_entity];
				valid_mask |= allow_table[grid.data[index].entity_type] << i_entity;
				collidee_position[i_entity] = load_u(grid.centre[index].i);
				collidee_extent[i_entity] = load_u(grid.bounding_box[index].i);
			}
			Transpose(collidee_position);
			Transpose(collidee_extent);

			__m128i result = set_all(-1);
			for (__int32 i_axis = X; i_axis < W; i_axis++) {

				__m128i delta = abs(collider_position[i_axis] - collidee_position[i_axis]);
				__m128i extent = collider_extent[i_axis] + collidee_extent[i_axis] + collider_magnitude;
				result &= delta < extent;
			}

			unsigned __int32 pack_mask = store_mask(result) & valid_mask;
			__m128i indices = load_u(grid.nodes[i_node].indices + i_entity_4);
			indices = Pack_Vector4(indices, pack_mask);
			store_u(indices, temp_index + collidee_data.n_collidees);
			collidee_data.n_collidees += __popcnt(pack_mask);
		}
	}

	__int32 n_temp = 0;

	for (__int32 i_type = 0; i_type < grid_::type_::id_::COUNT; i_type++) {

		collidee_data.i_start[i_type] = n_temp;
		collidee_data.n_entries[i_type] = 0;

		for (__int32 i_index = 0; i_index < collidee_data.n_collidees; i_index++) {
			const __int32 i_collidee = temp_index[i_index];
			collidee_data.indices[n_temp] = i_collidee;
			__int32 increment = grid.data[i_collidee].type_id == i_type;
			n_temp += increment;
			collidee_data.n_entries[i_type] += increment;
		}
	}
}

/*
==================
==================
*/
void systems_::collide_::collision_detection(

	void* parameters, __int32 i_thread
)
{

	parameters_::collide_::collision_detection_* func_parameters = (parameters_::collide_::collision_detection_*)parameters;
	const __int32 i_begin = func_parameters->i_begin;
	const __int32 n_entities  = func_parameters->n_colliders_per_job; 
	collision_manager_& collision_manager = *func_parameters->collision_manager; 
	archetype_data_& archetype_data = *func_parameters->archetype_data;
	grid_& grid = *func_parameters->grid;

	const __int32 step = 10 * fixed_scale;
	const int3_ zero = { 0, 0, 0 };
	const __int32 n_routines = sizeof(Collision_Routine) / sizeof(Collision_Routine[0]);
	thread_collision_& thread_collision = collision_manager.thread_collisions[i_thread];

	component_fetch_ component_fetch;
	component_fetch.n_components = 3;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::BASE;
	component_fetch.component_ids[1] = component_id_::MOVE;
	component_fetch.component_ids[2] = component_id_::COLLIDER;

	Populate_Fetch_Table(archetype_data, component_fetch);

	__int32 i_archetype_index = 0;
	__int32 i_entity = 0;
	__int32 entity_count = 0;

	for (__int32 i_element = 0; i_element < component_fetch.n_archetypes; i_element++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_element];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		i_entity = i_begin - entity_count;

		if (i_entity < archetype.n_entities) {
			break;
		}

		entity_count += archetype.n_entities;
		i_archetype_index++;
	}

	for (__int32 i_entity_index = 0; i_entity_index < n_entities; i_entity_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];

		component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
		component_::move_* move = (component_::move_*)component_fetch.table[1][i_archetype_index];
		component_::collider_* collider = (component_::collider_*)component_fetch.table[2][i_archetype_index];

		if (Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) {

			collider_data_ collider_data;
			{
				float3_ direction;
				float distance = Vector_Normalise_Magnitude(move[i_entity].displacement, direction);

				__int64 x = (__int64)(direction.x * fixed_scale_real);
				__int64 y = (__int64)(direction.y * fixed_scale_real);
				__int64 z = (__int64)(direction.z * fixed_scale_real);
				__int64 mag = (__int64)(distance * fixed_scale_real);

				x = max((min(x, INT_MAX)), INT_MIN);
				y = max((min(y, INT_MAX)), INT_MIN);
				z = max((min(z, INT_MAX)), INT_MIN);
				mag = max((min(mag, INT_MAX)), INT_MIN);

				collider_data.direction.x = (__int32)x;
				collider_data.direction.y = (__int32)y;
				collider_data.direction.z = (__int32)z;
				collider_data.magnitude = (__int32)mag;
				collider_data.position = base[i_entity].position_fixed;
				collider_data.extent = collider[i_entity].extent;
			}

			collider_data.is_player = (collider[i_entity].entity_type == colliding_type_::PLAYER) && (collider[i_entity].entity_id == 0);

			collidee_data_ collidee_data;
			collidee_data.n_collidees = 0;

			Collision_Routine_PARSE_BVH(

				collider_data,
				collision_manager.allow_table[collider[i_entity].entity_type],
				collidee_data,
				grid
			);

			const __int32 n_iterations = 6;
			for (__int32 i_iteration = 0; i_iteration < n_iterations; i_iteration++) {

				collision_output_& collision_data = thread_collision.collision_output[thread_collision.n_colliders];
				collision_data.collider.entity_type = collider[i_entity].entity_type;
				collision_data.collider.entity_id = collider[i_entity].entity_id;
				collision_data.collider.i_archetype = i_archetype;
				collision_data.collider.i_entity = i_entity;
				collision_data.collider.velocity = move[i_entity].velocity;
				collision_data.t_interval = collider_data.magnitude;

				for (__int32 i_routine = 0; i_routine < n_routines; i_routine++) {
					Collision_Routine[i_routine](

						collider_data,
						collidee_data,
						collision_data,
						grid
						); 
				}

				bool is_collide = collision_data.t_interval < collider_data.magnitude;

				if (!is_collide) {
					break;
				}

				bool can_step = collision_manager.can_step[collision_data.collider.entity_type][collision_data.collidee.entity_type];

				if (can_step) {

					collider_data.position.y += step;
					collision_output_& collision_data_next = thread_collision.collision_output[thread_collision.n_colliders + 1];
					collision_data_next.t_interval = collider_data.magnitude;

					for (__int32 i_routine = 0; i_routine < n_routines; i_routine++) {
						Collision_Routine[i_routine](

							collider_data,
							collidee_data,
							collision_data_next,
							grid
							);
					}

					collider_data.position.y -= step;

					bool is_stepped = (collision_data.collidee.i_axis != Y) && (collision_data_next.t_interval == collider_data.magnitude);

					if (is_stepped) {

						collision_data.collidee.i_axis = Y;
						collision_data.collidee.normal = zero;
						collision_data.collidee.normal.i[Y] = 1;
						collision_data.t_interval = 0;
					}
				}

				{
					for (__int32 i_axis = X; i_axis < W; i_axis++) {
						__int64 a = collider_data.direction.i[i_axis];
						__int64 b = collision_data.t_interval;
						__int64 displacement = (a * b) / fixed_scale;
						collision_data.collision_point.i[i_axis] = collider_data.position.i[i_axis] + (__int32)displacement;
					}
				}

				const __int32 i_response = collision_manager.i_response_type[collider[i_entity].entity_type][collision_data.collidee.entity_type];
				Collision_Response[i_response](collision_data, collider_data);

				collider_data.magnitude -= collision_data.t_interval;
				thread_collision.n_colliders += is_collide;

				if (collider_data.magnitude <= 0) {
					break;
				}
			}

			{
				float3_ direction;
				float distance;
				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					base[i_entity].position_fixed.i[i_axis] = collider_data.position.i[i_axis];
					direction.f[i_axis] = (float)collider_data.direction.i[i_axis] * r_fixed_scale_real;
				}
				distance = (float)collider_data.magnitude * r_fixed_scale_real;

				move[i_entity].displacement = direction * distance;
				float speed = Vector_Magnitude(move[i_entity].velocity);
				move[i_entity].velocity = direction * speed;

				collider[i_entity].is_ground_contact = false;
			}
		}

		i_entity++;
		bool is_next_archetype = i_entity == archetype.n_entities;
		i_archetype_index += is_next_archetype;
		i_entity = blend_int(0, i_entity, is_next_archetype);
	}
}


/*
==================
==================
*/
void systems_::collide_::collision_BVH(

	void* parameters, __int32 i_thread
)
{

	parameters_::collide_::collision_BVH_* func_parameters = (parameters_::collide_::collision_BVH_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;
	grid_& grid = *func_parameters->grid;

	enum {
		MAX_ELEMENTS = 128,
		NUM_BITS_SHIFT = 6,
		NUM_BITS = 1 << NUM_BITS_SHIFT,
		NUM_BLOCKS = MAX_ELEMENTS / NUM_BITS,
	};
	const unsigned __int64 mod_mask = NUM_BITS - 1;
	const unsigned __int64 one = 0x1;

	grid.n_entries = grid.n_entries_map;

	{
		component_fetch_ component_fetch;
		component_fetch.n_components = 2;
		component_fetch.n_excludes = 0;
		component_fetch.component_ids[0] = component_id_::BASE;
		component_fetch.component_ids[1] = component_id_::COLLIDEE_STATIC;

		Populate_Fetch_Table(archetype_data, component_fetch);

		unsigned __int64 mask[NUM_BLOCKS] = { 0x0 };

		for (__int32 i_node = 0; i_node < grid_::NUM_NODES; i_node++) {

			grid.nodes[i_node].collide.n_entries = grid.nodes[i_node].map.n_entries;
			grid.nodes[i_node].collide.centre = grid.nodes[i_node].map.centre;
			grid.nodes[i_node].collide.extent = grid.nodes[i_node].map.extent;

			__int32 i_element = 0;

			for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

				const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
				const archetype_& archetype = archetype_data.archetypes[i_archetype];
				const __int32 n_entities = archetype.n_entities;

				component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
				component_::collidee_static_* collidee = (component_::collidee_static_*)component_fetch.table[1][i_archetype_index];

				for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

					if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

					__int32 increment = 0x1;
					for (__int32 i_axis = X; i_axis < W; i_axis++) {
						__int32 delta = abs(grid.nodes[i_node].collide.centre.i[i_axis] - base[i_entity].position_fixed.i[i_axis]);
						increment &= delta < grid.nodes[i_node].collide.extent.i[i_axis];
					}

					grid.data[grid.n_entries].type_id = grid_::type_::id_::STATIC;
					grid.data[grid.n_entries].i_entity = i_entity;
					grid.data[grid.n_entries].i_archetype = i_archetype;
					grid.data[grid.n_entries].entity_id = collidee[i_entity].entity_id;
					grid.data[grid.n_entries].entity_type = collidee[i_entity].entity_type;
					grid.centre[grid.n_entries] = base[i_entity].position_fixed;
					grid.extent[grid.n_entries] = collidee[i_entity].extent;
					grid.bounding_box[grid.n_entries] = collidee[i_entity].extent;

					__int32 i_block = i_element >> NUM_BITS_SHIFT;
					__int32 i_mask = i_element & mod_mask;
					increment &= (mask[i_block] >> i_mask) ^ one;
					mask[i_block] |= increment << i_mask;
					i_element++;

					grid.nodes[i_node].indices[grid.nodes[i_node].collide.n_entries] = grid.n_entries;
					grid.nodes[i_node].collide.n_entries += increment;

					grid.n_entries += increment;
				}
			}
		}
	}
	{
		component_fetch_ component_fetch;
		component_fetch.n_components = 2;
		component_fetch.n_excludes = 0;
		component_fetch.component_ids[0] = component_id_::BASE;
		component_fetch.component_ids[1] = component_id_::COLLIDEE_MOVING;

		Populate_Fetch_Table(archetype_data, component_fetch);

		unsigned __int64 mask[NUM_BLOCKS] = { 0x0 };

		for (__int32 i_node = 0; i_node < grid_::NUM_NODES; i_node++) {

			__int32 i_element = 0;

			for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

				const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
				const archetype_& archetype = archetype_data.archetypes[i_archetype];
				const __int32 n_entities = archetype.n_entities;

				component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
				component_::collidee_moving_* collidee = (component_::collidee_moving_*)component_fetch.table[1][i_archetype_index];

				for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

					if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

					__int32 increment = 0x1;
					for (__int32 i_axis = X; i_axis < W; i_axis++) {
						__int32 delta = abs(grid.nodes[i_node].collide.centre.i[i_axis] - base[i_entity].position_fixed.i[i_axis]);
						increment &= delta < grid.nodes[i_node].collide.extent.i[i_axis];
					}

					grid.data[grid.n_entries].type_id = grid_::type_::id_::MOVING;
					grid.data[grid.n_entries].i_entity = i_entity;
					grid.data[grid.n_entries].i_archetype = i_archetype;
					grid.data[grid.n_entries].entity_id = collidee[i_entity].entity_id;
					grid.data[grid.n_entries].entity_type = collidee[i_entity].entity_type;
					grid.centre[grid.n_entries] = base[i_entity].position_fixed;
					grid.extent[grid.n_entries] = collidee[i_entity].extent;
					grid.bounding_box[grid.n_entries].x = collidee[i_entity].extent.x; // + (40 * fixed_scale);
					grid.bounding_box[grid.n_entries].y = collidee[i_entity].extent.y; // + (40 * fixed_scale);
					grid.bounding_box[grid.n_entries].z = collidee[i_entity].extent.z; // + (40 * fixed_scale);

					__int32 i_block = i_element >> NUM_BITS_SHIFT;
					__int32 i_mask = i_element & mod_mask;
					increment &= (mask[i_block] >> i_mask) ^ one;
					mask[i_block] |= increment << i_mask;
					i_element++;

					grid.nodes[i_node].indices[grid.nodes[i_node].collide.n_entries] = grid.n_entries;
					grid.nodes[i_node].collide.n_entries += increment;

					grid.n_entries += increment;
				}
			}
		}
	}
	{
		component_fetch_ component_fetch;
		component_fetch.n_components = 2;
		component_fetch.n_excludes = 0;
		component_fetch.component_ids[0] = component_id_::BASE;
		component_fetch.component_ids[1] = component_id_::COLLIDEE_ROTATED;

		Populate_Fetch_Table(archetype_data, component_fetch);

		unsigned __int64 mask[NUM_BLOCKS] = { 0x0 };

		for (__int32 i_node = 0; i_node < grid_::NUM_NODES; i_node++) {

			__int32 i_element = 0;

			for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

				const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
				const archetype_& archetype = archetype_data.archetypes[i_archetype];
				const __int32 n_entities = archetype.n_entities;

				component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
				component_::collidee_rotated_* collidee = (component_::collidee_rotated_*)component_fetch.table[1][i_archetype_index];

				for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

					if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

					__int32 increment = 0x1;
					for (__int32 i_axis = X; i_axis < W; i_axis++) {
						__int32 delta = abs(grid.nodes[i_node].collide.centre.i[i_axis] - base[i_entity].position_fixed.i[i_axis]);
						increment &= delta < grid.nodes[i_node].collide.extent.i[i_axis];
					}

					grid.data[grid.n_entries].type_id = grid_::type_::id_::ROTATED;
					grid.data[grid.n_entries].i_entity = i_entity;
					grid.data[grid.n_entries].i_archetype = i_archetype;
					grid.data[grid.n_entries].entity_id = collidee[i_entity].entity_id;
					grid.data[grid.n_entries].entity_type = collidee[i_entity].entity_type;
					grid.centre[grid.n_entries] = base[i_entity].position_fixed;
					grid.extent[grid.n_entries] = collidee[i_entity].extent;
					grid.bounding_box[grid.n_entries].x = collidee[i_entity].extent.x + (50 * fixed_scale);		// TOTAL HACK!!!
					grid.bounding_box[grid.n_entries].y = collidee[i_entity].extent.y + (50 * fixed_scale);
					grid.bounding_box[grid.n_entries].z = collidee[i_entity].extent.z + (50 * fixed_scale);

					grid.normals[grid.n_entries][X] = collidee[i_entity].normals[X];
					grid.normals[grid.n_entries][Y] = collidee[i_entity].normals[Y];
					grid.normals[grid.n_entries][Z] = collidee[i_entity].normals[Z];

					__int32 i_block = i_element >> NUM_BITS_SHIFT;
					__int32 i_mask = i_element & mod_mask;
					increment &= (mask[i_block] >> i_mask) ^ one;
					mask[i_block] |= increment << i_mask;
					i_element++;

					grid.nodes[i_node].indices[grid.nodes[i_node].collide.n_entries] = grid.n_entries;
					grid.nodes[i_node].collide.n_entries += increment;

					grid.n_entries += increment;
				}
			}
		}
	}

	for (__int32 i_node = 0; i_node < grid_::NUM_NODES; i_node++) {

		int3_ bound_max;
		int3_ bound_min;

		for (__int32 i_axis = X; i_axis < W; i_axis++) {

			bound_max.i[i_axis] = grid.nodes[i_node].map.centre.i[i_axis] + grid.nodes[i_node].map.extent.i[i_axis];
			bound_min.i[i_axis] = grid.nodes[i_node].map.centre.i[i_axis] - grid.nodes[i_node].map.extent.i[i_axis];
		}

		__int32 i_start = grid.nodes[i_node].map.n_entries;
		__int32 i_end = grid.nodes[i_node].collide.n_entries;
		for (__int32 i_entry = i_start; i_entry < i_end; i_entry++) {

			__int32 index = grid.nodes[i_node].indices[i_entry];

			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				__int32 centre = grid.centre[index].i[i_axis];
				//__int32 extent = grid.extent[index].i[i_axis];
				__int32 extent = grid.bounding_box[index].i[i_axis];
				bound_max.i[i_axis] = max(centre + extent, bound_max.i[i_axis]);
				bound_min.i[i_axis] = min(centre - extent, bound_min.i[i_axis]);
			}
		}

		for (__int32 i_axis = X; i_axis < W; i_axis++) {
			__int64 denom = 2 * fixed_scale;
			__int64 sum = bound_max.i[i_axis] + bound_min.i[i_axis];
			__int64 diff = bound_max.i[i_axis] - bound_min.i[i_axis];
			grid.nodes[i_node].collide.centre.i[i_axis] = __int32((sum * fixed_scale) / denom);
			grid.nodes[i_node].collide.extent.i[i_axis] = __int32((diff * fixed_scale) / denom);
			//grid.nodes[i_node].collide.extent.i[i_axis] += 10 * fixed_scale;
		}
	}
}
