#ifndef SERVER_HPP
# define SERVER_HPP

#include "mainIn.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <cstdlib>
#include <vector>
#include <map>
#include <algorithm>
#include <poll.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "Client.hpp"
#include "mainIn.hpp"
#include "Request.hpp"

class Server
{
private:
	std::string readFile(const std::string & filename);
	std::vector<t_serv> servers;


public:
	Server();
	~Server();

	class returnError : public std::exception
	{
		public:
			virtual const char *what() const throw()
			{
				return ("exception: CGI\n");
			}
	};

	std::map<int, Client*> fd_to_clients;
	std::map<int, Client*>::iterator client_it;
	void setUp(std::vector<t_serv>& s);
	bool launchCgi(Client *client);
    void sendHTMLResponse(Client *client, std::string filepath);
	bool sendDeleteResponse(class Client *client, std::string filepath);
	void sendPostResponse(class Client *client);

};
#endif
