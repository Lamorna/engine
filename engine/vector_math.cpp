#include "vector_math.h"

static __m128i const XYZ_Mask =	set( ~0x0, ~0x0, ~0x0, 0x0	);


const static __m128i Move_Distances_LUT[] = {

	set( 0, 0, 0, 0 ),		// 0  |  		| 0000	| 0x
	set( 0, 0, 0, 0 ),		// 1  | x		| 0001	| 0x1
	set( 0, 1, 0, 0 ),		// 2  | y		| 0010	| 0x2
	set( 0, 0, 0, 0 ),		// 3  | x y		| 0011	| 0x3
	
	set( 0, 0, 2, 0 ),		// 4  | z 		| 0100	| 0x4
	set( 0, 0, 1, 0 ),		// 5  | x z		| 0101	| 0x5
	set( 0, 1, 1, 0 ),		// 6  | y z		| 0110	| 0x6
	set( 0, 0, 0, 0 ),		// 7  | x y z	| 0111	| 0x7
	
	set( 0, 0, 0, 3 ),		// 8  | w 		| 1000	| 0x8
	set( 0, 0, 0, 2 ),		// 9  | x w		| 1001	| 0x9
	set( 0, 1, 0, 2 ),		// 10 | y w		| 1010	| 0xa
	set( 0, 0, 0, 1 ),		// 11 | x y w	| 1011	| 0xb
	
	set( 0, 0, 2, 2 ),		// 12 | z w		| 1100	| 0xc
	set( 0, 0, 1, 1 ),		// 13 | x z w	| 1101	| 0xd
	set( 0, 1, 1, 1 ),		// 14 | y z w	| 1110	| 0xe
	set( 0, 0, 0, 0 ),		// 15 | x y z w	| 1111	| 0xf
};

//======================================================================

/*
==================
==================
*/
__m128 Pack_Vector4(const __m128 &input, const unsigned __int32 mask_32) {

	__m128 rotate = input;
	__m128 output = input;
	__m128i move_distance = Move_Distances_LUT[mask_32];
	__m128i zero = set_zero_si128();
	__m128i one = set_all(1);
	for (__int32 i_rotate = 0; i_rotate < 3; i_rotate++){
		rotate = rotate_right(rotate);
		move_distance = rotate_right(move_distance);
		move_distance -= one;
		__m128i mask = (move_distance == zero);
		output = blend(rotate, output, mask);
	}
	return output;
}

/*
==================
==================
*/
__m128i Pack_Vector4(const __m128i &input, const unsigned __int32 mask_32) {

	__m128i rotate = input;
	__m128i output = input;
	__m128i move_distance = Move_Distances_LUT[mask_32];
	__m128i zero = set_zero_si128();
	__m128i one = set_all(1);

	//for (__int32 i_rotate = 0; i_rotate < 3; i_rotate++){
	//	rotate = rotate_right(rotate);
	//	move_distance = rotate_right(move_distance);
	//	move_distance -= one;
	//	__m128i mask = (move_distance == zero);
	//	output = blend(rotate, output, mask);
	//}

	{
		rotate = rotate_right(rotate);
		move_distance = rotate_right(move_distance);
		move_distance -= one;
		__m128i mask = (move_distance == zero);
		output = blend(rotate, output, mask);
	}
	{
		rotate = rotate_right(rotate);
		move_distance = rotate_right(move_distance);
		move_distance -= one;
		__m128i mask = (move_distance == zero);
		output = blend(rotate, output, mask);
	}
	{
		rotate = rotate_right(rotate);
		move_distance = rotate_right(move_distance);
		move_distance -= one;
		__m128i mask = (move_distance == zero);
		output = blend(rotate, output, mask);
	}

	return output;
}

/*
==================
==================
*/
void Vector_Normalise(float3_& in) {

	float3_ temp;
	temp.x = in.x * in.x;
	temp.y = in.y * in.y;
	temp.z = in.z * in.z;
	float sum = temp.x + temp.y + temp.z;
	float r_mag = store_s(_mm_rsqrt_ss(load_s(sum)));
	bool is_zero = sum == 0.0f;
	//r_mag = blend(0.0f, r_mag, is_zero);
	r_mag = is_zero ? 0.0f : r_mag;
	in.x *= r_mag;
	in.y *= r_mag;
	in.z *= r_mag;
}

