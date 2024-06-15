#pragma once
#include <unordered_map>

namespace NtshEngn {

	template <typename T, typename U>
	class Bimap {
	public:
		void insert_or_assign(const T& key, const U& elem) {
			m_tu[key] = elem;
			m_ut[elem] = key;
		}

		void insert_or_assign(const U& key, const T& elem) {
			m_ut[key] = elem;
			m_tu[elem] = key;
		}

		bool exist(const T& key) const {
			return (m_tu.find(key) != m_tu.end());
		}

		bool exist(const U& key) const {
			return (m_ut.find(key) != m_ut.end());
		}

		void erase(const T& key) {
			m_ut.erase(m_tu[key]);
			m_tu.erase(key);
		}

		void erase(const U& key) {
			m_tu.erase(m_ut[key]);
			m_ut.erase(key);
		}

		U& operator[](const T& key) {
			return m_tu[key];
		}
		const U operator[](const T& key) const {
			return m_tu[key];
		}

		T& operator[](const U& key) {
			return m_ut[key];
		}
		const T operator[](const U& key) const {
			return m_ut[key];
		}

		size_t size() const {
			return m_tu.size();
		}

	private:
		std::unordered_map<T, U> m_tu;
		std::unordered_map<U, T> m_ut;
	};

}