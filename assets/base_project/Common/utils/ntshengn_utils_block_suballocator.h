#pragma once
#include <deque>
#include <limits>

namespace NtshEngn {

	struct Block {
		size_t offset;
		size_t size;
		bool inUse;
	};

	struct BlockSuballocator {
	public:
		BlockSuballocator(size_t size) {
			m_blockQueue.push_back({ 0, size, false });
		}

		size_t addBlock(size_t size) {
			auto it = m_blockQueue.begin();
			while (it != m_blockQueue.end()) {
				Block& currentBlock = *it;
				if (!currentBlock.inUse) {
					if (currentBlock.size >= size) {
						m_blockQueue.insert(it + 1, { currentBlock.offset + size, currentBlock.size - size, false });
						currentBlock.size = size;
						currentBlock.inUse = true;

						return currentBlock.offset;
					}
				}

				it++;
			}

			return std::numeric_limits<size_t>::max();
		}

		void freeBlock(size_t offset, size_t size) {
			auto it = m_blockQueue.begin();
			while (it != m_blockQueue.end()) {
				Block& currentBlock = *it;
				if ((currentBlock.offset == offset) && (currentBlock.size == size)) {
					if ((it != m_blockQueue.begin()) &&
						((it + 1) != m_blockQueue.end())) {
						Block& previousBlock = *(it - 1);
						Block& nextBlock = *(it + 1);

						if (!previousBlock.inUse && !nextBlock.inUse) { // Merge
							previousBlock.size += size + nextBlock.size;

							m_blockQueue.erase(it + 1);
							m_blockQueue.erase(it);
						}
						else if (previousBlock.inUse && nextBlock.inUse) { // Not in use anymore
							currentBlock.inUse = false;
						}
						else if (!previousBlock.inUse && nextBlock.inUse) { // Merge
							previousBlock.size += size;

							m_blockQueue.erase(it);
						}
						else if (previousBlock.inUse && !nextBlock.inUse) { // Merge
							nextBlock.offset = offset;
							nextBlock.size += size;

							m_blockQueue.erase(it);
						}
					}
					else if (it != m_blockQueue.begin()) { // Next is end
						Block& previousBlock = *(it - 1);
						if (!previousBlock.inUse) { // Merge
							previousBlock.size += size;

							m_blockQueue.erase(it);
						}
						else { // Not in use anymore
							currentBlock.inUse = false;
						}
					}
					else if ((it + 1) != m_blockQueue.end()) { // Previous is begin
						Block& nextBlock = *(it + 1);
						if (!nextBlock.inUse) { // Merge
							nextBlock.offset = offset;
							nextBlock.size += size;

							m_blockQueue.erase(it);
						}
						else { // Not in use anymore
							currentBlock.inUse = false;
						}
					}

					return;
				}

				it++;
			}
		}

	private:
		std::deque<Block> m_blockQueue;
	};

}