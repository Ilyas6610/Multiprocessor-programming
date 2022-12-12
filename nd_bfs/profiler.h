#pragma once

#include <iostream>
#include <chrono>


class Profiler {
public:
	bool fl = 1;
	const std::chrono::time_point<std::chrono::system_clock> begin;

	Profiler(): begin(std::chrono::system_clock::now()) {};

	long long getCount() {
		fl = false;
		return std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now() - begin).count();
	}

	~Profiler() {
		if(fl){
			auto end = std::chrono::system_clock::now();
			std::cout << "duration: " << std::chrono::duration_cast<std::chrono::milliseconds>
				(end - begin).count() << " milliseconds" << std::endl;
		}
	}
};