#ifndef MASONC_TIMER_HPP
#define MASONC_TIMER_HPP

#include <common.hpp>

#include <chrono>
#include <string>

namespace masonc
{
	struct timer
	{
		void start();
		void end();

		std::string get_formatted_duration(int decimal_places = 4);

	private:
		std::chrono::high_resolution_clock::time_point clock_start;
		std::chrono::high_resolution_clock::time_point clock_end;
		long long duration;
	};
}

#endif