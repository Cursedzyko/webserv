#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <poll.h>


#define BUFF_SIZE 8000


class Response {
public:
    Response();

    void setStatus(int statusCode);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);
	void setFilename(std::string &name);
	bool sendResponse(std::string content_type);
	void setFd(int fd);

	pollfd	*pollstruct;

	bool post_done;

	std::string	status_code;
	std::string	content_type;
	size_t	content_len;
	std::string	additional_info;

	std::string header;
	std::streampos	position;
	bool 	header_sent;
	bool	response_complete;
	bool 	is_chunked;
    std::string body;
	int _target_fd;
    std::string filename;
	bool exec_err;
	int	exec_err_code;

};


#endif // RESPONSE_H
