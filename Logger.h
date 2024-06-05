//
// Created by Timothy Coelho on 6/1/24.
//

#ifndef LOWLATENCYFINTECH_LOGGER_H
#define LOWLATENCYFINTECH_LOGGER_H

#include <string>
#include <fstream>
#include <cstdio>
#include <type_traits>

#include "macros.h"
#include "lock_free_q.h"
#include "thread_utils.h"
#include "time_utils.h"

// SFINAE
template<typename T>
struct st_log_t
{
	static constexpr bool val = false;
};

// Full specialization for template class
template<>
struct st_log_t<int>
{
	static constexpr bool val = true;
};

template<>
struct st_log_t<long>
{
	static constexpr bool val = true;
};

template<>
struct st_log_t<long long>
{
	static constexpr bool val = true;
};

// Unsigned

template<>
struct st_log_t<unsigned int>
{
	static constexpr bool val = true;
};

template<>
struct st_log_t<unsigned long>
{
	static constexpr bool val = true;
};

template<>
struct st_log_t<unsigned long long>
{
	static constexpr bool val = true;
};

template<>
struct st_log_t<double>
{
	static constexpr bool val = true;
};

template<>
struct st_log_t<float>
{
	static constexpr bool val = true;
};

template<>
struct st_log_t<char>
{
	static constexpr bool val = true;
};

template<>
struct st_log_t<bool>
{
	static constexpr bool val = true;
};

template<typename T> constexpr bool is_matching = st_log_t<T>::val;

//using template - type alias
template<typename T> using log_t = typename std::enable_if<is_matching<T>, T>::type;

namespace Common
{
	constexpr size_t LOG_Q_SIZE = 8 * 1024 * 1024;

	enum class LogType : int8_t
	{
		CHAR = 0,
		INTEGER = 1,
		LONG_INTEGER = 2,
		LONG_LONG_INTEGER = 3,
		UNSIGNED_INTEGER = 4,
		UNSIGNED_LONG_INTEGER = 5,
		UNSIGNED_LONG_LONG_INTEGER = 6,
		FLOAT = 7,
		DOUBLE = 8
	};

	struct LogElement
	{
		LogType type_ = LogType::CHAR;

		//std::variant is not optimal since it tries to be "type safe" union is faster
		union
		{
			char c;
			int i;
			long li;
			long long ll;
			unsigned u;
			unsigned long ul;
			unsigned long long ull;
			float f;
			double d;
		} data_;

		LogElement() = default;

		LogElement(LogType type, char c) : type_(type)
		{
			data_.c = c;
		}

		LogElement(LogType type, int i) : type_(type)
		{
			data_.i = i;
		}

		LogElement(LogType type, long l) : type_(type)
		{
			data_.li = l;
		}

		LogElement(LogType type, long long ll) : type_(type)
		{
			data_.ll = ll;
		}

		LogElement(LogType type, unsigned u) : type_(type)
		{
			data_.u = u;
		}

		LogElement(LogType type, unsigned long ul) : type_(type)
		{
			data_.ul = ul;
		}

		LogElement(LogType type, unsigned long long ull) : type_(type)
		{
			data_.ull = ull;
		}

		LogElement(LogType type, float f) : type_(type)
		{
			data_.f = f;
		}

		LogElement(LogType type, double d) : type_(type)
		{
			data_.d = d;
		}
	};

	class Logger final
	{
	private:
		const std::string file_name_;
		std::ofstream file_;
		LFQueue<LogElement> queue_;
		std::atomic<bool> running_ = true;
		std::thread *logger_thread_ = nullptr;

	public:
		explicit Logger(const std::string &file_name) : file_name_(file_name), queue_(LOG_Q_SIZE)
		{
			file_.open(file_name);
			ASSERT(file_.is_open(), "Could not open log file: " + file_name);
			logger_thread_ = launch_thread(-1, "Common/Logger", [this]()
			{
				flush_queue();
			});
			ASSERT(logger_thread_ != nullptr, "Failed to start logger thread");
		}

		~Logger()
		{
			std::cerr << "Flushing and closing logger for " << file_name_ << '\n';

			while (queue_.size())
			{
				using namespace std::literals::chrono_literals;
				std::this_thread::sleep_for(1s);
			}

			running_ = false;

			file_.close();
		}

		Logger() = delete;

		Logger(const Logger &) = delete;

		Logger(const Logger &&) = delete;