/*
==================
==================
*/
void Vector_Normalise(const float3_& in, float3_& out) {

	float3_ temp;
	temp.x = in.x * in.x;
	temp.y = in.y * in.y;
	temp.z = in.z * in.z;
	float sum = temp.x + temp.y + temp.z;
	float r_mag = store_s(_mm_rsqrt_ss(load_s(sum)));
	bool is_zero = sum == 0.0f;
	//r_mag = blend(0.0f, r_mag, is_zero);
	r_mag = is_zero ? 0.0f : r_mag;
	out.x = in.x * r_mag;
	out.y = in.y * r_mag;
	out.z = in.z * r_mag;
}

/*
==================
==================
*/
float Vector_Normalise_Magnitude(const float3_& in, float3_& out) {

	float3_ temp;
	temp.x = in.x * in.x;
	temp.y = in.y * in.y;
	temp.z = in.z * in.z;
	float sum = temp.x + temp.y + temp.z;
	float mag = store_s(_mm_sqrt_ss(load_s(sum)));
	float r_mag = store_s(reciprocal(load_s(mag)));
	bool is_zero = sum == 0.0f;
	//r_mag = blend(0.0f, r_mag, is_zero);
	r_mag = is_zero ? 0.0f : r_mag;
	out.x = in.x * r_mag;
	out.y = in.y * r_mag;
	out.z = in.z * r_mag;
	return mag;
}

/*
==================
==================
*/
float Vector_Magnitude(const float3_& in) {

	float3_ temp;
	temp.x = in.x * in.x;
	temp.y = in.y * in.y;
	temp.z = in.z * in.z;
	float sum = temp.x + temp.y + temp.z;
	float mag = store_s(_mm_sqrt_ss(load_s(sum)));
	bool is_zero = sum == 0.0f;
	//mag = blend(0.0f, mag, is_zero);
	mag = is_zero ? 0.0f : mag;
	return mag;
}


/*
==================
==================
*/
__m128 Vector_Normalise(const __m128 in) {

	__m128i zero_mask = (in != set_zero());
	__m128 temp[4];
	temp[X] = in * in;										// v squared
	Transpose(temp);
	temp[W] = temp[X] + temp[Y] + temp[Z];			// sum
	temp[W] = _mm_rsqrt_ps(temp[W]);					// magnitude = 1.0f / sqrt(sum)
	temp[W] = broadcast(temp[W]);	// broadcast
	return in * (temp[W] & zero_mask);
}

/*
==================
==================
*/
__m128 Vector_Normalise_Magnitude(const __m128 in) {
	
	__m128 temp[4];
	for (__int32 i = 0; i < 4; i++){
		temp[i] = in;
	}
	Transpose(temp);
	temp[W] = set_zero();
	for (__int32 i = X; i < W; i++){
		temp[W] += temp[i] * temp[i];
	}
	temp[W] = _mm_sqrt_ps(temp[W]);
	__m128 r_magnitude = reciprocal(temp[W]);
	for (__int32 i = X; i < W; i++){
		__m128i is_not_zero = (temp[i] != set_zero());
		temp[i] *= r_magnitude;
		temp[i] &= is_not_zero;
	}
	Transpose(temp);
	return temp[X];
}

/*
==================
==================
*/
__m128 Vector_Normalise_4D(const __m128 in) {

	__m128 temp[4];
	for (__int32 i = 0; i < 4; i++){
		temp[i] = in;
	}
	Transpose(temp);
	__m128 sum = set_zero();
	for (__int32 i = X; i <= W; i++){
		sum += temp[i] * temp[i];
	}
	__m128 zero = _mm_setzero_ps();
	__m128i zero_mask = sum != zero;
	__m128 r_magnitude = _mm_rsqrt_ps(sum) & zero_mask;
	for (__int32 i = X; i <= W; i++){
		temp[i] *= r_magnitude;
	}
	Transpose(temp);
	return temp[X];
}

