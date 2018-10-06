#include "update.h"
#include "setup.h"
#include "render_front.h"
#include "command.h"
#include "model.h"

//======================================================================


/*
==================
==================
*/
void Render_Animated_Model(

	const __int32 i_thread,
	const __int32 i_draw_call,
	const model_manager_& model_manager,
	command_buffer_& command_buffer,
	display_& display

)
{
	matrix_ matrix_scale;
	matrix_ matrix_translate;
	memset(matrix_scale, 0, 4 * 4 * 4);
	memset(matrix_translate, 0, 4 * 4 * 4);

	matrix_translate[X].x = 1.0f;
	matrix_translate[Y].y = 1.0f;
	matrix_translate[Z].z = 1.0f;
	matrix_translate[W].w = 1.0f;

	matrix_scale[W].w = 1.0f;

	enum {
		MAX_VERTICES = 1 << 11,
	};

	float3_ vertices[MAX_VERTICES];


	draw_call_ &draw_call = command_buffer.draw_calls[i_draw_call];
	draw_call.i_thread = i_thread;

	for (__int32 i_model = 0; i_model < draw_call.n_models; i_model++) {

		const __int32 model_id = draw_call.model_id[i_model];
		const model_& model = model_manager.model[draw_call.model_id[i_model]];
		const __int32 n_vertices = model.n_vertices;
		const __int32 i_frame_0 = draw_call.animation_data[i_model].i_frames[0];
		const __int32 i_frame_1 = draw_call.animation_data[i_model].i_frames[1];
		const __m128 frame_interval = set_all(draw_call.animation_data[i_model].frame_interval);

		matrix_scale[X].x = draw_call.scale[i_model].x;
		matrix_scale[Y].y = draw_call.scale[i_model].y;
		matrix_scale[Z].z = draw_call.scale[i_model].z;

		matrix_translate[W].x = (float)(draw_call.position[i_model].x - command_buffer.position_camera.x) * r_fixed_scale_real;
		matrix_translate[W].y = (float)(draw_call.position[i_model].y - command_buffer.position_camera.y) * r_fixed_scale_real;
		matrix_translate[W].z = (float)(draw_call.position[i_model].z - command_buffer.position_camera.z) * r_fixed_scale_real;

		matrix_translate[W].x -= model.bounding_origin.x + draw_call.animation_data[i_model].origin.x;
		matrix_translate[W].y -= model.bounding_origin.y + draw_call.animation_data[i_model].origin.y;
		matrix_translate[W].z -= model.bounding_origin.z + draw_call.animation_data[i_model].origin.z;

		matrix_ temp_1;
		matrix_ temp_2;
		Matrix_X_Matrix(draw_call.m_rotate[i_model], matrix_scale, temp_1);
		Matrix_X_Matrix(matrix_translate, temp_1, temp_2);

		matrix_ matrix_out;
		Matrix_X_Matrix(command_buffer.m_clip_space, temp_2, matrix_out);

		for (__int32 i_vertex_4 = 0; i_vertex_4 < n_vertices; i_vertex_4 += 4) {

			__int32 n = min(n_vertices - i_vertex_4, 4);

			__m128 vertex_frame[2][4];
			for (__int32 i_vertex = 0; i_vertex < n; i_vertex++) {

				vertex_frame[0][i_vertex] = load_u(model.vertices_frame[i_frame_0][i_vertex_4 + i_vertex].f);
				vertex_frame[1][i_vertex] = load_u(model.vertices_frame[i_frame_1][i_vertex_4 + i_vertex].f);
			}
			Transpose(vertex_frame[0]);
			Transpose(vertex_frame[1]);

			__m128 new_vertex[4];
			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				new_vertex[i_axis] = vertex_frame[0][i_axis] + ((vertex_frame[1][i_axis] - vertex_frame[0][i_axis]) * frame_interval);
			}

			Transpose(new_vertex);

			for (__int32 i_vertex = 0; i_vertex < n; i_vertex++) {
				store_u(new_vertex[i_vertex], vertices[i_vertex_4 + i_vertex].f);
			}
		}

		__int32 i_models[1][2];
		i_models[0][0] = i_model;
		i_models[0][1] = model.n_triangles;

		Renderer_FRONT_END(

			i_draw_call,
			model.n_triangles,
			model.i_vertices,
			vertices,
			matrix_out,
			i_models,
			display.screen_bin[i_thread]
		);
	}
}

