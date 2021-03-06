
#pragma once

#include "master.h"

#define	CACHE_SIZE	64
#define CACHE_ALIGN __declspec(align(CACHE_SIZE))
#define	SIMD_ALIGN	_declspec(align(16))
#define VM_INLINE __forceinline


//#define INVALID_RESULT (size_t(-1))

//======================================================================

static const float PI = 3.14159265f;
static const float RADIANS_PER_DEGREE = PI / 180.0f;
static const float DEGREES_PER_RADIAN = 57.2958f;

//======================================================================

static const __int32 INVALID_RESULT = -1;
static const __int32 fixed_scale = 1 << 16;
static const float fixed_scale_real = float(fixed_scale);
static const float r_fixed_scale_real = 1.0f / fixed_scale_real;

//======================================================================

enum lamorna_ {

	VERTEX_POSITION,
	VERTEX_COLOUR,
	VERTEX_TEXCOORDS,

	NUM_VERTEX_ATTRIBUTES,

	ATTRIBUTE_COLOUR = 0,
	ATTRIBUTE_TEXCOORD,


	X = 0, Y, Z, W, N_VERTEX_COORDINATES,
	R = 0, G, B, A,
	U = 0, V,  

	MAX_COMPONENTS = NUM_VERTEX_ATTRIBUTES * N_VERTEX_COORDINATES,		// 4 elements a attribute vector
};

//======================================================================



union float4_ {

	struct {
		float x, y, z, w;
	};
	float f[4];
};

typedef float4_ matrix_[4];

union float3_ {
	
	struct {
		float x, y, z;
	};
	struct {
		float r, g, b;
	};
	float f[3];
};

union float2_ {

	struct {
		float x, y;
	};
	struct {
		float u, v;
	};
	struct {
		float s, t;
	};
	float f[2];
};

union int2_ {

	struct {
		__int32 x, y;
	};
	__int32 i[2];
};

union int3_ {

	struct {
		__int32 x, y, z;
	};
	__int32 i[3];
};

union int4_ {

	struct {
		__int32 x, y, z, w;
	};
	__int32 i[4];
};

union vertex4_ {

	struct {
		__m128 x, y, z;
	};
	__m128 v[3];
};

static VM_INLINE  float3_ operator + (float3_ a, float3_ b){

	float3_ result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}
static inline float3_& operator += (float3_& a, const float3_& b) {
	a = a + b;
	return a;
}
static VM_INLINE  float3_ operator - (float3_ a, float3_ b) {

	float3_ result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}
