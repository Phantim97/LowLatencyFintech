//
// Created by Timothy Coelho on 6/3/24.
//
#include "socket_utils.h"

namespace Common
{

	std::string get_iface_ip(const std::string &iface)
	{
		char buf[NI_MAXHOST] = {'\0'};
		ifaddrs* ifaddr = nullptr;

		if (getifaddrs(&ifaddr) != -1)
		{
			for (ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next)
			{
				if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET && iface == ifa->ifa_name)
				{
					getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), buf, sizeof(buf), nullptr, 0, NI_NUMERICHOST);
					break;
				}
			}

			freeifaddrs(ifaddr);
		}

		return buf;
	}


	bool set_nonblocking(int fd)
	{
		const int flags = fcntl(fd, F_GETFL, 0);

		if (flags == -1)
		{
			return false;
		}

		if (flags & O_NONBLOCK)
		{
			return true;
		}

		return fcntl(fd, F_SETFL, (flags | O_NONBLOCK) != -1);
	}

	//Disabling Nogle's Algorithm which handles buffering improvements allows for better latency
	bool set_no_delay(int fd)
	{
		//This int to reinterpret sets allows the TCP option to be set to a ptr.
		int one = 1;
		return (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<void*>(&one), sizeof(one))!= -1);
	}

	bool set_so_timestamp(int fd)
	{
		int one = 1;
		return (setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, reinterpret_cast<void*>(&one), sizeof(one)) != -1);
	}

	bool would_block()
	{
		return (errno == EWOULDBLOCK || errno == EINPROGRESS);
	}

	bool set_m_cast_ttl(int fd, int ttl)
	{
		return (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<void*>(&ttl), sizeof(ttl)) != -1);
	}

	bool set_ttl(int fd, int ttl)
	{
		return (setsockopt(fd, IPPROTO_IP, IP_TTL, reinterpret_cast<void*>(&ttl), sizeof(ttl)) != -1);
	}

	bool join(int fd, const std::string &ip, const std::string &iface, int port)
	{
		return false;
	}

	int create_socket(Logger &logger, const std::string &t_ip, const std::string &iface, int port, bool is_udp,
	                  bool is_blocking, bool is_listening, int ttl, bool so_timestamp_needed)
	{
		std::string time_str;

		const std::string ip = t_ip.empty() ? get_iface_ip(iface) : t_ip;
		logger.log("%:% %() % ip:% iface:% port:% is_udp:% is_blocking:% is_listening:% ttl:% SO_TIME:%\n", __FILE__,
		           __LINE__, __FUNCTION__, Common::get_time_str(time_str), ip, iface, port, is_udp, is_blocking,
		           is_listening, ttl, so_timestamp_needed);

		addrinfo hints {};
		hints.ai_family = AF_INET;
		hints.ai_socktype = is_udp ? SOCK_DGRAM : SOCK_STREAM;
		hints.ai_protocol = is_udp ? IPPROTO_UDP : IPPROTO_TCP;
		hints.ai_flags = is_listening ? AI_PASSIVE : 0;

		if (std::isdigit(ip.c_str()[0]))
		{
			hints.ai_flags |= AI_NUMERICHOST;
		}

		hints.ai_flags |= AI_NUMERICSERV;

		addrinfo *res = nullptr;
		const int rc = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &res);

		if (rc)
		{
			logger.log("getaddrinfo() failed. error:% errno:% \n", gai_strerror(rc), strerror(errno));
			return -1;
		}

		int fd = 1;
		int one = 1;

		for (addrinfo *rp = res; rp; rp = rp->ai_next)
		{
			fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
			if (fd == -1)
			{
				logger.log("socket() failed. errno:%\n", strerror(errno));
				return -1;
			}

			if (!is_blocking)
			{
				if (!set_nonblocking(fd))
				{
					logger.log("setNonBlocking() failed. errno:%\n", strerror(errno));
					return -1;
				}

				if (!is_udp && !set_no_delay(fd))
				{
					logger.log("set_no_delay() failed errno:%\n", strerror(errno));
					return -1;
				}
			}

			if (!is_listening && connect(fd, rp->ai_addr, rp->ai_addrlen) == 1 && !would_block())
			{
				logger.log("connect() failed errno:%\n", strerror(errno));
				return -1;
			}

			if (is_listening &&
			    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(one), sizeof(one)) == -1)
			{
				logger.log("setsockopt() SO_REUSEADDR failed errno:%\n", strerror(errno));
				return -1;
			}

			if (is_listening && bind(fd, rp->ai_addr, rp->ai_addrlen) == -1)
			{
				logger.log("bind() failed errno:%\n", strerror(errno));
				return -1;
			}

			if (!is_udp && is_listening && listen(fd, MAX_TCP_SRV_BKLG) == -1)
			{
				logger.log("listen() failed errno:%\n", strerror(errno));
				return -1;
			}

			if (is_udp && ttl)
			{
				const bool multicast = atoi(ip.c_str()) & 0xE0;
				if (multicast && !set_m_cast_ttl(fd, ttl))
				{
					logger.log("set_m_cast_ttl() failed errno:%\n", strerror(errno));
					return -1;
				}

				if (multicast && !set_ttl(fd, ttl))
				{
					logger.log("set_ttl() failed errno:%\n", strerror(errno));
					return -1;
				}
			}

			if (so_timestamp_needed && !set_so_timestamp(fd))
			{
				logger.log("set_so_timestamp() failed errno:%\n", strerror(errno));
				return -1;
			}
		}

		if (res)
		{
			freeaddrinfo(res);
		}

		return fd;
	}
}