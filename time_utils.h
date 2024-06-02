//
// Created by Timothy Coelho on 6/1/24.
//

#ifndef LOWLATENCYFINTECH_TIME_UTILS_H
#define LOWLATENCYFINTECH_TIME_UTILS_H
#include <chrono>
#include <ctime>
#include <string>

namespace Common
{
	typedef int64_t nanos;

	constexpr nanos NANOS_TO_MICROS = 1000;
	constexpr nanos MICROS_TO_MILLIS = 1000;
	constexpr nanos MILLIS_TO_SECS = 1000;
	constexpr nanos NANOS_TO_MILIS = NANOS_TO_MICROS * MICROS_TO_MILLIS;
	constexpr nanos NANOS_TO_SECS = NANOS_TO_MILIS * MILLIS_TO_SECS;


	inline time_t get_ns() noexcept
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count() ;
	}

	inline std::string& get_time_str(std::string& time_str)
	{
		const time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		time_str.assign(ctime(&time)); //since this is an assign and can throw bad_alloc we don't want to use noexcept

		if (!time_str.empty())
		{
			time_str.at(time_str.length()-1) = '\0';
		}

		return time_str;
	}
}
#endif //LOWLATENCYFINTECH_TIME_UTILS_H
