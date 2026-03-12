#pragma once
#ifndef MARCHING_CUBES_SCENE_UBO_HPP
#define MARCHING_CUBES_SCENE_UBO_HPP

#include <glm/glm.hpp>

#include <utils/glsl_alignment.hpp>

namespace marching_cubes::scene {

	using utils::alignment::GLSLLayout;
	using utils::alignment::GLSLStructAlignment140V;
	using utils::alignment::AlignedStructMember140;

	struct alignas(
		GLSLStructAlignment140V<
			glm::mat4,
			glm::mat4,
			glm::mat4
		>
	) ModelViewProjectionUBO {
		AlignedStructMember140<glm::mat4> model{ 1.0f };
		AlignedStructMember140<glm::mat4> view{ 1.0f };
		AlignedStructMember140<glm::mat4> projection{ 1.0f };
	};
}

#endif // !MARCHING_CUBES_SCENE_UBO_HPP