static inline float3_& operator -= (float3_& a, const float3_& b) {
	a = a - b;
	return a;
}
static VM_INLINE  float3_ operator * (float3_ a, float3_ b) {

	float3_ result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	return result;
}
static inline float3_& operator *= (float3_& a, const float3_& b) {
	a = a * b;
	return a;
}
static VM_INLINE  float3_ operator / (float3_ a, float3_ b) {

	float3_ result;
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	result.z = a.z / b.z;
	return result;
}
static inline float3_& operator /= (float3_& a, const float3_& b) {
	a = a / b;
	return a;
}
static VM_INLINE  float3_ operator + (float3_ a, float b) {

	float3_ result;
	result.x = a.x + b;
	result.y = a.y + b;
	result.z = a.z + b;
	return result;
}
static inline float3_& operator += (float3_& a, const float b) {
	a = a + b;
	return a;
}
static VM_INLINE  float3_ operator - (float3_ a, float b) {

	float3_ result;
	result.x = a.x - b;
	result.y = a.y - b;
	result.z = a.z - b;
	return result;
}
static inline float3_& operator -= (float3_& a, const float b) {
	a = a - b;
	return a;
}
static VM_INLINE  float3_ operator * (float3_ a, float b) {

	float3_ result;
	result.x = a.x * b;
	result.y = a.y * b;
	result.z = a.z * b;
	return result;
}
static inline float3_& operator *= (float3_& a, const float b) {
	a = a * b;
	return a;
}
static VM_INLINE  float3_ operator / (float3_ a, float b) {

	float3_ result;
	result.x = a.x / b;
	result.y = a.y / b;
	result.z = a.z / b;
	return result;
}
static inline float3_& operator /= (float3_& a, const float b) {
	a = a / b;
	return a;
}

 static VM_INLINE  __m128 operator + (__m128 a, __m128 b) {
	return _mm_add_ps(a, b);
}
 static inline __m128& operator += (__m128& a, __m128 b) {
	 a = a + b;
	 return a;
 }
 static VM_INLINE __m128 operator - (__m128 a, __m128 b) {
	 return _mm_sub_ps(a, b);
 }
 static VM_INLINE __m128& operator -= (__m128& a, __m128 b) {
	 a = a - b;
	 return a;
 }
 static VM_INLINE __m128 operator * (__m128 a, __m128 b) {
	 return _mm_mul_ps(a, b);
 }
 static VM_INLINE __m128& operator *= (__m128& a, __m128 b) {
	 a = a * b;
	 return a;
 }
 static VM_INLINE __m128 operator / (__m128 a, __m128 b) {
	 return _mm_div_ps(a, b);
 }
 static VM_INLINE __m128& operator /= (__m128 &a, __m128 b) {
	 a = a / b;
	 return a;
 }
 static VM_INLINE __m128 operator - (__m128 a) {		// unary minus
	 return _mm_xor_ps(a, _mm_castsi128_ps(_mm_set1_epi32(0x80000000)));
 }
 static VM_INLINE __m128i operator < (__m128 a, __m128 b) {
	 return _mm_castps_si128(_mm_cmplt_ps(a, b));
 }
 static VM_INLINE __m128i operator <= (__m128 a, __m128 b) {
	 return _mm_castps_si128(_mm_cmple_ps(a, b));
 }
 static VM_INLINE __m128i operator > (__m128 a, __m128 b) {
	 return b < a;
 }
 static VM_INLINE __m128i operator >= (__m128 a, __m128 b) {
	 return b <= a;
 }
 static VM_INLINE __m128i operator == (__m128 a, __m128 b) {
	 return _mm_castps_si128(_mm_cmpeq_ps(a, b));
 }
 static VM_INLINE __m128i operator != (__m128 a, __m128 b) {
	 return _mm_castps_si128(_mm_cmpneq_ps(a, b));
 }
 static VM_INLINE __m128 operator & (__m128 a, __m128 b) {
	 return _mm_and_ps(a, b);
 }
 static VM_INLINE __m128& operator &= (__m128& a, __m128 b) {
	 a = a & b;
	 return a;
 }
 static VM_INLINE __m128 operator & (__m128 a, __m128i b) {
	 return _mm_and_ps(a, _mm_castsi128_ps(b));
 }
 static VM_INLINE __m128& operator &= (__m128& a, __m128i b) {
	 a = a & b;
	 return a;
 }
 static VM_INLINE __m128 operator | (__m128 a, __m128 b) {
	 return _mm_or_ps(a, b);
 }
 static VM_INLINE __m128& operator |= (__m128& a, __m128 b) {
	 a = a | b;
	 return a;
 }
 static VM_INLINE __m128 operator | (__m128 a, __m128i& b) {
	 return _mm_or_ps(a, _mm_castsi128_ps(b));
 }
 static VM_INLINE __m128& operator |= (__m128& a, __m128i b) {
	 a = a | b;
	 return a;
 }
 static VM_INLINE __m128 operator ^ (__m128 a, __m128 b) {
	 return _mm_xor_ps(a, b);
 }
 static VM_INLINE __m128& operator ^= (__m128& a, __m128 b) {
	 a = a ^ b;
	 return a;
 }
 static VM_INLINE __m128 operator ^ (__m128 a, __m128i b) {
	 return _mm_xor_ps(a, _mm_castsi128_ps(b));
 }
 static VM_INLINE __m128& operator ^= (__m128& a, __m128i b) {
	 a = a ^ b;
	 return a;
 }

//------------------------------------------------------------------------
// __m128 functions
//------------------------------------------------------------------------

