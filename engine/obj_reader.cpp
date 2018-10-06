
#include "setup.h"

/*
==================
==================
*/
void load_obj_file(

	const __int32 token_id,
	const char *file_name,
	model_token_& model_token
) {

	enum {

		NUM_FACES = 6,
		NUM_VERTICES = 4,

		MAX_CHARS = 256,
		MAX_CHUNK_ENTRIES = 2048,
	};

	// NOT DRY LOL
	const float3_ world_scale = { 10.0f, 10.0f, 10.0f };

	FILE *file;
	fopen_s(&file, file_name, "r");		// txt mode

	if (!file) {
		printf_s("FAILED to open %s", file_name);
		exit(0);
	}
	printf("opened %s \n", file_name);

	char line[MAX_CHARS];

	// -------------------------------------------------------------------------------

	float3_ vertex_chunk[MAX_CHUNK_ENTRIES];
	{
		fseek(file, 0, SEEK_SET);

		const char vertex_string[] = "v ";
		const __int32 length = (__int32)strlen(vertex_string);
		__int32 n_vertices = 0;

		while (fgets(line, sizeof(line), file)) {

			if (strncmp(line, vertex_string, length) == 0) {

				char* data_ptr = line + length;
				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					vertex_chunk[n_vertices].f[i_axis] = (float)strtod(data_ptr, &data_ptr);
				}
				n_vertices++;

				assert(n_vertices < MAX_CHUNK_ENTRIES);
			}
		}
	}
	// -------------------------------------------------------------------------------

	float3_ normal_chunk[MAX_CHUNK_ENTRIES];
	{
		fseek(file, 0, SEEK_SET);

		const char normal_string[] = "vn ";
		const __int32 length = (__int32)strlen(normal_string);
		__int32 n_normals = 0;

		while (fgets(line, sizeof(line), file)) {

			if (strncmp(line, normal_string, length) == 0) {

				char* data_ptr = line + length;
				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					normal_chunk[n_normals].f[i_axis] = (float)strtod(data_ptr, &data_ptr);
				}
				n_normals++;
			}

			assert(n_normals < MAX_CHUNK_ENTRIES);
		}
	}
	// -------------------------------------------------------------------------------

	{
		const char face_string[] = "f ";
		const __int32 length = (__int32)strlen(face_string);
		__int32 n_faces = 0;
		__int32 vertex_index[NUM_FACES][NUM_VERTICES];
		__int32 normal_index[NUM_FACES][NUM_VERTICES];

		fseek(file, 0, SEEK_SET);

		while (fgets(line, sizeof(line), file)) {

			if (strncmp(line, face_string, length) == 0) {

				unsigned __int32 a = 0;
				for (__int32 i_char = 0; line[i_char] != '\n'; i_char++) {
					unsigned __int32 b = line[i_char] == '/';
					line[i_char] = (a | b) ? ' ' : line[i_char];
					a ^= b;
				}

				char* data_ptr = line + length;
				for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES; i_vertex++) {
					vertex_index[n_faces][i_vertex] = strtol(data_ptr, &data_ptr, 10) - 1;
					normal_index[n_faces][i_vertex] = strtol(data_ptr, &data_ptr, 10) - 1;
				}
				n_faces++;
			}

			if (n_faces == NUM_FACES) {

				float3_ max;
				float3_ min;
				max = min = vertex_chunk[vertex_index[0][0]];
				for (__int32 i_face = 0; i_face < NUM_FACES; i_face++) {
					for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES; i_vertex++) {
						for (__int32 i_axis = X; i_axis < W; i_axis++) {
							max.f[i_axis] = __max(max.f[i_axis], vertex_chunk[vertex_index[i_face][i_vertex]].f[i_axis]);
							min.f[i_axis] = __min(min.f[i_axis], vertex_chunk[vertex_index[i_face][i_vertex]].f[i_axis]);
						}
					}
				}

				//float3_ centre = (max + min) * 0.5f * world_scale;
				//float3_ extent = (max - min) * 0.5f * world_scale;
				float3_ centre = (max + min) * world_scale * 0.5f;
				float3_ extent = (max - min) * world_scale * 0.5f;

				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					model_token.centre[model_token.n_models].i[i_axis] = (__int32)(centre.f[i_axis] * fixed_scale_real);
					model_token.extent[model_token.n_models].i[i_axis] = (__int32)(extent.f[i_axis] * fixed_scale_real);
				}

				for (__int32 i_face = 0; i_face < NUM_FACES; i_face++) {
					model_token.normals[model_token.n_models][i_face] = normal_chunk[normal_index[i_face][0]];
				}

				model_token.n_models++;
				n_faces = 0;
			}
		}
	}

	// -------------------------------------------------------------------------------

	{
		fseek(file, 0, SEEK_SET);

		__int32 i_model = 0;
		const char mtl_string[] = "usemtl";
		const __int32 length = (__int32)strlen(mtl_string);
		while (fgets(line, sizeof(line), file)) {

			char* data_ptr = line + length;
			if (strncmp(line, mtl_string, length) == 0) {
				model_token.index[i_model] = strtol(data_ptr, &data_ptr, 10) - 1;
				i_model++;
			}
		}
	}

	// -------------------------------------------------------------------------------

	fclose(file);

	// -------------------------------------------------------------------------------
}