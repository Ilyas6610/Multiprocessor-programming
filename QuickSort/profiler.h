#pragma once

#include <iostream>
#include <chrono>


class Profiler {
public:
	const std::chrono::time_point<std::chrono::system_clock> begin;

	Profiler(): begin(std::chrono::system_clock::now()) {};

	~Profiler() {
		auto end = std::chrono::system_clock::now();
		std::cout << "duration: " << std::chrono::duration_cast<std::chrono::milliseconds>
		(end - begin).count() << " milliseconds" <<  std::endl;
	}
};