/*
==================
==================
*/
void Render_STATIC_SMALL(

	const __int32 i_thread,
	const __int32 i_draw_call,
	const model_manager_& model_manager,
	command_buffer_& render_buffer,
	display_& display
)
{

	enum {
		MAX_VERTICES = 256,
		MAX_INDICES = 1024,
		NUM_MODELS_PER_BLOCK = 16,
	};

	float3_ vertex_stream[MAX_VERTICES];

	draw_call_ &draw_call = render_buffer.draw_calls[i_draw_call];
	const model_& model_cube = model_manager.model[draw_call.model_id[0]];

	const __int32 n_indices = model_cube.n_triangles * 3;
	__int32 index_stream[MAX_INDICES];
	__int32 i_index_write = 0;
	__int32 base_index = 0;
	for (__int32 i_entity = 0; i_entity < NUM_MODELS_PER_BLOCK; i_entity++) {
		for (__int32 i_index = 0; i_index < n_indices; i_index++) {

			index_stream[i_index_write] = model_cube.i_vertices[i_index] + base_index;
			i_index_write++;
		}
		base_index += model_cube.n_vertices;
	}

	draw_call.i_thread = i_thread;
	__int32 i_triangle = 0;
	__int32 i_model_read = 0;

	bool is_skybox = i_draw_call == draw_call_::id_::SKYBOX;
	float modifier = is_skybox ? 0.0f : 1.0f;

	for (__int32 i_batch = 0; i_batch < draw_call.n_models; i_batch += NUM_MODELS_PER_BLOCK) {

		__int32 i_models[NUM_MODELS_PER_BLOCK][2];
		__int32 i_vertex_write = 0;
		__int32 n_triangles = 0;
		const __int32 n_models_batch = min(draw_call.n_models - i_batch, NUM_MODELS_PER_BLOCK);

		for (__int32 i_model = 0; i_model < n_models_batch; i_model++) {

			for (__int32 i_vertex = 0; i_vertex < model_cube.n_vertices; i_vertex++) {
				for (__int32 i_axis = X; i_axis < W; i_axis++) {

					float delta = float(draw_call.position[i_model_read].i[i_axis] - render_buffer.position_camera.i[i_axis]) * r_fixed_scale_real * modifier;
					vertex_stream[i_vertex_write].f[i_axis] = (model_cube.vertices_frame[0][i_vertex].f[i_axis] * draw_call.scale[i_model_read].f[i_axis]) + delta;
				}
				i_vertex_write++;
			}

			//i_models[i_model][0] = model_::id_::PLATFORM;
			i_models[i_model][0] = i_model_read;
			i_models[i_model][1] = model_cube.n_triangles;
			i_model_read++;
			n_triangles += model_cube.n_triangles;
		}

		Renderer_FRONT_END(

			i_draw_call,
			n_triangles,
			index_stream,
			vertex_stream,
			render_buffer.m_clip_space,
			i_models,
			display.screen_bin[i_thread]
		);

		i_triangle += n_triangles;
	}
}

