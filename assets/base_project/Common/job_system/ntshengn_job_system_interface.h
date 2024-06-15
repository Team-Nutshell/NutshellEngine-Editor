#pragma once
#include <functional>
#include <cstdint>

namespace NtshEngn {

	struct JobDispatchArguments {
		uint32_t workerIndex;
		uint32_t jobIndex;
	};

	class JobSystemInterface {
	public:
		virtual void init() = 0;
		virtual void destroy() = 0;

		virtual void execute(const std::function<void()>& job) = 0;
		virtual void dispatch(uint32_t jobCount, uint32_t jobsPerWorker, const std::function<void(JobDispatchArguments)>& job) = 0;

		virtual bool isBusy() = 0;
		virtual void wait() = 0;

		virtual uint32_t getNumThreads() const = 0;
	};

}