		Logger &operator=(const Logger &) = delete;

		Logger &operator=(const Logger &&) = delete;

		void flush_queue() noexcept
		{
			while (running_)
			{
				for (const LogElement *next = queue_.get_next_to_read();
				     queue_.size() && next; next = queue_.get_next_to_read())
				{
					switch (next->type_)
					{
						case LogType::CHAR:
							file_ << next->data_.c;
							break;
						case LogType::INTEGER:
							file_ << next->data_.i;
							break;
						case LogType::LONG_INTEGER:
							file_ << next->data_.li;
							break;
						case LogType::LONG_LONG_INTEGER:
							file_ << next->data_.ll;
							break;
						case LogType::UNSIGNED_INTEGER:
							file_ << next->data_.u;
							break;
						case LogType::UNSIGNED_LONG_INTEGER:
							file_ << next->data_.ul;
							break;
						case LogType::UNSIGNED_LONG_LONG_INTEGER:
							file_ << next->data_.ull;
							break;
						case LogType::FLOAT:
							file_ << next->data_.f;
							break;
						case LogType::DOUBLE:
							file_ << next->data_.d;
							break;
					}
					queue_.update_read_idx();
				}

				using namespace std::literals::chrono_literals;
				std::this_thread::sleep_for(1s);
			}
		}

		void push_value(const LogElement& log_element) noexcept
		{
			*(queue_.get_next_write_loc()) = log_element;
			queue_.update_write_idx();
		}

		/* SFINAE for compile time branching for log element construction. The literature decides to use function
		 * overloads, but I disagree with the notion and decided that using SFINAE is more optimal here because there
		 * is no function resolution at runtime anymore and push_values call is branched at compile time making this a
		 * seamless operation.
		 */
		template<typename T>
		typename std::enable_if<is_matching < T>, void>::type push_value(const T &value) noexcept
		{
			if constexpr (std::is_same_v<T, char>)
			{
				push_value(LogElement {LogType::CHAR, value});
			}
			else if constexpr (std::is_same_v<T, int>)
			{
				push_value(LogElement {LogType::INTEGER, value});
			}
			else if constexpr (std::is_same_v<T, long>)
			{
				push_value(LogElement {LogType::LONG_INTEGER, value});
			}
			else if constexpr (std::is_same_v<T, long long>)
			{
				push_value(LogElement {LogType::LONG_LONG_INTEGER, value});
			}
			else if constexpr (std::is_same_v<T, unsigned>)
			{
				push_value(LogElement {LogType::UNSIGNED_INTEGER, value});
			}
			else if constexpr (std::is_same_v<T, unsigned long>)
			{
				push_value(LogElement {LogType::UNSIGNED_LONG_INTEGER, value});
			}
			else if constexpr (std::is_same_v<T, unsigned long long>)
			{
				push_value(LogElement {LogType::UNSIGNED_LONG_LONG_INTEGER, value});
			}
			else if constexpr (std::is_same_v<T, float>)
			{
				push_value(LogElement {LogType::FLOAT, value});
			}
			else if constexpr (std::is_same_v<T, double>)
			{
				push_value(LogElement {LogType::DOUBLE, value});
			}
			else if constexpr (std::is_same_v<T, bool>)
			{
				push_value(LogElement {LogType::INTEGER, static_cast<int>(value)});
			}
		}

		void push_value(const char *value) noexcept
		{
			while (*value)
			{
				push_value(*value++);
			}
		}

		void push_value(const std::string &value) noexcept {
			for (const char &c : value)
			{
				push_value(c);
			}
		}

		template<typename T, typename... A>
		void log(const char *s, const T &value, A... args) noexcept
		{
			while (*s)
			{
				if (*s == '%')
				{
					if ((*(s + 1) == '%')) [[unlikely]]
					{
						s++;
					}
					else
					{
						push_value(value);
						log(s + 1, args...);
						return;
					}
				}
				push_value(*s++);
			}
			FATAL("extra arguments provided to log()");
		}

		void log(const char *s) noexcept
		{
			while (*s)
			{
				if (*s == '%')
				{
					if ((*(s + 1) == '%')) [[unlikely]]
					{
						s++;
					}
					else
					{
						FATAL("missing arguments to log()");
					}
				}
				push_value(*s++);
			}
		}
	};
}


#endif //LOWLATENCYFINTECH_LOGGER_H
