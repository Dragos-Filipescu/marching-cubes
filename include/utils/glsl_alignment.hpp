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

namespace marching_cubes::utils {

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

	enum class GLSLLayout : std::uint8_t {
		Std140 = 0,
		Std430
	};

	template<typename T>
	concept GLSLScalar =
		std::same_as<T, std::int32_t>		// GLSL “int”
		|| std::same_as<T, std::uint32_t>	// GLSL “uint”
		|| std::same_as<T, float>			// GLSL “float”
		|| std::same_as<T, double>			// GLSL “double”
		|| std::same_as<T, bool>;			// GLSL “bool” (4-byte)

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

	namespace detail {

		template<typename>
		struct IsStdArray : std::false_type {};

		template<typename T, std::size_t N>
		struct IsStdArray<std::array<T, N>> : std::true_type {};

		template<typename T>
		constexpr bool IsStdArrayV = IsStdArray<T>::value;

		template<typename T>
		concept GLSLCArray =
			std::is_array_v<std::remove_cvref_t<T>>
			&& GLSLPrimitive<std::remove_cvref_t<RemoveAllExtentsT<T>>>;

		template<typename T>
		concept GLSLStdArray =
			IsStdArrayV<std::remove_cvref_t<T>>
			&& GLSLPrimitive<std::remove_cvref_t<RemoveAllExtentsT<T>>>;
	}

	template<typename T>
	concept GLSLArray =
		detail::GLSLCArray<T>
		|| detail::GLSLStdArray<T>;

	template<typename T>
	concept GLSLVariable = GLSLPrimitive<T> || GLSLArray<T>;

	template<std::size_t Alignment>
		requires (Alignment > 0)
	constexpr std::size_t alignUp(std::size_t size) noexcept {
		if constexpr ((Alignment & (Alignment - 1)) == 0) {
			// fast path for power-of-two alignments
			return ((size + (Alignment - 1)) & ~(Alignment - 1));
		}
		return ((size + Alignment - 1) / Alignment) * Alignment;
	}

	// Alignment for a standalone element.
	// *NOT* part of an array or struct (behaviour there differs between std140 and std430).

	template<GLSLLayout L, typename T>
	struct GLSLElementAlignment {
		static_assert(sizeof(T) == 0, "GLSLElementAlignment: unsupported GLSLVariable type");
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
		detail::VecTraits<V>::Dim * sizeof(detail::VecTraits<V>::ElemType)
	> {};

	//	vec3 specialization because it gets aligned to 4 * sizeof(element_type) under both layouts

	template<GLSLLayout L, GLSLScalar S, glm::qualifier Q>
	struct GLSLElementAlignment<L, glm::vec<3, S, Q>> : std::integral_constant<
		std::size_t,
		4 * sizeof(S)
	> {};

	namespace detail {

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
			(L == GLSLLayout::Std140)
			? alignUp<16>(GLSLArrayElementAlignment<L, RemoveAllExtentsT<Arr>>::value)
			: GLSLArrayElementAlignment<L, RemoveAllExtentsT<Arr>>::value
		> {};

		template<GLSLLayout L, GLSLVariable V>
		constexpr std::size_t GLSLArrayElementAlignmentV = GLSLArrayElementAlignment<L, V>::value;
	}

	template<GLSLLayout L, GLSLArray Arr>
	struct GLSLElementAlignment<L, Arr> : std::integral_constant<
		std::size_t,
		detail::GLSLArrayElementAlignment<L, Arr>::value
	> {};

	template<GLSLLayout L, GLSLMatrix M>
	struct GLSLElementAlignment<L, M> : std::integral_constant<
		std::size_t,
		GLSLElementAlignment<L, typename detail::MatTraits<M>::ColType[detail::MatTraits<M>::Cols]>::value
	> {};

	template<GLSLLayout L, GLSLVariable V>
	struct GLSLStructMemberAlignment : std::integral_constant<
		std::size_t,
		GLSLElementAlignment<L, V>::value
	> {};

	template<GLSLLayout L, GLSLScalar S, glm::qualifier Q>
	struct GLSLStructMemberAlignment<L, glm::vec<3, S, Q>> : std::integral_constant<
		std::size_t,
		(L == GLSLLayout::Std140)
		? 4 * sizeof(S)
		: 3 * sizeof(S)
	> {};

	template<GLSLLayout L, GLSLVariable... Vs>
	struct GLSLStructAlignment : std::integral_constant<
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
	> {};

	template<GLSLLayout L, typename T>
	constexpr std::size_t GLSLElementAlignmentV = GLSLElementAlignment<L, T>::value;

	template<GLSLLayout L, GLSLVariable V>
	constexpr std::size_t GLSLStructMemberAlignmentV = GLSLStructMemberAlignment<L, V>::value;

	template<GLSLLayout L, GLSLVariable... Vs>
	constexpr std::size_t GLSLStructAlignmentV = GLSLStructAlignment<L, Vs...>::value;
}

#endif // !MARCHING_CUBES_UTILS_GLSL_ALIGNMENT_HPP