/*
==================
==================
*/
void Normalise_Quaternion(__m128 in[4]) {

	Transpose(in);
	__m128 sum = set_zero();
	for (__int32 i = X; i <= W; i++){
		sum += in[i] * in[i];
	}
	__m128 zero = _mm_setzero_ps();
	__m128i zero_mask = sum != zero;
	__m128 r_magnitude = _mm_rsqrt_ps(sum) & zero_mask;
	for (__int32 i = X; i <= W; i++){
		in[i] *= r_magnitude;
	}
	Transpose(in);
}


/*
==================
==================
*/
void Normalise_Quaternion(float4_& in) {

	float sum = 0.0f;
	for (__int32 i = X; i <= W; i++) {
		sum += in.f[i] * in.f[i];
	}
	bool is_zero = sum == 0.0f;
	float r_magnitude = store_s(_mm_rsqrt_ss(load_s(sum)));
	//r_magnitude = blend(0.0f, r_magnitude, is_zero);
	r_magnitude = is_zero ? 0.0f : r_magnitude;
	for (__int32 i = X; i <= W; i++) {
		in.f[i] *= r_magnitude;
	}
}

/*
==================
==================
*/
void Normalise_Quaternion(const float4_& in, float4_& out) {

	float sum = 0.0f;
	for (__int32 i = X; i <= W; i++) {
		sum += in.f[i] * in.f[i];
	}
	bool is_zero = sum == 0.0f;
	float r_magnitude = store_s(_mm_rsqrt_ss(load_s(sum)));
	//r_magnitude = blend(0.0f, r_magnitude, is_zero);
	r_magnitude = is_zero ? 0.0f : r_magnitude;
	for (__int32 i = X; i <= W; i++) {
		out.f[i] = in.f[i] * r_magnitude;
	}
}

/*
==================
==================
*/
void Normalise_Quaternion(const __m128 in[4], __m128 out[4]) {

	Transpose(in, out);
	__m128 sum = set_zero();
	for (__int32 i = X; i <= W; i++){
		sum += out[i] * out[i];
	}
	__m128 zero = _mm_setzero_ps();
	__m128i zero_mask = sum != zero;
	__m128 r_magnitude = _mm_rsqrt_ps(sum) & zero_mask;
	for (__int32 i = X; i <= W; i++){
		out[i] *= r_magnitude;
	}
	Transpose(out);
}


/*
==================
==================
*/
void Matrix_To_Quaternion(const matrix_& a, float4_& b) {

	//float temp = 1.0f + a[X][X] + a[Y][Y] + a[Z][Z];
	//bool is_zero = temp == 0.0f;
	//b[W] = store_s(_mm_sqrt_ss(load_s(temp))) * 0.5;
	//b[X] = (a[Z][Y] - a[Y][Z]) / (4 * b[W]);
	//b[Y] = (a[X][Z] - a[Z][X]) / (4 * b[W]);
	//b[Z] = (a[Y][X] - a[X][Y]) / (4 * b[W]);

	//b[X] = blend(0.0f, b[X], is_zero);
	//b[Y] = blend(0.0f, b[Y], is_zero);
	//b[Z] = blend(0.0f, b[Z], is_zero);
	//b[W] = blend(1.0f, b[W], is_zero);

	//----------------------------------------------------------------------

	if ((a[X].x + a[Y].y + a[Z].z) > 0.0f) {

		float trace = a[X].x + a[Y].y + a[Z].z + 1.0f;
		float s = store_s(_mm_sqrt_ss(load_s(trace))) * 0.5f;
		b.w = trace * s;
		b.z = (a[X].y - a[Y].x) * s;
		b.y = (a[Z].x - a[X].z) * s;
		b.x = (a[Y].z - a[Z].y) * s;
	}
	else if ((a[X].x > a[Y].y) && (a[X].x > a[Z].z)) {

		float trace = a[X].x - a[Y].y - a[Z].z + 1.0f;
		float s = store_s(_mm_sqrt_ss(load_s(trace))) * 0.5f;
		b.x = trace * s;
		b.y = (a[X].y + a[Y].x) * s;
		b.z = (a[Z].x + a[X].z) * s;
		b.w = (a[Y].z - a[Z].y) * s;
	}
	else if (a[Y].y > a[Z].z) {

		float trace = - a[X].x + a[Y].y - a[Z].z + 1.0f;
		float s = store_s(_mm_sqrt_ss(load_s(trace))) * 0.5f;
		b.y = trace * s;
		b.x = (a[X].y + a[Y].x) * s;
		b.w = (a[Z].x - a[X].z) * s;
		b.z = (a[Y].z + a[Z].y) * s;
	}
	else {

		float trace = -a[X].x - a[Y].y + a[Z].z + 1.0f;
		float s = store_s(_mm_sqrt_ss(load_s(trace))) * 0.5f;
		b.z = trace * s;
		b.w = (a[X].y - a[Y].x) * s;
		b.x = (a[Z].x + a[X].z) * s;
		b.y = (a[Y].z + a[Z].y) * s;
	}							  
}