static VM_INLINE __m128 set(float a, float b, float c, float d){
	return _mm_setr_ps(a, b, c, d);
}
static VM_INLINE __m128 set_all(float a) {
	__m128 temp = _mm_set_ss(a);
	return _mm_shuffle_ps(temp, temp, 0x0);
}
static VM_INLINE __m128 load(const float *mem_addr) {
	return _mm_load_ps(mem_addr);
}
static VM_INLINE __m128 load_u(const float *mem_addr) {
	return _mm_loadu_ps(mem_addr);
}
static VM_INLINE __m128 load_s(float a) {
	return _mm_set_ss(a);
}
static VM_INLINE void store(__m128 a, float *mem_addr) {
	_mm_store_ps(mem_addr, a);
}
static VM_INLINE void store_u(__m128 a, float *mem_addr) {
	_mm_storeu_ps(mem_addr, a);
}
static VM_INLINE float store_s(__m128 a) {
	return _mm_cvtss_f32(a);
}
static VM_INLINE __int32 store_mask(__m128 a) {
	return _mm_movemask_ps(a);
}

static VM_INLINE __m128i convert_int_round(__m128 a) {		// Note: assume MXCSR control register is set to rounding
	return _mm_cvtps_epi32(a);
}
static VM_INLINE __m128i convert_int_trunc(__m128 a) {
	return _mm_cvttps_epi32(a);
}
static VM_INLINE __m128 set_zero(void) {
	return _mm_setzero_ps();
}
static VM_INLINE __m128 reciprocal(__m128 a) {					// faster than 1.0f/x
	return _mm_rcp_ps(a);
}
static VM_INLINE __m128 rotate_right(__m128 a) {
	return _mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 3, 2, 1));
}
static VM_INLINE __m128 rotate_left(__m128 a) {
	return _mm_shuffle_ps(a, a, _MM_SHUFFLE(2, 1, 0, 3));		// 3 2 1 0
}
static VM_INLINE __m128 broadcast(__m128 a) {
	return _mm_shuffle_ps(a, a, 0x0);
}
static VM_INLINE __m128 broadcast_y(__m128 a) {
	return _mm_shuffle_ps(a, a, 0x55);
}
static VM_INLINE __m128 broadcast_z(__m128 a) {
	return _mm_shuffle_ps(a, a, 0xaa);
}
static VM_INLINE __m128 broadcast_w(__m128 a) {
	return _mm_shuffle_ps(a, a, 0xff);
}
static VM_INLINE __m128 max_vec(__m128 a, __m128 b) {
	return _mm_max_ps(a, b);
}
static VM_INLINE __m128 min_vec(__m128 a, __m128 b) {
	return _mm_min_ps(a, b);
}
static VM_INLINE __m128 interleave_lo(__m128 a, __m128 b) {
	return _mm_unpacklo_ps(a, b);
}
static VM_INLINE __m128 interleave_hi(__m128 a, __m128 b) {
	return _mm_unpackhi_ps(a, b);
}
static VM_INLINE __m128 and_not(__m128 a, __m128i b) {
	return _mm_andnot_ps(_mm_castsi128_ps(b), a);
}
static VM_INLINE __m128 lerp(__m128 a, __m128 b, float t) {
	return a + ((b - a) * set_all(t));
}
static VM_INLINE __m128 lerp(__m128 a, __m128 b, __m128 t) {
	return a + ((b - a) * t);
}
static VM_INLINE __m128 blend(__m128 a, __m128 b, __m128i mask){
	//__m128 and_ = _mm_and_ps(a, _mm_castsi128_ps(mask));
	//__m128 and_not = _mm_andnot_ps(_mm_castsi128_ps(mask), b);
	//return _mm_or_ps(and_, and_not);
	return _mm_blendv_ps(b, a, _mm_castsi128_ps(mask));
}
static VM_INLINE __m128 abs(__m128 a) {
	__m128 sign = _mm_and_ps(a, _mm_castsi128_ps(_mm_set1_epi32(0x80000000)));
	return _mm_xor_ps(a, sign);
}
static VM_INLINE __m128 horizontal_min(__m128 a){
	__m128 max1 = _mm_movehl_ps(a, a);
	__m128 max2 = _mm_min_ps(a, max1);
	__m128 max3 = _mm_shuffle_ps(max2, max2, _MM_SHUFFLE(1, 1, 1, 1));
	return _mm_min_ps(max2, max3);
}
static VM_INLINE __m128 horizontal_max(__m128 a){
	__m128 max1 = _mm_movehl_ps(a, a);
	__m128 max2 = _mm_max_ps(a, max1);
	__m128 max3 = _mm_shuffle_ps(max2, max2, _MM_SHUFFLE(1, 1, 1, 1));
	return _mm_max_ps(max2, max3);
}
static VM_INLINE __m128i cast_int32(__m128 a) {
	return _mm_castps_si128(a);
}

