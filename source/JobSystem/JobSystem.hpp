#pragma once

#include <functional>

// A Dispatched job will receive this as function argument
struct JobDispatchArgs
{
	uint32_t mJobIndex;
	uint32_t mGroupIndex;
};

namespace JobSystem
{
	// Creates the internal resources including worker threads. Called once when initializing the application
	void initialise();

	// Add a job to execute asynchronously. Any idle thread will execute this job
	void execute(const std::function<void()>& pJob);

	// Divide a job into multiple jobs and execute in parallel
	// pJobCount - how many jobs to generate for this task
	// pGroupSize - how many jobs to execute per thread. Jobs inside a group execute serially. It might be worth to increase for small jobs
	// pJob - receives a JobDispatchArgs as parameter
	void dispatch(uint32_t pJobCount, uint32_t pGroupSize, const std::function<void(JobDispatchArgs)>& pJob);

	// Check if any threads are working currently
	bool isBusy();

	// Wait until all threads become idle
	void wait();
}