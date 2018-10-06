
#pragma once

#include "vector.h"


//==============================================================================================

__m128 Pack_Vector4(const __m128&, const unsigned __int32);
__m128i Pack_Vector4(const __m128i&, const unsigned __int32);
__m128 Vector_Normalise(const __m128&);
__m128 Vector_Normalise_Magnitude(const __m128&);
__m128 Vector_Normalise_4D(const __m128&);

void Vector_Normalise(float3_&);

void Vector_Normalise(const float3_&, float3_&);
float  Vector_Magnitude(const float3_&);
float  Vector_Normalise_Magnitude(const float3_&, float3_&);
void Cross_Product(const float3_&, const float3_&, float3_&);
void Matrix_To_Quaternion(const matrix_&, float4_&);
float Dot_Product(const float3_&, const float3_&);
float Dot_Product_4D(const float4_&, const float4_&);
void Quaternion_X_Quaternion(const float4_&, const float4_&, float4_&);
void Axis_Angle_To_Quaternion(float4_ const&, float4_&);
void Normalise_Quaternion(float4_&);
void Normalise_Quaternion(const float4_&, float4_&);
void Quaternion_To_Matrix(const float4_&, matrix_&);



__m128 Cross_Product_3D(const __m128&, const __m128&);
__m128 Normalise_Axis_Aligned(const __m128&);
void Build_Transform_Matrix(const __m128&, matrix);

__m128 Axis_Angle_To_Quaternion(__m128 const&);
__m128 Axis_Angle_To_Quaternion_Radians(__m128 const&);

void Quaternion_To_Matrix(const quaternion[4], matrix[4]);

void Quaternion_X_Quaternion(const matrix, const matrix, matrix);

void Normalise_Quaternion(matrix);

void Normalise_Quaternion(const matrix, matrix);

float Smooth_Step(float, float, float);


//==============================================================================================

