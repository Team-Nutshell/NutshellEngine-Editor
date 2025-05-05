#pragma once
#include <queue>
#include <cstdint>

namespace NtshEngn {

	class IDPool {
	public:
		uint32_t get() {
			if (!m_queue.empty()) {
				uint32_t id = m_queue.front();
				m_queue.pop();
				return id;
			}

			return m_currentID++;
		}

		void free(uint32_t id) {
			m_queue.push(id);
		}

	private:
		uint32_t m_currentID = 0;
		std::queue<uint32_t> m_queue;
	};

}