//------------------------------------------------------------------------
// __m128i operators
//------------------------------------------------------------------------

static VM_INLINE __m128i operator + (__m128i a, __m128i b) {
	return _mm_add_epi32(a, b);
}
static VM_INLINE __m128i& operator += (__m128i& a, __m128i b) {
	a = a + b;
	return a;
}
static VM_INLINE __m128i operator - (__m128i a, __m128i b) {
	return _mm_sub_epi32(a, b);
}
static VM_INLINE __m128i& operator -= (__m128i& a, __m128i b) {
	a = a - b;
	return a;
}
static VM_INLINE __m128i operator * (__m128i a, __m128i b) {
	//__m128i a13 = _mm_shuffle_epi32(a, 0xF5);				// (-,a3,-,a1)
	//__m128i b13 = _mm_shuffle_epi32(b, 0xF5);				// (-,b3,-,b1)
	//__m128i prod02 = _mm_mul_epu32(a, b);					// (-,a2*b2,-,a0*b0)
	//__m128i prod13 = _mm_mul_epu32(a13, b13);				// (-,a3*b3,-,a1*b1)
	//__m128i prod01 = _mm_unpacklo_epi32(prod02, prod13);	// (-,-,a1*b1,a0*b0) 
	//__m128i prod23 = _mm_unpackhi_epi32(prod02, prod13);	// (-,-,a3*b3,a2*b2) 
	//return           _mm_unpacklo_epi64(prod01, prod23);	// (ab3,ab2,ab1,ab0)
	return _mm_mullo_epi32(a, b);
}
static VM_INLINE __m128i& operator *= (__m128i& a, __m128i b) {
	a = a * b;
	return a;
}
static VM_INLINE __m128i operator ~ (__m128i a) {
	return _mm_xor_si128(a, _mm_set1_epi32(-1));
}
static VM_INLINE __m128i operator > (__m128i a, __m128i b) {
	return _mm_cmpgt_epi32(a, b);
}
static VM_INLINE __m128i operator < (__m128i a, __m128i b) {
	return b > a;
}
static VM_INLINE __m128i operator >= (__m128i a, __m128i b) {
	return ~(b > a);
}
static VM_INLINE __m128i operator <= (__m128i a, __m128i b) {
	return b >= a;
}
static VM_INLINE __m128i operator << (__m128i a, __int32 imm8) {
	return _mm_slli_epi32(a, imm8);
}
static VM_INLINE __m128i& operator <<= (__m128i& a, __int32 imm8) {
	a = a << imm8;
	return a;
}
static VM_INLINE __m128i operator << (__m128i a, __m128i count) {
	return _mm_sll_epi32(a, count);
}
static VM_INLINE __m128i& operator <<= (__m128i& a, __m128i count) {
	a = a << count;
	return a;
}
static VM_INLINE __m128i operator >> (__m128i a, __int32 imm8) {
	return _mm_srai_epi32(a, imm8);										
}
static VM_INLINE __m128i& operator >>= (__m128i& a, __int32 imm8) {
	a = a >> imm8;
	return a;
}
static VM_INLINE __m128i operator >> (__m128i a, __m128i count) {
	return _mm_sra_epi32(a, count);
}
static VM_INLINE __m128i& operator >>= (__m128i& a, __m128i count) {
	a = a >> count;
	return a;
}
static VM_INLINE __m128i operator & (__m128i a, __m128i b) {
	return _mm_and_si128(a, b);
}
static VM_INLINE __m128i& operator &= (__m128i& a, __m128i b) {
	a = a & b;
	return a;
}
static VM_INLINE __m128i operator | (__m128i a, __m128i b) {
	return _mm_or_si128(a, b);
}
static VM_INLINE __m128i& operator |= (__m128i& a, __m128i b) {
	a = a | b;
	return a;
}
static VM_INLINE __m128i operator ^ (__m128i a, __m128i b) {
	return _mm_xor_si128(a, b);
}
static VM_INLINE __m128i& operator ^= (__m128i& a, __m128i b) {
	a = a ^ b;
	return a;
}
static VM_INLINE __m128i operator == (__m128i a, __m128i b) {
	return _mm_cmpeq_epi32(a, b);
}
static VM_INLINE __m128i operator != (__m128i a, __m128i b) {
	return ~(a == b);
}
//------------------------------------------------------------------------
// __m128i functions
//------------------------------------------------------------------------

