#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

namespace NtshEngn {

	template<typename T>
	class ThreadSafeQueue {
	public:
		inline void push_back(const T& element) {
			std::unique_lock<std::mutex> lock(m_mutex);

			m_queue.push(element);

			lock.unlock();
		}

		inline bool pop_front(T& element) {
			std::unique_lock<std::mutex> lock(m_mutex);

			bool result = false;

			if (!m_queue.empty()) {
				element = m_queue.front();
				m_queue.pop();

				result = true;
			}
			else {
				result = false;
			}

			lock.unlock();

			return result;
		}

	private:
		std::queue<T> m_queue;

		std::mutex m_mutex;
		std::condition_variable m_conditionVariable;
	};

}