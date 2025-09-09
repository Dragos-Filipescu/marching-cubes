#pragma once
#ifndef MARCHING_CUBES_CORE_WRAPPER_HPP
#define MARCHING_CUBES_CORE_WRAPPER_HPP

#include <compare>
#include <concepts>
#include <type_traits>
#include <utility>

#include <core/detail/default_deleters.hpp>

namespace marching_cubes::core {

	namespace detail {

		template<typename T>
		using DefaultDeleterT = typename deleters::DefaultDeleter<T>::type;
	}

	template<typename Derived, typename WrappedT>
	struct WrapperTraits {

		[[nodiscard]] constexpr operator WrappedT() const noexcept
		{
			return static_cast<const Derived*>(this)->getWrapped();
		}

		[[nodiscard]] constexpr auto operator<=>(WrappedT rhs) const noexcept
		{
			return static_cast<const Derived*>(this)->getWrapped() <=> rhs;
		}

		template<typename U>
			requires std::is_convertible_v<U, WrappedT>
		[[nodiscard]] constexpr auto operator<=>(U rhs) const noexcept
		{
			return static_cast<const Derived*>(this)->getWrapped() <=> static_cast<WrappedT>(rhs);
		}
	};

	template<
		typename WrappedT,
		typename DeleterT = detail::DefaultDeleterT<WrappedT>
	>
		requires (
			std::default_initializable<WrappedT>
			&& std::three_way_comparable<WrappedT>
			&& std::invocable<DeleterT, WrappedT>
		)
	class OwningWrapper final : public WrapperTraits<OwningWrapper<WrappedT, DeleterT>, WrappedT> {

		static constexpr bool HasNoopDeleter = std::is_same_v<DeleterT, deleters::NoopDeleter>;

	public:

		constexpr OwningWrapper(
			WrappedT value = WrappedT{},
			DeleterT deleter = DeleterT{}
		) noexcept
			: m_Value{ value },
			m_Deleter{ std::move(deleter) }
		{
		}

		OwningWrapper(const OwningWrapper&) = delete;
		OwningWrapper& operator=(const OwningWrapper&) = delete;

		constexpr OwningWrapper(OwningWrapper&& other) noexcept
			: m_Value{ std::exchange(other.m_Value, WrappedT{}) },
			m_Deleter{ std::move(other.m_Deleter) }
		{
		}
		constexpr OwningWrapper& operator=(OwningWrapper&& other) noexcept
		{
			if (this != &other) {
				if constexpr (!HasNoopDeleter) {
					if (m_Value != WrappedT{}) {
						m_Deleter(m_Value);
					}
				}
				m_Value = std::exchange(other.m_Value, WrappedT{});
				m_Deleter = std::move(other.m_Deleter);
			}
			return *this;
		}

		constexpr ~OwningWrapper() noexcept
		{
			if constexpr (!HasNoopDeleter) {
				if (m_Value != WrappedT{}) {
					m_Deleter(m_Value);
					m_Value = WrappedT{};
				}
			}
		}

		[[nodiscard]] constexpr WrappedT getWrapped() const noexcept
		{
			return m_Value;
		}

	private:
		WrappedT m_Value;
		DeleterT m_Deleter;
	};
	
	template<typename WrappedT, typename DeleterT>
	OwningWrapper(WrappedT, DeleterT) -> OwningWrapper<WrappedT, DeleterT>;

	template<typename WrappedT>
	OwningWrapper(WrappedT) -> OwningWrapper<WrappedT>;
}

#endif // !MARCHING_CUBES_CORE_WRAPPER_HPP