static VM_INLINE __m128i set(__int32 a, __int32 b, __int32 c, __int32 d) {
	return _mm_setr_epi32(a, b, c, d);
}
static VM_INLINE __m128i set_all(__int32 a) {
	return  _mm_set1_epi32(a);
}
static VM_INLINE __m128i add_uint8_saturate(__m128i a, __m128i b) {
	return _mm_adds_epu8(a, b);
}
static VM_INLINE __m128i load(const __int32 *mem_addr) {
	return _mm_load_si128((__m128i *)mem_addr);
}
static VM_INLINE __m128i load(const unsigned __int32 *mem_addr) {
	return _mm_load_si128((__m128i *)mem_addr);
}
static VM_INLINE __m128i load_u(const __int32 *mem_addr) {
	return _mm_loadu_si128((__m128i *)mem_addr);
}
static VM_INLINE __m128i load_u(const unsigned __int32 *mem_addr) {
	return _mm_loadu_si128((__m128i *)mem_addr);
}
static VM_INLINE __m128i load_s(__int32 a) {
	return _mm_cvtsi32_si128(a);
}
static VM_INLINE __m128i load_s(unsigned __int32 a) {
	return _mm_cvtsi32_si128(a);
}
static VM_INLINE void store(__m128i a, __int32 *mem_addr) {
	_mm_store_si128((__m128i *)mem_addr, a);
}
static VM_INLINE void store(__m128i a, unsigned __int32 *mem_addr) {
	_mm_store_si128((__m128i *)mem_addr, a);
}
static VM_INLINE void store_u(__m128i a, __int32 *mem_addr) {
	_mm_storeu_si128((__m128i *)mem_addr, a);
}
static VM_INLINE void store_u(__m128i a, unsigned __int32 *mem_addr) {
	_mm_storeu_si128((__m128i *)mem_addr, a);
}
static VM_INLINE __int32 store_s(__m128i a) {
	return _mm_cvtsi128_si32(a);
}
static VM_INLINE __int32 store_mask(__m128i a) {
	return _mm_movemask_ps(_mm_castsi128_ps(a));
}
static VM_INLINE __m128 convert_float(__m128i a) {
	return _mm_cvtepi32_ps(a);
}
static VM_INLINE __m128i set_zero_si128(void) {
	return _mm_setzero_si128();
}
static VM_INLINE __m128i rotate_right(__m128i a) {
	return _mm_shuffle_epi32(a, _MM_SHUFFLE(0, 3, 2, 1));
}
static VM_INLINE __m128i rotate_left(__m128i a) {
	return _mm_shuffle_epi32(a, _MM_SHUFFLE(2, 1, 0, 3));
}
static VM_INLINE __m128i broadcast(__m128i a) {
	return _mm_shuffle_epi32(a, 0x0);
}
static VM_INLINE __m128i broadcast_y(__m128i a) {
	return _mm_shuffle_epi32(a, 0x55);
}
static VM_INLINE __m128i broadcast_z(__m128i a) {
	return _mm_shuffle_epi32(a, 0xaa);
}
static VM_INLINE __m128i broadcast_w(__m128i a) {
	return _mm_shuffle_epi32(a, 0xff);
}
static VM_INLINE __m128i interleave_lo(__m128i a, __m128i b) {
	return _mm_unpacklo_epi32(a, b);
}
static VM_INLINE __m128i interleave_hi(__m128i a, __m128i b) {
	return _mm_unpackhi_epi32(a, b);
}
static VM_INLINE __m128i blend(__m128i a, __m128i b, __m128i mask){
	//__m128i and_ = _mm_and_si128(a, mask);
	//__m128i and_not = _mm_andnot_si128(mask, b);
	//return _mm_or_si128(and_, and_not);
	return _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(b), _mm_castsi128_ps(a), _mm_castsi128_ps(mask)));
}
static VM_INLINE __m128i max_vec(__m128i a, __m128i b) {
	//return blend(a, b, a > b);
	return _mm_max_epi32(a, b);
}
static VM_INLINE __m128i min_vec(__m128i a, __m128i b) {
	//return blend(a, b, a < b);
	return _mm_min_epi32(a, b);
}
static VM_INLINE __m128 cast_float32(__m128i a) {
	return _mm_castsi128_ps(a);
}
static VM_INLINE __m128i abs(__m128i a) {
	__m128i mask = (a >> 31) != set_zero_si128();
	return (a + mask) ^ mask;
}