/*
==================
==================
*/
void Cross_Product(const float3_& a, const float3_& b, float3_& out) {

	out.x = (a.y * b.z) - (a.z * b.y);
	out.y = (a.z * b.x) - (a.x * b.z);
	out.z = (a.x * b.y) - (a.y * b.x);
}


/*
==================
==================
*/
float Dot_Product_4D(const float4_& a, const float4_& b) {

	float result = 0.0f;
	result += (a.x * b.x);
	result += (a.y * b.y);
	result += (a.z * b.z);
	result += (a.w * b.w);
	return result;
}

/*
==================
==================
*/
float Dot_Product(const float3_& a, const float3_& b) {

	float result = 0.0f;
	result += (a.x * b.x);
	result += (a.y * b.y);
	result += (a.z * b.z);
	return result;
}


/*
==================
==================
*/
__m128 Cross_Product_3D( const __m128 av, const __m128 bv ) {			

	__m128 a[4];
	__m128 b[4];
	for (__int32 i = 0; i < 4; i++) {
		a[i] = av;
		b[i] = bv;
	}
	Transpose(a);
	Transpose(b);
	__m128 c[4];
	c[X] = (a[Y] * b[Z]) - (a[Z] * b[Y]);
	c[Y] = (a[Z] * b[X]) - (a[X] * b[Z]);
	c[Z] = (a[X] * b[Y]) - (a[Y] * b[X]);
	c[W] = set_zero();
	Transpose(c);
	return c[0];
}

/*
==================
==================
*/
__m128 Normalise_Axis_Aligned(const __m128 in) {

	__m128i sign_bit = broadcast(load_s(0x80000000));
	__m128i not_sign_bit = broadcast(load_s(~0x80000000));
	__m128 saved_sign_bit = in & sign_bit;
	__m128 out[4];
	for (__int32 i = 0; i < 4; i++) {
		out[i] = in & not_sign_bit;
	}
	Transpose(out);
	__m128 max = out[0];
	for (__int32 i = 0; i < 4; i++) {
		max = max_vec(out[i], max);
	}
	__m128 one = set_all(1.0f);
	for (__int32 i = 0; i < 4; i++) {
		__m128i result = (max == out[i]);
		out[i] = blend(one, out[i], result);
		out[i] &= result;
	}
	Transpose(out);
	__m128i result = out[0] > set_zero();			// remove any -0.0f
	return out[0] | (saved_sign_bit & result);
}

/*
==================
==================
*/
void Build_Transform_Matrix(const __m128 axis_angle, __m128 matrix_in[4]) {

	__m128 quaternion_m[4];
	quaternion_m[X] = Axis_Angle_To_Quaternion(axis_angle);
	__m128 out[4][4];
	Quaternion_To_Matrix(quaternion_m, out);

	for (__int32 i = 0; i < 4; i++) {
		matrix_in[i] = out[0][i];
	}
}

