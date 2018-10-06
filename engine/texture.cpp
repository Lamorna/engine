#include "texture.h"
#include "setup.h"
#include "memory.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

/*
==================
==================
*/
void Build_MIP_Map_Chain(

	texture_handler_& texture_handler,
	memory_chunk_& memory_chunk

) {


	__int32 width_read = texture_handler.width;
	__int32 height_read = texture_handler.height;
	__int32 shortest_side = min(width_read, height_read);
	unsigned long log = 0;
	_BitScanForward(&log, shortest_side);
	const __int32 n_mip_levels = log + 1;
	texture_handler.n_mip_levels = n_mip_levels;
	//printf_s("NUM MIP LEVELS: %i \n", n_mip_levels);

	//unsigned __int32 test_colour[] = {

	//	(200 << 16) | (000 << 8) | (000 << 0),
	//	(000 << 16) | (200 << 8) | (000 << 0),
	//	(000 << 16) | (000 << 8) | (200 << 0),
	//	(200 << 16) | (200 << 8) | (200 << 0),
	//	(200 << 16) | (200 << 8) | (200 << 0),
	//	(200 << 16) | (200 << 8) | (200 << 0),
	//	(200 << 16) | (200 << 8) | (200 << 0),
	//	(200 << 16) | (200 << 8) | (200 << 0),
	//};

	const __int32 n_mip_writes = n_mip_levels - 1;

	for (__int32 i_mip_level = 0; i_mip_level < n_mip_writes; i_mip_level++) {

		texture_handler.texture[i_mip_level + 1] = (unsigned __int32*)memory_chunk.chunk_ptr;
		//unsigned __int32 mod_width = width_read - 1;
		//unsigned __int32 mod_height = height_read - 1;

		__int32 i_index = 0;
		for (__int32 y = 0; y < height_read; y += 2) {
			for (__int32 x = 0; x < width_read; x += 2) {

				//__int32 x_step = (x + 1) & mod_width;
				//__int32 y_step = (y + 1) & mod_height;
				__int32 x_step = (x + 1) % width_read;
				__int32 y_step = (y + 1) % height_read;

				unsigned __int32 colour_read[4];
				colour_read[0] = texture_handler.texture[i_mip_level][(y * width_read) + x];
				colour_read[1] = texture_handler.texture[i_mip_level][(y * width_read) + x_step];
				colour_read[2] = texture_handler.texture[i_mip_level][(y_step * width_read) + x];
				colour_read[3] = texture_handler.texture[i_mip_level][(y_step * width_read) + x_step];

				float colour_sum[3] = { 0.0f, 0.0f, 0.0f };
				for (__int32 i = 0; i < 4; i++) {

					colour_sum[R] += float((colour_read[i] >> 0) & 0xff);
					colour_sum[G] += float((colour_read[i] >> 8) & 0xff);
					colour_sum[B] += float((colour_read[i] >> 16) & 0xff);
				}

				unsigned __int32 colour_out[3];
				colour_out[R] = unsigned __int32(colour_sum[R] * 0.25f);
				colour_out[G] = unsigned __int32(colour_sum[G] * 0.25f);
				colour_out[B] = unsigned __int32(colour_sum[B] * 0.25f);

				for (__int32 i_channel = R; i_channel < A; i_channel++) {

					colour_out[i_channel] = max(colour_out[i_channel], 0);
					colour_out[i_channel] = min(colour_out[i_channel], 255);
				}

				unsigned __int32 final_colour = (colour_out[R] << 0) | (colour_out[G] << 8) | (colour_out[B] << 16);
				texture_handler.texture[i_mip_level + 1][i_index] = final_colour;
				i_index++;
			}
		}

		width_read >>= 1;
		height_read >>= 1;
		memory_chunk.chunk_ptr = texture_handler.texture[i_mip_level + 1] + (width_read * height_read);
	}
}

/*
==================
==================
*/
void Load_Image_STB(

	const char* filename,
	texture_handler_& texture_handler,
	memory_chunk_& memory_chunk
) {

	FILE *file_handle;
	fopen_s(&file_handle, filename, "rb");		// txt mode
	if (!file_handle) {
		printf_s("FAILED to open %s \n", filename);
		exit(0);
	}

	__int32 width;
	__int32 height;
	__int32 comp;
	__int32 n_components_per_pixel = 4;
	unsigned char *data;
	data = stbi_load_from_file(file_handle, &width, &height, &comp, n_components_per_pixel);

	texture_handler.texture[0] = (unsigned __int32*)memory_chunk.chunk_ptr;
	texture_handler.width = width;
	texture_handler.height = height;

	unsigned long width_shift;
	_BitScanForward(&width_shift, width);
	unsigned long height_shift;
	_BitScanForward(&height_shift, height);
	texture_handler.width_shift = width_shift;
	texture_handler.height_shift = height_shift;

	memcpy(texture_handler.texture[0], data, width * height * 4);

	union pixel {
		unsigned __int32 pixel_int;
		unsigned __int8 pixel_char[4];
	};
	for (__int32 i = 0; i < (width * height); i++) {
		pixel pixel;
		pixel.pixel_int = texture_handler.texture[0][i];
		texture_handler.texture[0][i] = pixel.pixel_char[B] | (pixel.pixel_char[G] << 8) | (pixel.pixel_char[R] << 16) | (pixel.pixel_char[A] << 24);
	}

	memory_chunk.chunk_ptr = texture_handler.texture[0] + (texture_handler.width * texture_handler.height);

	Build_MIP_Map_Chain(texture_handler, memory_chunk);

	fclose(file_handle);
	stbi_image_free(data);
}