static __m128i const load_mask[] = {

	set( 0x0, 0x0, 0x0, 0x0		),		// 0  |  		| 0000
	set( ~0x0, 0x0, 0x0, 0x0	),		// 1  | x		| 0001
	set( 0x0, ~0x0, 0x0, 0x0	),		// 2  | y		| 0010
	set( ~0x0, ~0x0, 0x0, 0x0	),		// 3  | x y		| 0011
	
	set( 0x0, 0x0, ~0x0, 0x0	),		// 4  | z 		| 0100
	set( ~0x0, 0x0, ~0x0, 0x0	),		// 5  | x z		| 0101
	set( 0x0, ~0x0, ~0x0, 0x0	),		// 6  | y z		| 0110			
	set( ~0x0, ~0x0, ~0x0, 0x0	),		// 7  | x y z	| 0111
	
	set( 0x0, 0x0, 0x0, ~0x0	),		// 8  | w 		| 1000
	set( ~0x0, 0x0, 0x0, ~0x0	),		// 9  | x w		| 1001
	set( 0x0, ~0x0, 0x0, ~0x0	),		// 10 | y w		| 1010
	set( ~0x0, ~0x0, 0x0, ~0x0	),		// 11 | x y w	| 1011
	
	set( 0x0, 0x0, ~0x0, ~0x0	),		// 12 | z w		| 1100
	set( ~0x0, 0x0, ~0x0, ~0x0	),		// 13 | x z w	| 1101
	set( 0x0, ~0x0, ~0x0, ~0x0	),		// 14 | y z w	| 1110
	set( ~0x0, ~0x0, ~0x0, ~0x0 ),		// 15 | x y z w	| 1111

};

static const float SIGN_BIT = 0x80000000;
static __m128i const SIGN_BIT_4 = set_all((__int32)SIGN_BIT);

static __m128 const Unit_Axis[4] = {

	set( 1.0f, 0.0f, 0.0f, 0.0f ),
	set( 0.0f, 1.0f, 0.0f, 0.0f ),
	set( 0.0f, 0.0f, 1.0f, 0.0f ),
	set( 0.0f, 0.0f, 0.0f, 1.0f ),
};

static __m128i const Axis_Mask[4] = {

	set( ~0x0, 0x0, 0x0, 0x0 ),
	set( 0x0, ~0x0, 0x0, 0x0 ),
	set( 0x0, 0x0, ~0x0, 0x0 ),
	set( 0x0, 0x0, 0x0, ~0x0 ),
};

