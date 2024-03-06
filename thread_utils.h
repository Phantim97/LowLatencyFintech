//
// Created by Timothy Coelho on 3/5/24.
//

#ifndef LOWLATENCYFINTECH_THREAD_UTILS_H
#define LOWLATENCYFINTECH_THREAD_UTILS_H

#include <iostream>
#include <atomic>
#include <thread>
#include <unistd.h>
#include <pthread.h>

#include <sys/syscall.h>

inline bool setThreadCore(int core_id) noexcept
{
#ifndef __APPLE__
	cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    return (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) == 0);
#else
	return true;
#endif
}

/* Perfect Fowarding Boilerplate Example:

 template<typename Function, typename... Args>
void wrapper(Function&& func, Args&&... args)
{
    // Forward the function and arguments maintaining their lvalue/rvalue nature
    std::forward<Function>(func)(std::forward<Args>(args)...);
}

 */

/// Creates a thread instance, sets affinity on it, assigns it a name and
/// passes the function to be run on that thread as well as the arguments to the function.
template<typename T, typename... A>
inline std::thread* launch_thread(int core_id, const std::string &name, T &&func, A &&... args) noexcept {
	std::thread* t = new std::thread([&]()
			{
#ifndef __APPLE__
		if (core_id >= 0 && !setThreadCore(core_id))
		{
			std::cerr << "Failed to set core affinity for " << name << " " << pthread_self() << " to " << core_id << std::endl;
			exit(EXIT_FAILURE);
		}
		std::cerr << "Set core affinity for " << name << " " << pthread_self() << " to " << core_id << std::endl;
#endif
		std::forward<T>(func)((std::forward<A>(args))...); //Perfect Forwarding
	});

	using namespace std::literals::chrono_literals;
	std::this_thread::sleep_for(1s);

	return t;
}

#endif //LOWLATENCYFINTECH_THREAD_UTILS_H
