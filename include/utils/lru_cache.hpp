#pragma once
#ifndef MARCHING_CUBES_UTILS_LRU_CACHE_HPP
#define MARCHING_CUBES_UTILS_LRU_CACHE_HPP

#include <glm/gtx/hash.hpp>

#include <concepts>
#include <functional>
#include <list>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace marching_cubes::utils::lru_cache {

	template<typename HasherT, typename T>
	concept HasherFor = std::invocable<
		HasherT,
		const T&
	>
	&& std::convertible_to<
		std::invoke_result_t<
			HasherT,
			const T&
		>,
		std::size_t
	>;

	template<typename EqualT, typename T>
	concept EqualFor = std::invocable<
		EqualT,
		const T&,
		const T&
	>
	&& std::convertible_to<
		std::invoke_result_t<
			EqualT,
			const T&,
			const T&
		>,
		bool
	>;

	template<
		typename KeyT,
		typename ValueT,
		typename KeyHasherT = std::hash<KeyT>,
		typename KeyEqualT = std::equal_to<KeyT>
	>
		requires HasherFor<KeyHasherT, KeyT>&& EqualFor<KeyEqualT, KeyT>
	class BasicLRUCache final {

		using HandleT = std::shared_ptr<ValueT>;

		struct Node final {
			KeyT key;
			HandleT value;
		};

	public:
		using KeyType = KeyT;
		using ValueType = ValueT;
		using KeyHasherType = KeyHasherT;
		using KeyEqualType = KeyEqualT;
		using EvictCallbackType = std::function<void(HandleT)>;

		using DefaultKeyHasher = std::hash<KeyType>;
		using DefaultKeyEqual = std::equal_to<KeyType>;

		using NodeType = Node;
		using ListType = std::list<NodeType>;
		using MapType = std::unordered_map<
			KeyType,
			typename ListType::iterator,
			KeyHasherType,
			KeyEqualType
		>;

		BasicLRUCache() noexcept = default;

		BasicLRUCache(
			std::size_t capacity,
			EvictCallbackType onEvict = {}
		)
			: m_Capacity{ capacity },
			m_List{},
			m_Map{},
			m_OnEvict{ std::move(onEvict) }
		{
			m_Map.reserve(capacity);
		}

		BasicLRUCache(const BasicLRUCache&) = delete;
		BasicLRUCache& operator=(const BasicLRUCache&) = delete;

		BasicLRUCache(BasicLRUCache&&) noexcept = default;
		BasicLRUCache& operator=(BasicLRUCache&&) noexcept = default;

		~BasicLRUCache() noexcept = default;

		/// @brief Put a value into the cache, bumping the LRU entry.
		void put(const KeyType& key, HandleT handle)
		{
			if (m_Capacity == 0) {
				return;
			}

			auto mapIt = m_Map.find(key);

			// 1) If the key already exists, just bump it & overwrite
			if (mapIt != m_Map.end()) {
				auto& [_, listIt] = *mapIt;
				listIt->value = std::move(handle);
				bump(mapIt);
				return;
			}

			// 2) Evict if full
			if (m_List.size() >= m_Capacity) {
				auto& [k, v] = m_List.back();
				if (m_OnEvict) {
					m_OnEvict(v);
				}
				m_Map.erase(k);
				m_List.pop_back();
			}

			// 3) Insert new node at front
			m_List.emplace_front(NodeType{ key, std::move(handle) });
			m_Map.emplace(key, m_List.begin());
		}

		template<typename U>
			requires std::constructible_from<ValueT, U>
		void put(const KeyType& key, U&& value)
		{
			put(key, std::make_shared<ValueT>(std::forward<U>(value)));
		}

		/// @brief Get the value associated with the key, bumping the LRU entry.
		[[nodiscard]] HandleT get(const KeyType& key)
		{
			if (auto it = m_Map.find(key); it != m_Map.end()) {
				// Bump the LRU entry
				bump(it);
				return it->second->value;
			}
			return nullptr;
		}

		/// @brief Get the value associated with the key, without bumping the LRU entry.
		[[nodiscard]] HandleT peek(const KeyType& key)
		{
			if (auto it = m_Map.find(key); it != m_Map.end()) {
				return it->second->value;
			}
			return nullptr;
		}

	private:

		void bump(typename MapType::iterator mapIt)
		{
			m_List.splice(m_List.begin(), m_List, mapIt->second);
			mapIt->second = m_List.begin();
		}

		std::size_t m_Capacity{};
		ListType m_List{};
		MapType m_Map{};
		EvictCallbackType m_OnEvict{};
	};
}

#endif // !MARCHING_CUBES_UTILS_LRU_CACHE_HPP

