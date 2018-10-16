
#pragma once

#include "master.h"

//======================================================================

typedef void(*function_pointer_)(void*, __int32);

//======================================================================

struct thread_pool_;

struct job_ {

	enum {
		MAX_PERMISSIONs = 8,
	};

	__int32 id;
	__int32 type;
	__int32 n_dependencies;
	__int32 n_permisions;
	__int32 i_permissions[MAX_PERMISSIONs][2];
	void* parameters;
	function_pointer_ function;
};

struct worker_parameters_ {

	__int32 i_thread;
	thread_pool_* thread_pool;
};

struct job_queue_ {

	enum {

		NUM_JOBS_BATCH = 16,
		BATCH_MODULO = NUM_JOBS_BATCH - 1,
	};

	__int32 n_jobs;
	unsigned __int32 i_write;
	unsigned __int32 i_read;
	job_ jobs[NUM_JOBS_BATCH];
};

struct jobs_complete_ {

	enum {
		MAX_JOBS = job_queue_::NUM_JOBS_BATCH * 2,
	};

	__int32 n_jobs;
	__int32 ids[MAX_JOBS];
};

struct jobs_ready_ {

	__int32 n_jobs;
	unsigned __int32 i_write;
	unsigned __int32 i_read;
	__int32 ids[job_queue_::NUM_JOBS_BATCH];
};

struct master_thread_ {

	jobs_complete_ jobs_complete;
	jobs_ready_ jobs_ready;
};

struct thread_pool_ {

	enum {

		MAX_WORKER_THREADS = 8,
	};

	bool is_running;
	__int32 n_threads;
	worker_parameters_ worker_parameters[MAX_WORKER_THREADS];
	HANDLE  handles[MAX_WORKER_THREADS];
	CONDITION_VARIABLE consumer_variable;
	CONDITION_VARIABLE producer_variable;
	CRITICAL_SECTION  critical_section;

	__int32 n_jobs_finished;
	job_queue_ job_queue;
	jobs_complete_ jobs_complete;

};

//======================================================================

void Thread_Pool_Initialise(thread_pool_&);

void Thread_Pool_Submit_Jobs(

	const __int32,
	job_[],
	thread_pool_&

);

void Thread_Pool_Shutdown(thread_pool_&);



