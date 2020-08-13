#include <timer.hpp>

#include <cmath>

namespace masonc
{
	void timer::start()
	{
		clock_start = std::chrono::high_resolution_clock::now();
	}

	void timer::end()
	{
		clock_end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(clock_end - clock_start).count();
	}

	std::string timer::get_formatted_duration(int decimal_places)
	{
		// Print fractional ms by looking at ns
		if (duration < 1)
		{
			// Get duration in ns
			duration = std::chrono::duration_cast<std::chrono::nanoseconds>(clock_end - clock_start).count();

			long double d = std::pow(10.0l, static_cast<long double>(decimal_places));

			// Dividing by 1000000.0 converts ns to ms
			// 'd' is used to round to a specific amount of decimal places
			return std::to_string(std::round(static_cast<long double>(duration) / 1000000.0l * d) / d).substr(0, 2 + decimal_places) + " ms";
		}

		// Print in fractional seconds if amount of ms exceeds this number
		if (duration > 100)
		{
			long double d = std::pow(10.0l, static_cast<long double>(decimal_places));

			// Dividing by 1000.0 converts ms to s
			// 'd' is used to round to a specific amount of decimal places
			return std::to_string(std::round(static_cast<long double>(duration) / 1000.0l * d) / d).substr(0, 2 + decimal_places) + " s";
		}

		// Print in simple non-fractional ms
		return std::to_string(duration) + " ms";
	}
}