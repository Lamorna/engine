#include "threads.h"

DWORD WINAPI Main_Loop(LPVOID void_paramaters);

/*
==================
==================
*/
void Null_Work_Function(void* parameters, __int32 i_thread){

	printf_s("ERROR: NULL WORK FUNCTION CALLED");
}

/*
==================
==================
*/
void Thread_Pool_Initialise(thread_pool_& thread_pool) {

	thread_pool.is_running = true;

	InitializeConditionVariable(&thread_pool.consumer_variable);
	InitializeConditionVariable(&thread_pool.producer_variable);
	InitializeCriticalSection(&thread_pool.critical_section);

	{
		thread_pool.job_queue.i_read = 0;
		thread_pool.job_queue.i_write = 0;
		thread_pool.job_queue.n_jobs = 0;
		thread_pool.n_jobs_finished = 0;
		thread_pool.jobs_complete.n_jobs = 0;

		for (__int32 i_job = 0; i_job < job_queue_::NUM_JOBS_BATCH; i_job++) {

			thread_pool.job_queue.jobs[i_job].function = &Null_Work_Function;
			thread_pool.job_queue.jobs[i_job].parameters = NULL;
		}
	}

	for (__int32 i_thread = 0; i_thread < thread_pool.n_threads; i_thread++) {

		thread_pool.worker_parameters[i_thread].i_thread = i_thread;
		thread_pool.worker_parameters[i_thread].thread_pool = &thread_pool;
		thread_pool.handles[i_thread] = CreateThread(NULL, 0, Main_Loop, &thread_pool.worker_parameters[i_thread], 0, 0);
	}
}

/*
==================
==================
*/
DWORD WINAPI Main_Loop(LPVOID void_paramaters) {

	const struct worker_parameters_* parameters = (struct worker_parameters_*)void_paramaters;

	const __int32 i_thread = parameters->i_thread;
	thread_pool_& thread_pool = *parameters->thread_pool;
	
	__int32 job_id = -1;
	__int32 increment = 0;

	while (true) {

		function_pointer_ function_pointer = NULL;
		void* function_parameters = NULL;

		EnterCriticalSection(&thread_pool.critical_section);

		thread_pool.jobs_complete.ids[thread_pool.jobs_complete.n_jobs] = job_id;
		thread_pool.jobs_complete.n_jobs += increment;
		thread_pool.n_jobs_finished += increment;
		increment = 1;

		WakeAllConditionVariable(&thread_pool.producer_variable);

		while ((thread_pool.job_queue.n_jobs == 0) && thread_pool.is_running) {

			SleepConditionVariableCS(&thread_pool.consumer_variable, &thread_pool.critical_section, INFINITE);
		}

		if (thread_pool.is_running == false) {

			LeaveCriticalSection(&thread_pool.critical_section);
			return 0;
		}

		job_id = thread_pool.job_queue.jobs[thread_pool.job_queue.i_read].id;
		function_pointer = thread_pool.job_queue.jobs[thread_pool.job_queue.i_read].function;
		function_parameters = thread_pool.job_queue.jobs[thread_pool.job_queue.i_read].parameters;
		thread_pool.job_queue.n_jobs--;
		thread_pool.job_queue.i_read = (thread_pool.job_queue.i_read + 1) & job_queue_::BATCH_MODULO;

		LeaveCriticalSection(&thread_pool.critical_section);

		function_pointer(function_parameters, i_thread);
	}

	return 0;
}

