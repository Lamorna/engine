#pragma once



//#define far
//#define near
//#define max(a, b)  (((a) > (b)) ? (a) : (b)) 
//#define min(a, b)  (((a) < (b)) ? (a) : (b)) 

//#include "windows_test.h"
//#undef NEAR
//#undef FAR

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRA_LEAN


#include <windows.h>
#include <mmreg.h>
#include <intrin.h>
#include <cstdio>
#include <cmath>
#include <D3D9.h>
#include <DSound.h>

#undef NDEBUG

#include <assert.h>

struct parameters_ {

	struct entity_;
	struct player_;
	struct camera_;
	struct environment_;
	struct monster_;
	struct collide_;
	struct sound_;
	struct item_;
	struct command_;
	struct render_;
	struct lightmap_;

};

struct systems_ {

	struct player_;
	struct monster_;
	struct entity_;
	struct camera_;
	struct environment_;
	struct collide_;
	struct collision_response_;
	struct sound_;
	struct item_;
	struct command_;
	struct render_;
	struct lightmap_;
};




