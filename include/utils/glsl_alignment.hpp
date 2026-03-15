#pragma once
#ifndef MARCHING_CUBES_UTILS_GLSL_ALIGNMENT_HPP
#define MARCHING_CUBES_UTILS_GLSL_ALIGNMENT_HPP

#include <glm/glm.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include <core/aliases.hpp>
#include <utils/utils.hpp>

namespace marching_cubes::utils::alignment {

	template<auto V>
	struct AlwaysFalse : std::false_type {};

	template<auto V>
	constexpr bool AlwaysFalseV = AlwaysFalse<V>::value;

	template<typename T>
	struct RemoveAllExtents {
		using type = T;
	};

	template<typename T>
	using RemoveAllExtentsT = typename RemoveAllExtents<T>::type;

	template<typename T>
	struct RemoveAllExtents<T[]> {
		using type = RemoveAllExtentsT<T>;
	};

	template<typename T, std::size_t N>
	struct RemoveAllExtents<T[N]> {
		using type = RemoveAllExtentsT<T>;
	};

	template<typename T, std::size_t N>
	struct RemoveAllExtents<std::array<T, N>> {
		using type = RemoveAllExtentsT<T>;
	};

	namespace detail {

		// Helper for identifying GLSL vector types based on glm vectors

		template<typename>
		struct VecTraits;

		template<typename T, glm::qualifier Q>
		struct VecTraits<glm::vec<2, T, Q>> {
			static constexpr std::size_t Dim = 2;
			using ElemType = T;
		};
		template<typename T, glm::qualifier Q>
		struct VecTraits<glm::vec<3, T, Q>> {
			static constexpr std::size_t Dim = 3;
			using ElemType = T;
		};
		template<typename T, glm::qualifier Q>
		struct VecTraits<glm::vec<4, T, Q>> {
			static constexpr std::size_t Dim = 4;
			using ElemType = T;
		};

		// Helper for identifying GLSL matrix types based on glm matrices

		template<typename>
		struct MatTraits;

		template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
		struct MatTraits<glm::mat<C, R, T, Q>> {
			static constexpr size_t Cols = C;
			static constexpr size_t Rows = R;
			using ColType = typename glm::mat<C, R, T, Q>::col_type;
			using RowType = typename glm::mat<C, R, T, Q>::row_type;
			using ElemType = T;
		};
	}

	enum class GLSLLayout : u8 {
		Std140 = 0,
		Std430
	};

	template<GLSLLayout L, typename... Ms>
	struct GLSLStructProxy {};

	namespace detail {

		template<typename T>
		concept GLSLScalar =
			std::same_as<T, std::int32_t>		// GLSL "int"
			|| std::same_as<T, std::uint32_t>	// GLSL "uint"
			|| std::same_as<T, float>			// GLSL "float"
			|| std::same_as<T, double>			// GLSL "double"
			|| std::same_as<T, bool>;			// GLSL "bool" (4-byte)

		template<typename T>
		concept GLSLVector =
			requires {
				{ detail::VecTraits<T>::Dim } -> std::convertible_to<std::size_t>;
				typename detail::VecTraits<T>::ElemType;
		}
		&& GLSLScalar<typename detail::VecTraits<T>::ElemType>;

		template<typename T>
		concept GLSLMatrix =
			requires {
				{ detail::MatTraits<T>::Cols } -> std::convertible_to<std::size_t>;
				{ detail::MatTraits<T>::Rows } -> std::convertible_to<std::size_t>;
				typename detail::MatTraits<T>::ColType;
				typename detail::MatTraits<T>::RowType;
				typename detail::MatTraits<T>::ElemType;
		}
		&& GLSLVector<typename detail::MatTraits<T>::ColType>
			&& GLSLVector<typename detail::MatTraits<T>::RowType>
			&& GLSLScalar<typename detail::MatTraits<T>::ElemType>;

		template<typename T>
		concept GLSLPrimitive = GLSLScalar<T> || GLSLVector<T> || GLSLMatrix<T>;

		template<typename>
		struct IsStdArray : std::false_type {};

		template<typename T, std::size_t N>
		struct IsStdArray<std::array<T, N>> : std::true_type {};

		template<typename T>
		constexpr bool IsStdArrayV = IsStdArray<T>::value;

		template<typename>
		struct IsGLSLStructProxy : std::false_type {};

