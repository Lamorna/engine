#pragma once

struct memory_ {

	enum {

		___1_KiB = 1 << 10,
		___2_KiB = 1 << 11,
		___4_KiB = 1 << 12,
		___8_KiB = 1 << 13,
		__16_KiB = 1 << 14,
		__32_KiB = 1 << 15,
		__64_KiB = 1 << 16,
		_128_KiB = 1 << 17,
		_256_KiB = 1 << 18,
		_512_KiB = 1 << 19,

		___1_MiB = 1 << 20,
		___2_MiB = 1 << 21,
		___4_MiB = 1 << 22,
		___8_MiB = 1 << 23,
		__16_MiB = 1 << 24,
		__32_MiB = 1 << 25,
		__64_MiB = 1 << 26,
		_128_MiB = 1 << 27,
		_256_MiB = 1 << 28,
		_512_MiB = 1 << 29,

	};
};

struct memory_chunk_ {

	enum {
		NUM_BYTES = memory_::__64_MiB,
	};

	void* chunk_ptr;
	__int8 chunk[NUM_BYTES];
};