/*
==================
==================
*/
__m128 Axis_Angle_To_Quaternion(__m128 const &axis_angle){

	float angle = store_s(_mm_shuffle_ps(axis_angle, axis_angle, _MM_SHUFFLE(W, W, W, W)));
	float a = angle * RADIANS_PER_DEGREE * 0.5f;
	float s = sin(a);
	float c = cos(a);

	__m128 b4 = broadcast(load_s(s));
	__m128 c4 = broadcast(load_s(c));
	__m128 temp = blend(b4, c4, XYZ_Mask);
	__m128 axis = blend(axis_angle, set_all(1.0f), XYZ_Mask);
	return temp * axis;
}

/*
==================
==================
*/
void Axis_Angle_To_Quaternion(float4_ const& in, float4_& out) {

	float a = in.w * RADIANS_PER_DEGREE * 0.5f;
	float s = sin(a);

	out.x = in.x * s;
	out.y = in.y * s;
	out.z = in.z * s;
	out.w = cos(a);
}

/*
==================
==================
*/
__m128 Axis_Angle_To_Quaternion_Radians(const __m128 axis_angle){

	float angle = store_s(_mm_shuffle_ps(axis_angle, axis_angle, _MM_SHUFFLE(W, W, W, W)));
	float a = angle * 0.5f;
	float s = sin(a);
	float c = cos(a);

	__m128 b4 = broadcast(load_s(s));
	__m128 c4 = broadcast(load_s(c));
	__m128 temp = blend(b4, c4, XYZ_Mask);
	__m128 axis = blend(axis_angle, set_all(1.0f), XYZ_Mask);
	return temp * axis;
}

/*
==================
==================
*/
void Quaternion_To_Matrix(const float4_& in, matrix_& out) {

	float x = in.x;
	float y = in.y;
	float z = in.z;
	float w = in.w;

	out[X].x = 1.0f - (2.0f * y * y) - (2.0f * z * z);
	out[X].y = (2.0f * x * y) + (2.0f * w * z);
	out[X].z = (2.0f * x * z) - (2.0f * w * y);
	out[X].w = 0.0f;
		  
	out[Y].x = (2.0f * x * y) - (2.0f * w * z);
	out[Y].y = 1.0f - (2.0f * x * x) - (2.0f * z * z);
	out[Y].z = (2.0f * y * z) + (2.0f * w * x);
	out[Y].w = 0.0f;
		  
	out[Z].x = (2.0f * x * z) + (2.0f * w * y);
	out[Z].y = (2.0f * y * z) - (2.0f * w * x);
	out[Z].z = 1.0f - (2.0f * x * x) - (2.0f * y * y);
	out[Z].w = 0.0f;
		  
	out[W].x = 0.0f;
	out[W].y = 0.0f;
	out[W].z = 0.0f;
	out[W].w = 1.0f;
}