/*
==================
==================
*/
void Render_Rotated_Model(

	const __int32 i_thread,
	const __int32 i_draw_call,
	const model_manager_& model_manager,
	command_buffer_& render_buffer,
	display_& display

)
{
	matrix_ matrix_scale;
	memset(matrix_scale, 0, 4 * 4 * 4);
	matrix_ matrix_translate;
	memset(matrix_translate, 0, 4 * 4 * 4);

	matrix_translate[X].x = 1.0f;
	matrix_translate[Y].y = 1.0f;
	matrix_translate[Z].z = 1.0f;
	matrix_translate[W].w = 1.0f;

	matrix_scale[W].w = 1.0f;


	draw_call_& draw_call = render_buffer.draw_calls[i_draw_call];
	draw_call.i_thread = i_thread;

	for (__int32 i_model = 0; i_model < draw_call.n_models; i_model++) {

		matrix_scale[X].x = draw_call.scale[i_model].x;
		matrix_scale[Y].y = draw_call.scale[i_model].y;
		matrix_scale[Z].z = draw_call.scale[i_model].z;

		matrix_translate[W].x = (float)((draw_call.position[i_model].x - render_buffer.position_camera.x) * r_fixed_scale_real);
		matrix_translate[W].y = (float)((draw_call.position[i_model].y - render_buffer.position_camera.y) * r_fixed_scale_real);
		matrix_translate[W].z = (float)((draw_call.position[i_model].z - render_buffer.position_camera.z) * r_fixed_scale_real);

		matrix_ temp_1;
		matrix_ temp_2;
		Matrix_X_Matrix(draw_call.m_rotate[i_model], matrix_scale, temp_1);
		Matrix_X_Matrix(matrix_translate, temp_1, temp_2);
		matrix_ matrix_out;
		Matrix_X_Matrix(render_buffer.m_clip_space, temp_2, matrix_out);

		const model_& model = model_manager.model[draw_call.model_id[i_model]];

		__int32 i_models[1][2];
		i_models[0][0] = i_model;
		i_models[0][1] = model.n_triangles;;

		Renderer_FRONT_END(

			i_draw_call,
			model.n_triangles,
			model.i_vertices,
			model.vertices_frame[0],
			matrix_out,
			i_models,
			display.screen_bin[i_thread]
		);
	}
}

/*
==================
==================
*/
void Render_Static_Model(

	const __int32 i_thread,
	const __int32 i_draw_call,
	const model_manager_& model_manager,
	command_buffer_& render_buffer,
	display_& display

)
{
	matrix_ matrix_translate;
	memset(matrix_translate, 0, 4 * 4 * 4);
	matrix_translate[X].x = 1.0f;
	matrix_translate[Y].y = 1.0f;
	matrix_translate[Z].z = 1.0f;
	matrix_translate[W].w = 1.0f;

	draw_call_& draw_call = render_buffer.draw_calls[i_draw_call];
	draw_call.i_thread = i_thread;

	for (__int32 i_model = 0; i_model < draw_call.n_models; i_model++) {

		matrix_translate[W].x = (float)((draw_call.position[i_model].x - render_buffer.position_camera.x) * r_fixed_scale_real);
		matrix_translate[W].y = (float)((draw_call.position[i_model].y - render_buffer.position_camera.y) * r_fixed_scale_real);
		matrix_translate[W].z = (float)((draw_call.position[i_model].z - render_buffer.position_camera.z) * r_fixed_scale_real);

		matrix_ matrix_out;
		Matrix_X_Matrix(render_buffer.m_clip_space, matrix_translate, matrix_out);

		const model_& model = model_manager.model[draw_call.model_id[i_model]];

		__int32 i_models[1][2];
		i_models[0][0] = i_model;
		i_models[0][1] = model.n_triangles;

		Renderer_FRONT_END(

			i_draw_call,
			model.n_triangles,
			model.i_vertices,
			model.vertices_frame[0],
			matrix_out,
			i_models,
			display.screen_bin[i_thread]
		);
	}
}

/*
==================
==================
*/
void Render_Draw_Call_PRECALL(void* parameter, __int32 i_thread) {

	parameters_::render_::process_draw_call_* parameters = (parameters_::render_::process_draw_call_*)parameter;

	const __int32 i_draw_call = parameters->i_draw_call;
	const __int32 i_read = parameters->command_buffer_handler->i_read;
	const model_manager_& model_manager = *parameters->model_manager;
	command_buffer_& command_buffer = parameters->command_buffer_handler->command_buffers[i_read];
	draw_call_& draw_call = command_buffer.draw_calls[i_draw_call];

	//bool is_detail = (i_draw_call != draw_call_::id_::MAP) && (i_draw_call != draw_call_::id_::SHAMBLER) && (i_draw_call != draw_call_::id_::PLAYER);
	//if (is_detail) {
	//	return;
	//}

	if (draw_call.n_models > 0) {

		draw_call.renderer_function(

			i_thread,
			i_draw_call,
			model_manager,
			command_buffer,
			*parameters->display
		);
	}
}