/*
==================
==================
*/
void Thread_Pool_Submit_Jobs(

	const __int32 n_jobs,
	job_ jobs_in[],
	thread_pool_& thread_pool

) {

	master_thread_ master_thread;
	master_thread.jobs_complete.n_jobs = 0;
	master_thread.jobs_ready.n_jobs = 0;
	master_thread.jobs_ready.i_read = 0;
	master_thread.jobs_ready.i_write = 0;

	bool is_jobs_remaining = true;

	thread_pool.n_jobs_finished = 0;

	// -------------------------------------------------------------------------------
	{
		for (__int32 i_job = 0; i_job < n_jobs; i_job++) {

			jobs_in[i_job].n_dependencies = 0;
		}

		for (__int32 i_job = 0; i_job < n_jobs; i_job++) {

			for (__int32 i_permission = 0; i_permission < jobs_in[i_job].n_permisions; i_permission++) {

				const __int32 i_first = jobs_in[i_job].i_permissions[i_permission][0];
				const __int32 i_last = i_first + jobs_in[i_job].i_permissions[i_permission][1];

				for (__int32 i_job_permit = i_first; i_job_permit < i_last; i_job_permit++) {

					jobs_in[i_job_permit].n_dependencies++;
				}
			}
		}
	}
	// -------------------------------------------------------------------------------

	while (is_jobs_remaining) {

		{
			const __int32 n_jobs = master_thread.jobs_complete.n_jobs;
			for (__int32 i_job = 0; i_job < n_jobs; i_job++) {

				__int32 job_id = master_thread.jobs_complete.ids[i_job];

				for (__int32 i_permission = 0; i_permission < jobs_in[job_id].n_permisions; i_permission++) {

					const __int32 i_first = jobs_in[job_id].i_permissions[i_permission][0];
					const __int32 i_last = i_first + jobs_in[job_id].i_permissions[i_permission][1];

					for (__int32 i_job_permit = i_first; i_job_permit < i_last; i_job_permit++) {
						jobs_in[i_job_permit].n_dependencies--;
					}
				}
				master_thread.jobs_complete.n_jobs--;
			}
		}
		{
			__int32 i_job = 0;
			while ((master_thread.jobs_ready.n_jobs < job_queue_::NUM_JOBS_BATCH) && (i_job < n_jobs)) {

				if (jobs_in[i_job].n_dependencies == 0) {

					master_thread.jobs_ready.ids[master_thread.jobs_ready.i_write] = jobs_in[i_job].id;
					master_thread.jobs_ready.i_write = (master_thread.jobs_ready.i_write + 1) & job_queue_::BATCH_MODULO;
					master_thread.jobs_ready.n_jobs++;
					jobs_in[i_job].n_dependencies--;
				}
				i_job++;
			}
		}

		EnterCriticalSection(&thread_pool.critical_section);

		{
			const __int32 n_jobs = min(master_thread.jobs_ready.n_jobs, job_queue_::NUM_JOBS_BATCH - thread_pool.job_queue.n_jobs);

			for (__int32 i_job = 0; i_job < n_jobs; i_job++) {

				const __int32 job_id = master_thread.jobs_ready.ids[master_thread.jobs_ready.i_read];
				thread_pool.job_queue.jobs[thread_pool.job_queue.i_write].id = job_id;
				thread_pool.job_queue.jobs[thread_pool.job_queue.i_write].function = jobs_in[job_id].function;
				thread_pool.job_queue.jobs[thread_pool.job_queue.i_write].parameters = jobs_in[job_id].parameters;
				thread_pool.job_queue.i_write = (thread_pool.job_queue.i_write + 1) & job_queue_::BATCH_MODULO;
				thread_pool.job_queue.n_jobs++;
				master_thread.jobs_ready.i_read = (master_thread.jobs_ready.i_read + 1) & job_queue_::BATCH_MODULO;
				master_thread.jobs_ready.n_jobs--;
			}
		}

		WakeAllConditionVariable(&thread_pool.consumer_variable);

		while ((thread_pool.jobs_complete.n_jobs == 0) && thread_pool.is_running) {

			SleepConditionVariableCS(&thread_pool.producer_variable, &thread_pool.critical_section, INFINITE);
		}

		{
			const __int32 n_jobs = thread_pool.jobs_complete.n_jobs;
			for (__int32 i_job = 0; i_job < n_jobs; i_job++) {

				master_thread.jobs_complete.ids[i_job] = thread_pool.jobs_complete.ids[i_job];
				master_thread.jobs_complete.n_jobs++;
				thread_pool.jobs_complete.n_jobs--;
			}
		}

		is_jobs_remaining = thread_pool.n_jobs_finished < n_jobs;

		LeaveCriticalSection(&thread_pool.critical_section);
	}
}

/*
==================
==================
*/
void Thread_Pool_Shutdown(thread_pool_& thread_pool) {

	EnterCriticalSection(&thread_pool.critical_section);
	{
		thread_pool.is_running = false;
	}
	LeaveCriticalSection(&thread_pool.critical_section);
	WakeAllConditionVariable(&thread_pool.consumer_variable);
	WaitForMultipleObjects(thread_pool.n_threads, thread_pool.handles, TRUE, INFINITE);
}