/*
==================
==================
*/
void Quaternion_To_Matrix(const __m128 in[4], __m128 out[4][4]){

	//m[0][X] = 1.0f - (2.0f * y * y) - (2.0f * z * z);
	//m[0][Y] = (2.0f * x * y) + (2.0f * w * z);
	//m[0][Z] = (2.0f * x * z) - (2.0f * w * y);
	//m[0][W] = 0.0f;

	//m[1][X] = (2.0f * x * y) - (2.0f * w * z);
	//m[1][Y] = 1.0f - (2.0f * x * x) - (2.0f * z * z);
	//m[1][Z] = (2.0f * y * z) + (2.0f * w * x);
	//m[1][W] = 0.0f;

	//m[2][X] = (2.0f * x * z) + (2.0f * w * y);
	//m[2][Y] = (2.0f * y * z) - (2.0f * w * x);
	//m[2][Z] = 1.0f - (2.0f * x * x) - (2.0f * y * y);
	//m[2][W] = 0.0f;

	// 4 quaternions input, 4 matrices output;

	__m128 zero = set_zero();
	__m128 one = set_all(1.0f);

	__m128 q[4];
	Transpose(in, q);
	__m128 temp[4][4];

	__m128 x2 = q[X] + q[X];
	__m128 y2 = q[Y] + q[Y];
	__m128 z2 = q[Z] + q[Z];
	{
		__m128 xx2 = q[X] * x2;
		__m128 yy2 = q[Y] * y2;
		__m128 zz2 = q[Z] * z2;
		temp[0][X] = one - yy2 - zz2;
		temp[1][Y] = one - xx2 - zz2;
		temp[2][Z] = one - xx2 - yy2;
	}
	{
		__m128 yz2 = q[Y] * z2;
		__m128 wx2 = q[W] * x2;
		temp[2][Y] = yz2 - wx2;
		temp[1][Z] = yz2 + wx2;
	}
	{
		__m128 xy2 = q[X] * y2;
		__m128 wz2 = q[W] * z2;
		temp[1][X] = xy2 - wz2;
		temp[0][Y] = xy2 + wz2;
	}
	{
		__m128 xz2 = q[X] * z2;
		__m128 wy2 = q[W] * y2;
		temp[0][Z] = xz2 - wy2;
		temp[2][X] = xz2 + wy2;
	}

	for (__int32 i = 0; i < 4; i++) {
		temp[i][W] = zero;
		temp[3][i] = zero;
	}
	temp[3][W] = one;

	for (__int32 i_axis = X; i_axis <= W; i_axis++) {
		Transpose(temp[i_axis]);
		for (__int32 i = 0; i < 4; i++) {
			out[i][i_axis] = temp[i_axis][i];
		}
	}
}

/*
==================
==================
*/
void Quaternion_X_Quaternion(const __m128 a[4], const __m128 b[4], __m128 result[4]){

	__m128 q0[4];
	__m128 q1[4];
	Transpose(a, q0);
	Transpose(b, q1);

	result[X] = (q0[W] * q1[X]) + (q0[X] * q1[W]) + (q0[Y] * q1[Z]) - (q0[Z] * q1[Y]);
	result[Y] = (q0[W] * q1[Y]) + (q0[Y] * q1[W]) + (q0[X] * q1[Z]) - (q0[Z] * q1[X]);
	result[Z] = (q0[W] * q1[Z]) + (q0[Z] * q1[W]) + (q0[X] * q1[Y]) - (q0[Y] * q1[X]);
	result[W] = (q0[W] * q1[W]) - (q0[X] * q1[X]) - (q0[Y] * q1[Y]) - (q0[Z] * q1[Z]);

	Transpose(result);
}

/*
==================
==================
*/
void Quaternion_X_Quaternion(const float4_& a, const float4_& b, float4_& result) {

	float4_ temp;
	temp.x = (a.w * b.x) + (a.x * b.w) + (a.y * b.z) - (a.z * b.y);
	temp.y = (a.w * b.y) + (a.y * b.w) + (a.x * b.z) - (a.z * b.x);
	temp.z = (a.w * b.z) + (a.z * b.w) + (a.x * b.y) - (a.y * b.x);
	temp.w = (a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z);
	result = temp;
}

/*
==================
==================
*/
float Smooth_Step(float edge0, float edge1, float t) {

	t = (t - edge0) / (edge1 - edge0);
	t = max(t, 0.0f);
	t = min(t, 1.0f);
	return t*t*(3 - 2 * t);
}

