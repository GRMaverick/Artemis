#include "JobManager.h"

namespace Artemis::Core::Jobs
{
	bool JobManager::Initialise(unsigned int& _threads)
	{
		m_atmFinishedLabel.store(0);

		unsigned int threads = _threads;
		if (threads == 0)
		{
			threads = std::thread::hardware_concurrency();
		}

		for (unsigned int i = 0; i < threads; ++i)
		{
			std::thread worker(
				[this] {
					std::function<void()> job;
					while (true)
					{
						if (m_JobBuffer.Pop(job))
						{
							job();
							m_atmFinishedLabel.fetch_add(1);
						}
						else
						{
							std::unique_lock<std::mutex> lock(m_mtxWake);
							m_cvWakeCondition.wait(lock);
						}
					}
				}
			);
		}

		return true;
	}
	
	bool JobManager::IsBusy(void)
	{
		return m_atmFinishedLabel.load() < m_ullCurrentLabel;
	}

	void JobManager::Wait(void)
	{
		while (IsBusy())
		{
			Poll();
		}
	}

	void JobManager::Poll(void)
	{
		m_cvWakeCondition.notify_one();
		std::this_thread::yield();
	}

	void JobManager::Execute(const std::function<void()>& _job)
	{
		m_ullCurrentLabel += 1;
		while (!m_JobBuffer.Push(_job))
		{
			Poll();
		}

		m_cvWakeCondition.notify_one();
	}
}