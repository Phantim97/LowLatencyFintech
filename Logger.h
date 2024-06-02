//
// Created by Timothy Coelho on 6/1/24.
//

#ifndef LOWLATENCYFINTECH_LOGGER_H
#define LOWLATENCYFINTECH_LOGGER_H

#include <string>
#include <fstream>
#include <cstdio>

#include "macros.h"
#include "lock_free_q.h"
#include "thread_utils.h"
#include "time_utils.h"

namespace Common
{
	constexpr size_t LOG_Q_SIZE = 8 *  1024 * 1024;

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
			unsigned long  ul;
			unsigned long long ull;
			float f;
			double d;
		} u_;

		class Logger final
		{
		private:
			const std::string file_name_;
			std::ofstream file_;
			LFQueue<LogElement> queue_;
			std::atomic<bool> running_ = true;
			std::thread* logger_thread_ = nullptr;

		public:
			explicit Logger(const std::string& file_name) : file_name_(file_name), queue_(LOG_Q_SIZE)
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
				std::cerr << "Flushing and closing logger for" << file_name_ << '\n';

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
					for (const LogElement* next = queue_.get_next_to_read(); queue_.size() && next; next = queue_.get_next_to_read())
					{
						switch (next->type_)
						{
							case LogType::CHAR:
								file_ << next->u_.c;
								break;
							case LogType::INTEGER:
								file_ << next->u_.i;
								break;
							case LogType::LONG_INTEGER:
								file_ << next->u_.li;
								break;
							case LogType::LONG_LONG_INTEGER:
								file_ << next->u_.ll;
								break;
							case LogType::UNSIGNED_INTEGER:
								file_ << next->u_.u;
								break;
							case LogType::UNSIGNED_LONG_INTEGER:
								file_ << next->u_.ul;
								break;
							case LogType::UNSIGNED_LONG_LONG_INTEGER:
								file_ << next->u_.ull;
								break;
							case LogType::FLOAT:
								file_ << next->u_.f;
								break;
							case LogType::DOUBLE:
								file_ << next->u_.d;
								break;
						}
						queue_.update_read_idx();
						next = queue_.get_next_to_read();
					}
					using namespace std::literals::chrono_literals;
					std::this_thread::sleep_for(1s);
				}
			}
		};
	};
}

#endif //LOWLATENCYFINTECH_LOGGER_H