/*
==================
==================
*/
void Quaternion_To_Matrix_SINGLE(__m128 const &quaternion, __m128 matrix[4]){

	//m[0][X] = 1.0f - (2.0f * y * y) - (2.0f * z * z);
	//m[0][Y] = (2.0f * x * y) + (2.0f * w * z);
	//m[0][Z] = (2.0f * x * z) - (2.0f * w * y);
	//m[0][W] = 0.0f;

	//m[1][X] = (2.0f * x * y) - (2.0f * w * z);
	//m[1][Y] = 1.0f - (2.0f * x * x) - (2.0f * z * z);
	//m[1][Z] = (2.0f * y * z) + (2.0f * w * x);
	//m[1][W] = 0.0f;

	//m[2][X] = (2.0f * x * z) + (2.0f * w * y);
	//m[2][Y] = (2.0f * y * z) - (2.0f * w * x);
	//m[2][Z] = 1.0f - (2.0f * x * x) - (2.0f * y * y);
	//m[2][W] = 0.0f;

	__m128 one = set_all(1.0f);
	__m128 two = one + one;
	__m128 x_y_z_w = quaternion;

	//m[0][X] = 1.0f - (2.0f * y * y) - (2.0f * z * z);
	//m[0][Y] = 1.0f - (2.0f * x * x) - (2.0f * z * z);
	//m[0][Z] = 1.0f - (2.0f * x * x) - (2.0f * y * y);
	//m[0][W] = 0.0f;

	__m128 y_x_x_w = _mm_shuffle_ps(x_y_z_w, x_y_z_w, _MM_SHUFFLE(W, X, X, Y));
	__m128 z_z_y_w = _mm_shuffle_ps(x_y_z_w, x_y_z_w, _MM_SHUFFLE(W, Y, Z, Z));

	matrix[X] = one - (two * y_x_x_w * y_x_x_w) - (two * z_z_y_w * z_z_y_w);
	matrix[X] &= XYZ_Mask;

	//m[1][X] = (2.0f * x * y) + (2.0f * w * z);
	//m[1][Y] = (2.0f * y * z) + (2.0f * w * x);
	//m[1][Z] = (2.0f * x * z) + (2.0f * w * y);
	//m[1][W] = 0.0f;

	__m128 x_y_x_w = _mm_shuffle_ps(x_y_z_w, x_y_z_w, _MM_SHUFFLE(W, X, Y, X));
	__m128 y_z_z_w = _mm_shuffle_ps(x_y_z_w, x_y_z_w, _MM_SHUFFLE(W, Z, Z, Y));
	__m128 z_x_y_w = _mm_shuffle_ps(x_y_z_w, x_y_z_w, _MM_SHUFFLE(W, Y, X, Z));
	__m128 w_w_w_w = _mm_shuffle_ps(x_y_z_w, x_y_z_w, _MM_SHUFFLE(W, W, W, W));

	matrix[Y] = (two * x_y_x_w * y_z_z_w) + (two * z_x_y_w * w_w_w_w);
	matrix[Y] &= XYZ_Mask;

	//m[2][X] = (2.0f * x * z) - (2.0f * w * y);
	//m[2][Y] = (2.0f * x * y) - (2.0f * w * z);
	//m[2][Z] = (2.0f * y * z) - (2.0f * w * x);
	//m[2][W] = 0.0f;

	__m128 x_x_y_w = _mm_shuffle_ps(x_y_z_w, x_y_z_w, _MM_SHUFFLE(W, Y, X, X));
	__m128 z_y_z_w = _mm_shuffle_ps(x_y_z_w, x_y_z_w, _MM_SHUFFLE(W, Z, Y, Z));
	__m128 y_z_x_w = _mm_shuffle_ps(x_y_z_w, x_y_z_w, _MM_SHUFFLE(W, X, Z, Y));

	matrix[Z] = (two * x_x_y_w * z_y_z_w) - (two * y_z_x_w * w_w_w_w);
	matrix[Z] &= XYZ_Mask;

	// a0 a1 a2 00				// a0 b0 c0 00						// a0 b0 c0 00
	// b0 b1 b2 00	transpose 	// a1 b1 c1 00	align diagonals		// c1 a1 b1 00	
	// c0 c1 c2 00	------> 	// a2 b2 c2 00	------------>		// b2 c2 a2 00
	// 00 00 00 01				// 00 00 00 01						// 00 00 00 01	

	matrix[W] = set_all(1.0f) & ~XYZ_Mask;
	Transpose(matrix);

	matrix[Y] = _mm_shuffle_ps(matrix[Y], matrix[Y], _MM_SHUFFLE(W, Y, X, Z));
	matrix[Z] = _mm_shuffle_ps(matrix[Z], matrix[Z], _MM_SHUFFLE(W, X, Z, Y));
}






