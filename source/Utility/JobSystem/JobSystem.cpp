#include "JobSystem.hpp"
#include <algorithm>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <sstream>
#include <assert.h>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

#include "Logger.hpp"

// Fixed size very simple thread safe ring buffer
template <typename T, size_t capacity>
class ThreadSafeRingBuffer
{
public:
	// Push an item to the end if there is free space
	// Returns true if succesful, returns false if there is not enough space
	inline bool push_back(const T &item)
	{
		bool result = false;
		mBufferLock.lock();
		size_t next = (mHead + 1) % capacity;
		if (next != mTail)
		{
			mData[mHead] = item;
			mHead = next;
			result = true;
		}
		mBufferLock.unlock();
		return result;
	}

	// Get an item if there are any
	// Returns true if succesful, returns false if there are no items
	inline bool pop_front(T &item)
	{
		bool result = false;
		mBufferLock.lock();
		if (mTail != mHead)
		{
			item = mData[mTail];
			mTail = (mTail + 1) % capacity;
			result = true;
		}
		mBufferLock.unlock();
		return result;
	}

private:
	T mData[capacity];
	size_t mHead = 0;
	size_t mTail = 0;
	std::mutex mBufferLock; // this just works better than a spinlock here (on windows)
};

namespace JobSystem
{
	uint32_t mThreads = 0;								  		// number of worker threads, it will be initialized in the Initialize() function
	uint64_t mCurrentLabel = 0;								  	// tracks the state of execution of the main thread
	ThreadSafeRingBuffer<std::function<void()>, 256> mJobPool; 	// a thread safe queue to put pending jobs onto the end (with a capacity of 256 jobs). A worker thread can grab a job from the beginning
	std::condition_variable mWakeCondition;					  	// used in conjunction with the WakeMutex below. Worker threads just sleep when there is no job, and the main thread can wake them up
	std::mutex mWakeMutex;									  	// used in conjunction with the WakeCondition above
	std::atomic<uint64_t> mFinishedLabel;					  	// track the state of execution across background worker threads

	void initialise()
	{
		// Initialize the worker execution state to 0:
		mFinishedLabel.store(0);

		auto numCores = std::thread::hardware_concurrency(); // Number of hardware threads in this system
		mThreads = std::max(1u, numCores); // Actual number of worker threads we want
		LOG("Job System found {} available cores and {} threads for hardware", numCores, mThreads);

		// Create all our worker threads while immediately starting them:
		for (uint32_t threadID = 0; threadID < mThreads; ++threadID)
		{
			std::thread worker([]
							   {
								   std::function<void()> job; // the current job for the thread, it's empty at start

								   // This is the infinite loop that a worker thread will do
								   while (true)
								   {
									   if (mJobPool.pop_front(job)) // try to grab a job from the JobPool queue
									   {
										   // It found a job, execute it:
										   job();					   // execute job
										   mFinishedLabel.fetch_add(1); // update worker label state
									   }
									   else
									   {
										   // no job, put thread to sleep
										   std::unique_lock<std::mutex> lock(mWakeMutex);
										   mWakeCondition.wait(lock);
									   }
								   }
							   });

#ifdef _WIN32
			// Do Windows-specific thread setup:
			HANDLE handle = (HANDLE)worker.native_handle();

			// Put each thread on to dedicated core
			DWORD_PTR affinityMask = 1ull << threadID;
			DWORD_PTR affinity_result = SetThreadAffinityMask(handle, affinityMask);
			assert(affinity_result > 0);

			//// Increase thread priority:
			//BOOL priority_result = SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);
			//assert(priority_result != 0);

			// Name the thread:
			std::wstringstream wss;
			wss << "JobSystem_" << threadID;
			HRESULT hr = SetThreadDescription(handle, wss.str().c_str());
			assert(SUCCEEDED(hr));
#endif // _WIN32

			worker.detach(); // forget about this thread, let it do it's job in the infinite loop that we created above
		}
	}

	// This little helper function will not let the system to be deadlocked while the main thread is waiting for something
	inline void poll()
	{
		mWakeCondition.notify_one(); // wake one worker thread
		std::this_thread::yield();	// allow this thread to be rescheduled
	}

	void execute(const std::function<void()>& pJob)
	{
		// The main thread label state is updated
		mCurrentLabel += 1;

		// Try to push a new job until it is pushed successfully
		while (!mJobPool.push_back(pJob))
		{
			poll();
		}

		mWakeCondition.notify_one(); // wake one thread
	}

	bool isBusy()
	{
		// Whenever the main thread label is not reached by the workers, it indicates that some worker is still alive
		return mFinishedLabel.load() < mCurrentLabel;
	}

	void wait()
	{
		while (isBusy())
		{
			poll();
		}
	}

	void dispatch(uint32_t pJobCount, uint32_t pGroupSize, const std::function<void(JobDispatchArgs)>& pJob)
	{
		if (pJobCount == 0 || pGroupSize == 0)
			return;

		// Calculate the amount of job groups to dispatch (overestimate, or "ceil")
		const uint32_t groupCount = (pJobCount + pGroupSize - 1) / pGroupSize;
		// The main thread label state is updated
		mCurrentLabel += groupCount;

		for (uint32_t groupIndex = 0; groupIndex < groupCount; ++groupIndex)
		{
			// For each group, generate one real job:
			const auto &jobGroup = [pJobCount, pGroupSize, pJob, groupIndex]()
			{
				// Calculate the current group's offset into the jobs
				const uint32_t groupJobOffset = groupIndex * pGroupSize;
				const uint32_t groupJobEnd = std::min(groupJobOffset + pGroupSize, pJobCount);

				JobDispatchArgs args;
				args.mGroupIndex = groupIndex;

				// Inside the group, loop through all job indices and execute job for each index
				for (uint32_t i = groupJobOffset; i < groupJobEnd; ++i)
				{
					args.mJobIndex = i;
					pJob(args);
				}
			};

			// Try to push a new job until it is pushed successfully
			while (!mJobPool.push_back(jobGroup))
				poll();

			mWakeCondition.notify_one(); // wake one thread
		}
	}
}