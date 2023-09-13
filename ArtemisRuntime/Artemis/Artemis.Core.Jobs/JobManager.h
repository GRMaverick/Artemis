#pragma once

#include <thread>
#include <functional>
#include <algorithm>
#include <atomic>
#include <condition_variable>

#include "JobBuffer.h"

namespace Artemis::Core::Jobs
{
	class JobManager
	{
	public:
		bool Initialise(unsigned int& _threads);
		bool IsBusy(void);

		void Wait(void);
		void Poll(void);
		void Execute(const std::function<void()>& _job);

	private:
		unsigned long long m_ullCurrentLabel = 0;
		std::atomic<unsigned long long> m_atmFinishedLabel = 0;
		std::mutex m_mtxWake;
		std::condition_variable m_cvWakeCondition;
		JobBuffer <std::function<void()>, 256> m_JobBuffer;
	};
}