		template<GLSLLayout L, typename... Ms>
		struct IsGLSLStructProxy<GLSLStructProxy<L, Ms...>> : std::true_type {};

		template<typename T>
		constexpr bool IsGLSLStructProxyV = IsGLSLStructProxy<T>::value;

		template<typename T>
		concept GLSLCArray =
			std::is_array_v<std::remove_cvref_t<T>>
			&& (GLSLPrimitive<std::remove_cvref_t<RemoveAllExtentsT<T>>> || IsGLSLStructProxyV<std::remove_cvref_t<RemoveAllExtentsT<T>>>);

		template<typename T>
		concept GLSLStdArray =
			IsStdArrayV<std::remove_cvref_t<T>>
			&& (GLSLPrimitive<std::remove_cvref_t<RemoveAllExtentsT<T>>> || IsGLSLStructProxyV<std::remove_cvref_t<RemoveAllExtentsT<T>>>);

		template<typename T>
		concept GLSLArray =
			detail::GLSLCArray<T>
			|| detail::GLSLStdArray<T>;

	}

	template<typename T>
	concept GLSLVariable = detail::GLSLPrimitive<T> || detail::GLSLArray<T> || detail::IsGLSLStructProxyV<T>;

	namespace detail {

		// Alignment for a standalone element.
		// *NOT* part of an array or struct (behaviour there differs between std140 and std430).

		template<GLSLLayout L, typename T>
		struct GLSLElementAlignment {
			static_assert(
				false,
				"GLSLElementAlignment: unsupported GLSLVariable type"
			);
		};

		//	Scalar specialization
		template<GLSLLayout L, GLSLScalar S>
		struct GLSLElementAlignment<L, S> : std::integral_constant<std::size_t, sizeof(S)> {};

		//	Scalar bool specialization, because bool is	4 bytes and not sizeof(bool)
		template<GLSLLayout L>
		struct GLSLElementAlignment<L, bool> : std::integral_constant<std::size_t, 4> {};

		//	Vector specialization
		template<GLSLLayout L, GLSLVector V>
		struct GLSLElementAlignment<L, V> : std::integral_constant<
			std::size_t,
			detail::VecTraits<V>::Dim * sizeof(typename detail::VecTraits<V>::ElemType)
		> {};

		//	vec3 specialization because it gets aligned to 4 * sizeof(element_type) under both layouts
		template<GLSLLayout L, GLSLScalar S, glm::qualifier Q>
		struct GLSLElementAlignment<L, glm::vec<3, S, Q>> : std::integral_constant<
			std::size_t,
			4 * sizeof(S)
		> {};

		template<GLSLLayout L, GLSLVariable V>
		struct GLSLStructMemberAlignment : std::integral_constant<
			std::size_t,
			GLSLElementAlignment<L, V>::value
		> {
		};

		template<GLSLLayout L, GLSLScalar S, glm::qualifier Q>
		struct GLSLStructMemberAlignment<L, glm::vec<3, S, Q>> : std::integral_constant<
			std::size_t,
			(L == GLSLLayout::Std140)
			? 4 * sizeof(S)
			: 3 * sizeof(S)
		> {
		};

		template<GLSLLayout L1, GLSLLayout L2, GLSLVariable... Vs>
		struct GLSLElementAlignment<L1, GLSLStructProxy<L2, Vs...>> : std::integral_constant <
			std::size_t,
			(L1 == GLSLLayout::Std140)
			? alignUp<16>(
				std::max(
					std::initializer_list{ GLSLStructMemberAlignment<L2, Vs>::value... }
				)
			)
			: std::max(
				std::initializer_list{ GLSLStructMemberAlignment<L2, Vs>::value... }
			)
		> {
		};

		template<GLSLLayout L, GLSLVariable V>
		struct GLSLArrayElementAlignment : std::integral_constant<
			std::size_t,
			GLSLElementAlignment<L, V>::value
		> {};

		template<GLSLLayout L, GLSLScalar S, glm::qualifier Q>
		struct GLSLArrayElementAlignment<L, glm::vec<3, S, Q>> : std::integral_constant<
			std::size_t,
			(L == GLSLLayout::Std140)
			? 4 * sizeof(S)
			: 3 * sizeof(S)
		> {};

