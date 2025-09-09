#pragma once
#ifndef MARCHING_CUBES_PROFILING_CHUNK_MANAGER_HPP
#define MARCHING_CUBES_PROFILING_CHUNK_MANAGER_HPP

#include <core/aliases.hpp>

namespace marching_cubes::profiling {

	/// Runs both sync and async benches and prints a summary.
	/// frames: how many frames to simulate
	/// threads: number of worker threads for async bench (0 => hardware_concurrency()-1)
	void run_chunk_manager_benchmarks(u32 frames = 5000, unsigned int threads = 0);

} // namespace marching_cubes::tests

#endif // !MARCHING_CUBES_PROFILING_CHUNK_MANAGER_HPP
