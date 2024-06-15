#pragma once
#include "ntshengn_utils_math.h"
#include <vector>
#include <algorithm>
#include <limits>
#include <functional>

namespace NtshEngn {

	template <typename T>
	class Octree {
	public:
		struct Entry {
			Entry(const T& object, const Math::vec3& position, const Math::vec3& size) : object(object), position(position), size(size) {}

			T object;
			Math::vec3 position;
			Math::vec3 size;
		};

	private:
		class Node {
		public:
			Node(const Math::vec3& position, const Math::vec3& size) : m_position(position), m_size(size) {}
			
			void execute(const std::function<void(std::vector<Entry>&)>& operation) {
				if (!m_children.empty()) {
					for (uint8_t i = 0; i < 8; i++) {
						m_children[i].execute(operation);
					}
				}
				else {
					if (!m_entries.empty()) {
						operation(m_entries);
					}
				}
			}

			void split() {
				Math::vec3 halfSize = m_size / 2.0f;
				m_children.reserve(8);
				m_children.emplace_back(m_position + Math::vec3(-halfSize.x, halfSize.y, halfSize.z), halfSize);
				m_children.emplace_back(m_position + Math::vec3(halfSize.x, halfSize.y, halfSize.z), halfSize);
				m_children.emplace_back(m_position + Math::vec3(-halfSize.x, halfSize.y, -halfSize.z), halfSize);
				m_children.emplace_back(m_position + Math::vec3(halfSize.x, halfSize.y, -halfSize.z), halfSize);
				m_children.emplace_back(m_position + Math::vec3(-halfSize.x, -halfSize.y, halfSize.z), halfSize);
				m_children.emplace_back(m_position + Math::vec3(halfSize.x, -halfSize.y, halfSize.z), halfSize);
				m_children.emplace_back(m_position + Math::vec3(-halfSize.x, -halfSize.y, -halfSize.z), halfSize);
				m_children.emplace_back(m_position + Math::vec3(halfSize.x, -halfSize.y, -halfSize.z), halfSize);
			}

			void insert(const T& object, const Math::vec3& objectPosition, const Math::vec3& objectSize, uint32_t depthLeft) {
				if (!intersect(m_position, m_size, objectPosition, objectSize)) {
					return;
				}

				if (!m_children.empty()) {
					for (uint8_t i = 0; i < 8; i++) {
						m_children[i].insert(object, objectPosition, objectSize, depthLeft - 1);
					}
				}
				else {
					m_entries.emplace_back(object, objectPosition, objectSize);
					if (depthLeft > 0) {
						split();
						for (const Entry& entry : m_entries) {
							for (uint8_t i = 0; i < 8; i++) {
								m_children[i].insert(entry.object, entry.position, entry.size, depthLeft - 1);
							}
						}
						m_entries.clear();
					}
				}
			}

		private:
			bool intersect(const Math::vec3& p0, const Math::vec3& s0, const Math::vec3& p1, const Math::vec3& s1) {
				return (std::abs(p0.x - p1.x) <= (s0.x + s1.x)) && (std::abs(p0.y - p1.y) <= (s0.y + s1.y)) && (std::abs(p0.z - p1.z) <= (s0.z + s1.z));
			}

		private:
			std::vector<Entry> m_entries;
			std::vector<Node> m_children;
			Math::vec3 m_position;
			Math::vec3 m_size;
		};

	public:
		Octree(const Math::vec3& position, const Math::vec3& size, uint32_t maxDepth) : m_root(position, size), m_maxDepth(maxDepth) {}

		void execute(const std::function<void(std::vector<Entry>&)>& operation) {
			m_root.execute(operation);
		}
		
		void insert(T object, const Math::vec3& objectPosition, const Math::vec3& objectSize) {
			m_root.insert(object, objectPosition, objectSize, m_maxDepth);
		}

	private:
		Node m_root;
		uint32_t m_maxDepth;
	};

}