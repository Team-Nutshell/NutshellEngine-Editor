#pragma once
#include <algorithm>
#include <stdexcept>
#include <string>
#include <cstddef>
#include <vector>

namespace NtshEngn {

	class Buffer {
	public:
		Buffer() : m_cursor(0) {}
		explicit Buffer(size_t size) : m_data(size), m_cursor(0) {}
		Buffer(const std::byte* data, size_t size) : m_data(size), m_cursor(0) {
			write(data, size);
			setCursorPosition(0);
		}
		Buffer(const Buffer& other) : m_data(other.m_data), m_cursor(other.m_cursor) {}
		Buffer& operator=(const Buffer& other) {
			m_data = other.m_data;
			m_cursor = other.m_cursor;

			return *this;
		}
		Buffer(Buffer&&) noexcept = default;

		void write(const std::byte* dataToWrite, size_t size) {
			if ((m_cursor + size) > m_data.size()) {
				m_data.resize(m_cursor + size);
			}
			std::copy(dataToWrite, dataToWrite + size, m_data.begin() + m_cursor);

			m_cursor += size;
		}

		size_t read(std::byte* dataToRead, size_t size) {
			size_t actualSize = (m_cursor + size) > m_data.size() ? (m_data.size() - m_cursor) : size;
			if (actualSize == 0) {
				return 0;
			}
			std::copy(m_data.begin() + m_cursor, m_data.begin() + m_cursor + actualSize, dataToRead);

			m_cursor += actualSize;
			
			return actualSize;
		}

		void setCursorPosition(size_t cursorPosition) {
			if (cursorPosition > m_data.size()) {
				throw std::out_of_range("setCursorPosition: cursor position (" + std::to_string(cursorPosition) + ") cannot be greater than DataBuffer size (" + std::to_string(m_data.size()) + ").");
			}
			m_cursor = cursorPosition;
		}

		const std::byte* getData() const { return m_data.data(); }
		size_t getSize() const { return m_data.size(); }

	private:
		std::vector<std::byte> m_data;

		size_t m_cursor;
	};

}