//======================================================================

//------------------------------------------------------------------------
// matrix operators
//------------------------------------------------------------------------

static void Matrix_X_Matrix(const __m128 a[4], const __m128 b[4], __m128 out[4]){

	for (__int32 i_row = 0; i_row < 4; i_row++) {

		out[i_row]  = a[X] * _mm_shuffle_ps(b[i_row], b[i_row], 0x0);
		out[i_row] += a[Y] * _mm_shuffle_ps(b[i_row], b[i_row], 0x55);
		out[i_row] += a[Z] * _mm_shuffle_ps(b[i_row], b[i_row], 0xaa);
		out[i_row] += a[W] * _mm_shuffle_ps(b[i_row], b[i_row], 0xff);
	}
}

static void Matrix_X_Matrix(const matrix_& a, const matrix_& b, matrix_& out) {

	for (__int32 i_row = 0; i_row < 4; i_row++) {

		__m128 sum = load_u(a[X].f) * set_all(b[i_row].x);
		sum += load_u(a[Y].f) * set_all(b[i_row].y);
		sum += load_u(a[Z].f) * set_all(b[i_row].z);
		sum += load_u(a[W].f) * set_all(b[i_row].w);

		store_u(sum, out[i_row].f);
	}
}

static __m128 Vector_X_Matrix(const __m128 a, const __m128 b[4]) {

	__m128 out;
	out  = b[X] * _mm_shuffle_ps(a, a, 0x0);
	out += b[Y] * _mm_shuffle_ps(a, a, 0x55);
	out += b[Z] * _mm_shuffle_ps(a, a, 0xaa);
	out += b[W] * _mm_shuffle_ps(a, a, 0xff);

	return out;
}

static void Vector_X_Matrix(const float3_& in, const matrix_& m, float3_& out) {

	__m128 result = set_zero();
	result += load_u(m[X].f) * set_all(in.x);
	result += load_u(m[Y].f) * set_all(in.y);
	result += load_u(m[Z].f) * set_all(in.z);
	result += load_u(m[W].f) * set_all(1.0f);
	float temp[4];
	store_u(result, temp);
	out.x = temp[X];
	out.y = temp[Y];
	out.z = temp[Z];
}

static void Vector_X_Matrix(const float3_& in, const matrix_& m, float4_& out) {

	__m128 result = set_zero();
	result += load_u(m[X].f) * broadcast(load_s(in.x));
	result += load_u(m[Y].f) * broadcast(load_s(in.y));
	result += load_u(m[Z].f) * broadcast(load_s(in.z));
	result += load_u(m[W].f) * set_all(1.0f);
	store_u(result, out.f);
}

static void Vector_X_Matrix(const float4_& in, const matrix_& m, float4_& out) {

	__m128 result = set_zero();
	result += load_u(m[X].f) * broadcast(load_s(in.x));
	result += load_u(m[Y].f) * broadcast(load_s(in.y));
	result += load_u(m[Z].f) * broadcast(load_s(in.z));
	result += load_u(m[W].f) * broadcast(load_s(in.w));
	store_u(result, out.f);
}

//------------------------------------------------------------------------
// matrix functions
//------------------------------------------------------------------------

static void Transpose(__m128 in[4]) {

	__m128 temp[4];
	temp[0] = interleave_lo(in[0], in[2]);			// x0 x2 y0 y2
	temp[1] = interleave_lo(in[1], in[3]);			// x1 x3 y1 y3
	temp[2] = interleave_hi(in[0], in[2]);			// z0 z2 w0 w2
	temp[3] = interleave_hi(in[1], in[3]);			// z1 z3 w1 w3

	in[0] = interleave_lo(temp[0], temp[1]);		// x0 x1 x2 x3
	in[1] = interleave_hi(temp[0], temp[1]);		// y0 y1 y2 y3
	in[2] = interleave_lo(temp[2], temp[3]);		// z0 z1 z2 z3
	in[3] = interleave_hi(temp[2], temp[3]);		// w0 w1 w2 w3
}

