#ifndef SOCKET_H_
#define SOCKET_H_

// Socket headers
// TODO: new repo. general ipc headers
// TODO: switch for different os
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// Standard headers
#include <iostream>
#include <stdexcept>

// SocketStream class
class SocketStream {
	int		_socket;
	std::string	_host;
	std::string	_serv;
public:
	// Constructor
	SocketStream(int socket, std::string host, std::string serv) :
		_socket(socket), _host(host), _serv(serv) {}
	
	// Destructor
	~SocketStream() {
		close(_socket);
	}

	// As a boolean
	operator bool() {
		return (_socket >= 0);
	}

	// Sending data
	// TODO:virtual functions from some virtual base class (Stream)
	int send(const char *buf, int len) {
		return ::send(_socket, buf, len, 0);
	}

	int send(const std::string &buf) {
		return send(buf.c_str(), buf.length());
	}

	template <class T>
	int send(const T &buf) {
		return send((char *) &buf, sizeof(T));
	}

	template <class T>
	int send(const T *buf, int len) {
		return send((char *) buf, len * sizeof(T));
	}

	// Receiving data
	int recv(char *buf, int len) {
		return ::recv(_socket, buf, len, 0);
	}

	int recv(std::string &buf) {
		// TODO: constant for buffer size
		char buffer[1024] = {0};
		int ret = recv(buffer, sizeof(buffer));
		buf = buffer;
		return ret;
	}

	template <class T>
	int recv(T &buf) {
		return recv((char *) &buf, sizeof(T));
	}

	template <class T>
	int recv(T *buf, int len) {
		return recv((char *) buf, len * sizeof(T));
	}

	// TODO: template <class T> T recv that throw exception if not read

	// Get host and service
	std::string host() {
		return _host;
	}

	std::string serv() {
		return _serv;
	}
};

// TODO: fifo class

// Socket class
class Socket {
	int		_fd;
	int		_port;

	sockaddr_in	_addr;
	socklen_t	_addr_len;

	// Creating listening socket
	// TODO: logger class
	void _mk_listener() {
		_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (_fd < 0)
			throw "Error creating socket";
	}
public:
	Socket() {}

	// Close listening socket
	void close() {
		::close(_fd);
	}

	// Bind socket to port
	void bind(int port) {
		_mk_listener();	// TODO: in constructor?

		// Create socket address
		int optval = 1;
		int ret = setsockopt(
			_fd,
			SOL_SOCKET,
			SO_REUSEADDR,
			&optval,
			sizeof(optval)
		);

		if (ret < 0) {
			throw "Error setting socket options";
		}

		// Address description
		_port = port;
		_addr = sockaddr_in {
			.sin_family = AF_INET,
			.sin_port = htons(port),
			.sin_addr = {
				.s_addr = INADDR_ANY
			}
		};

		_addr_len = sizeof(_addr);

		// Bind socket to port
		ret = ::bind(_fd, (sockaddr *) &_addr, sizeof(_addr));
		if (ret < 0) {
			throw std::runtime_error("Socket: Error binding socket");
		}

		// TODO: pass backlog
		ret = listen(_fd, 10);

		if (ret < 0) {
			throw "Error listening";
		}
	}

	// Accepting connection
	SocketStream accept() {
		int nsock = ::accept(
			_fd,
			(sockaddr *) &_addr,
			(socklen_t *) &_addr_len
		);

		char host[NI_MAXHOST];
		char serv[NI_MAXSERV];
		getnameinfo(
			(sockaddr *) &_addr,
			_addr_len,
			host,
			NI_MAXHOST,
			serv,
			NI_MAXSERV,
			0
		);
		
		return SocketStream(nsock, host, serv);
	}

	// Get host name
	std::string hostname() {
		char hostname[1024] = {0};
		gethostname(hostname, sizeof(hostname));
		return hostname;
	}

	// Get port
	int port() {
		return _port;
	}
};

#endif
