//
// Created by Timothy Coelho on 6/3/24.
//

#ifndef LOWLATENCYFINTECH_SOCKET_UTILS_H
#define LOWLATENCYFINTECH_SOCKET_UTILS_H

#include <iostream>
#include <string>
#include <unordered_set>

#ifdef __APPLE__
	#include <sys/event.h>
#else
#include <sys/epoll.h>
#endif
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <fcntl.h>

#include "macros.h"
#include "Logger.h"

namespace Common
{
	constexpr int MAX_TCP_SRV_BKLG = 1024;

	std::string get_iface_ip(const std::string& iface);
	bool set_nonblocking(int fd);
	bool set_no_delay(int fd);
	bool set_so_timestamp(int fd);
	bool would_block();
	bool set_m_cast_ttl(int fd, int ttl);
	bool set_ttl(int fd, int ttl);
	bool join(int fd, const std::string& ip, const std::string& iface, int port);
	int create_socket(Logger& logger,  const std::string& t_ip, const std::string& iface, int port, bool is_udp, bool is_blocking, bool is_listening, int ttl, bool so_timestamp_needed);
}



#endif //LOWLATENCYFINTECH_SOCKET_UTILS_H
