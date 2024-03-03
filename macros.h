//
// Created by Timothy Coelho on 3/1/24.
//

#ifndef LOWLATENCYFINTECH_MACROS_H
#define LOWLATENCYFINTECH_MACROS_H

#include <string>
#include <iostream>

inline void ASSERT(bool cond, const std::string& msg) noexcept
{
	if (!cond)
	{
			std::cerr << msg << '\n';
			exit(EXIT_FAILURE);
	}
}

inline void FATAL(const std::string& msg) noexcept
{
	std::cerr << "msg" << '\n';
	exit(EXIT_FAILURE);
}
#endif //LOWLATENCYFINTECH_MACROS_H
