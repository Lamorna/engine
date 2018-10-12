#pragma once

#include "render_front.h"

struct visibility_data_ {

	enum {

		NUM_BUCKETS = 16,
		NUM_ENTRIES = screen_bin_::MAX_TRIANGLES_PER_BIN / NUM_BUCKETS,

	};
	struct bucket_ {

		__int32 n_triangles;
		__int32 triangle_ids[NUM_ENTRIES];
	};

	struct hash_map_ {

		__int32 draw_id;
		__int32 n_triangles;
		bucket_ buckets[NUM_BUCKETS];
	};

	__int32 n_active_hash_maps;
	__int32 n_active_buckets;
	hash_map_ hash_maps[draw_call_::id_::COUNT];
	hash_map_* active_hash_maps[draw_call_::id_::COUNT];
	bucket_* active_buckets[NUM_BUCKETS];

};
