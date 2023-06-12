#pragma once

#include <mutex>

namespace Artemis::Core::Jobs
{
	template<typename T, size_t elements>
	class JobBuffer
	{
	public:
		bool Push(const T& _item)
		{
			bool bResult = false;
			m_mtxLock.lock();
			{
				unsigned int next = (m_uiHead + 1) % elements;
				if (next != m_uiTail)
				{
					m_Data[m_uiHead] = _item;
					m_uiHead = next;
					bResult = true;
				}
			}
			m_mtxLock.unlock();
			return bResult;
		}

		bool Pop(T& _item)
		{
			bool bResult = false;
			m_mtxLock.lock();
			{
				if (m_uiTail != m_uiHead)
				{
					_item = m_Data[m_uiTail];
					m_uiTail = (m_uiTail + 1) % elements;
					bResult = true;
				}
			}
			m_mtxLock.unlock();
			return bResult;
		}

	private:
		T m_Data[elements];

		unsigned int m_uiHead = 0;
		unsigned int m_uiTail = 0;

		std::mutex m_mtxLock;
	};
}