		template<GLSLLayout L, GLSLArray Arr>
		struct GLSLArrayElementAlignment<L, Arr> : std::integral_constant<
			std::size_t,
			(
				(L == GLSLLayout::Std140)
				? alignUp<16>(GLSLArrayElementAlignment<L, RemoveAllExtentsT<Arr>>::value)
				: GLSLArrayElementAlignment<L, RemoveAllExtentsT<Arr>>::value
			)
		> {};

		template<GLSLLayout L, GLSLArray Arr>
		struct GLSLElementAlignment<L, Arr> : std::integral_constant<
			std::size_t,
			detail::GLSLArrayElementAlignment<L, Arr>::value
		> {
		};

		template<GLSLLayout L, GLSLMatrix M>
		struct GLSLElementAlignment<L, M> : std::integral_constant<
			std::size_t,
			GLSLElementAlignment<L, typename detail::MatTraits<M>::ColType[detail::MatTraits<M>::Cols]>::value
		> {
		};

		template<GLSLLayout L, GLSLVariable... Vs>
		struct GLSLStructAlignment : std::integral_constant <
			std::size_t,
			(L == GLSLLayout::Std140)
			? alignUp<16>(
				std::max(
					std::initializer_list{ GLSLStructMemberAlignment<L, Vs>::value... }
				)
			)
			: std::max(
				std::initializer_list{ GLSLStructMemberAlignment<L, Vs>::value... }
			)
		> {
		};

		template<GLSLLayout L>
		struct GLSLStructAlignment<L> {
			static_assert(
				false,
				"GLSLStructAlignment: parameter pack cannot be empty"
			);
		};
	}

	// Primitive alignment helpers

	template<GLSLLayout L, typename T>
	constexpr std::size_t GLSLElementAlignmentV = detail::GLSLElementAlignment<L, T>::value;

	template<typename T>
	constexpr std::size_t GLSLElementAlignment140V = detail::GLSLElementAlignment<GLSLLayout::Std140, T>::value;

	template<typename T>
	constexpr std::size_t GLSLElementAlignment430V = detail::GLSLElementAlignment<GLSLLayout::Std430, T>::value;

	// Array alignment helpers

	template<GLSLLayout L, GLSLVariable V>
	constexpr std::size_t GLSLArrayElementAlignmentV = detail::GLSLArrayElementAlignment<L, V>::value;

	template<GLSLVariable V>
	constexpr std::size_t GLSLArrayElementAlignment140V = detail::GLSLArrayElementAlignment<GLSLLayout::Std140, V>::value;

	template<GLSLVariable V>
	constexpr std::size_t GLSLArrayElementAlignment430V = detail::GLSLArrayElementAlignment<GLSLLayout::Std430, V>::value;

	// Struct member alignment helpers

	template<GLSLLayout L, GLSLVariable V>
	constexpr std::size_t GLSLStructMemberAlignmentV = detail::GLSLStructMemberAlignment<L, V>::value;

	template<GLSLVariable V>
	constexpr std::size_t GLSLStructMemberAlignment140V = detail::GLSLStructMemberAlignment<GLSLLayout::Std140, V>::value;

	template<GLSLVariable V>
	constexpr std::size_t GLSLStructMemberAlignment430V = detail::GLSLStructMemberAlignment<GLSLLayout::Std430, V>::value;

	// Struct alignment helpers

	template<GLSLLayout L, GLSLVariable... Vs>
	constexpr std::size_t GLSLStructAlignmentV = detail::GLSLStructAlignment<L, Vs...>::value;

	template<GLSLVariable... Vs>
	constexpr std::size_t GLSLStructAlignment140V = detail::GLSLStructAlignment<GLSLLayout::Std140, Vs...>::value;

	template<GLSLVariable... Vs>
	constexpr std::size_t GLSLStructAlignment430V = detail::GLSLStructAlignment<GLSLLayout::Std430, Vs...>::value;

	template<GLSLLayout L, GLSLVariable T>
	struct alignas(GLSLStructMemberAlignmentV<L, T>) AlignedStructMember {
		T value;
		constexpr operator T& () { return value; }
		constexpr operator const T& () const { return value; }
		AlignedStructMember& operator=(const T& v) { value = v; return *this; }
	};

	template<GLSLVariable T>
	using AlignedStructMember140 = AlignedStructMember<GLSLLayout::Std140, T>;

	template<GLSLVariable T>
	using AlignedStructMember430 = AlignedStructMember<GLSLLayout::Std430, T>;
}

#endif // !MARCHING_CUBES_UTILS_GLSL_ALIGNMENT_HPP