static void Transpose(const __m128 in[4], __m128 out[4]) {

	__m128 temp[4];
	temp[0] = interleave_lo(in[0], in[2]);			// x0 x2 y0 y2
	temp[1] = interleave_lo(in[1], in[3]);			// x1 x3 y1 y3
	temp[2] = interleave_hi(in[0], in[2]);			// z0 z2 w0 w2
	temp[3] = interleave_hi(in[1], in[3]);			// z1 z3 w1 w3

	out[0] = interleave_lo(temp[0], temp[1]);		// x0 x1 x2 x3
	out[1] = interleave_hi(temp[0], temp[1]);		// y0 y1 y2 y3
	out[2] = interleave_lo(temp[2], temp[3]);		// z0 z1 z2 z3
	out[3] = interleave_hi(temp[2], temp[3]);		// w0 w1 w2 w3
}

static void Transpose(const matrix_ in, matrix_ out) {


	__m128 load[4];
	load[0] = load_u(in[0].f);
	load[1] = load_u(in[1].f);
	load[2] = load_u(in[2].f);
	load[3] = load_u(in[3].f);

	__m128 temp_one[4];
	temp_one[0] = interleave_lo(load[0], load[2]);			// x0 x2 y0 y2
	temp_one[1] = interleave_lo(load[1], load[3]);			// x1 x3 y1 y3
	temp_one[2] = interleave_hi(load[0], load[2]);			// z0 z2 w0 w2
	temp_one[3] = interleave_hi(load[1], load[3]);			// z1 z3 w1 w3

	__m128 temp_two[4];
	temp_two[0] = interleave_lo(temp_one[0], temp_one[1]);		// x0 x1 x2 x3
	temp_two[1] = interleave_hi(temp_one[0], temp_one[1]);		// y0 y1 y2 y3
	temp_two[2] = interleave_lo(temp_one[2], temp_one[3]);		// z0 z1 z2 z3
	temp_two[3] = interleave_hi(temp_one[2], temp_one[3]);		// w0 w1 w2 w3

	store_u(temp_two[0], out[0].f);
	store_u(temp_two[1], out[1].f);
	store_u(temp_two[2], out[2].f);
	store_u(temp_two[3], out[3].f);
}



static void Transpose(__m128i in[4]) {

	__m128i temp[4];
	temp[0] = interleave_lo(in[0], in[2]);			// x0 x2 y0 y2
	temp[1] = interleave_lo(in[1], in[3]);			// x1 x3 y1 y3
	temp[2] = interleave_hi(in[0], in[2]);			// z0 z2 w0 w2
	temp[3] = interleave_hi(in[1], in[3]);			// z1 z3 w1 w3

	in[0] = interleave_lo(temp[0], temp[1]);		// x0 x1 x2 x3
	in[1] = interleave_hi(temp[0], temp[1]);		// y0 y1 y2 y3
	in[2] = interleave_lo(temp[2], temp[3]);		// z0 z1 z2 z3
	in[3] = interleave_hi(temp[2], temp[3]);		// w0 w1 w2 w3
}

static void Transpose(const __m128i in[4], __m128i out[4]) {

	__m128i temp[4];
	temp[0] = interleave_lo(in[0], in[2]);			// x0 x2 y0 y2
	temp[1] = interleave_lo(in[1], in[3]);			// x1 x3 y1 y3
	temp[2] = interleave_hi(in[0], in[2]);			// z0 z2 w0 w2
	temp[3] = interleave_hi(in[1], in[3]);			// z1 z3 w1 w3

	out[0] = interleave_lo(temp[0], temp[1]);		// x0 x1 x2 x3
	out[1] = interleave_hi(temp[0], temp[1]);		// y0 y1 y2 y3
	out[2] = interleave_lo(temp[2], temp[3]);		// z0 z1 z2 z3
	out[3] = interleave_hi(temp[2], temp[3]);		// w0 w1 w2 w3
}

