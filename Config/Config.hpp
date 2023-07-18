#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

# define RED "\033[31m"
# define NORMAL "\033[0m"

	typedef struct s_serv
	{
		std::string		name;
		std::string 	host;
		int				port;
	}	t_serv;

class Config
{
private:
	std::vector<std::string> configRead(std::string fileName);
	t_serv parseTokens(size_t& i, std::vector<std::string>& tokens);
	size_t findNth(const std::vector<std::string> s, size_t i);
	void initServer(t_serv &t);
	std::vector<t_serv> servers;
public:
	Config(/* args */);
	~Config();
	int parse(std::string fileName);
};

Config::Config(/* args */)
{
}

Config::~Config()
{
}


std::vector<std::string> Config::configRead(std::string fileName)
{
	std::ifstream file(fileName.c_str());
	std::string buf;
	std::string cbuf;

	if (!file.is_open()) {
		std::cerr << "Config file not found\n";
		exit(1);
	}
	while (std::getline(file, buf)) {
		cbuf += buf + '\n';
	}

	char* tmp = new char[cbuf.length() + 1];
	std::strcpy(tmp, cbuf.c_str());

	std::vector<std::string> tokens;
	char* token = std::strtok(tmp, "\n\t");

	while (token != NULL) {
		tokens.push_back(token);
		token = std::strtok(NULL, "\n\t");
	}
	for (size_t i = 0; i < tokens.size(); i++) {
		size_t pos = tokens[i].find_first_not_of(' ');
		if (pos != std::string::npos){
			tokens[i].erase(0, pos);
		}
		if (tokens[i].find("server") != std::string::npos)
			trimSp(tokens[i]);
	}
	delete[] tmp;
	return (tokens);
}

void trimSp(std::string& s)
{
	for(size_t j = 0; j < s.size(); j++)
	{
		if (s[j] == ' ') {
    		s.erase(j, 1);
			--j;
       	}
	}
}

void Config::initServer(t_serv &t)
{
	t.name = "";
	t.host = "";
	t.port = 0;
	// t.logFile = "";
}

size_t Config::findNth(const std::vector<std::string> s, size_t i)
{
	for(; i < s.size(); i++)
	{
		if (s[i] == "server")
		{
			// std::cout << s[i] << std::endl;
			return (i - 1);
		}
	}
	return i;
}

void Config::valueForServer(std::vector<std::string> tokens, size_t end, size_t start, t_serv& t)
{
	for(; start < end; start++)
	{
		if (tokens[start].find("host:") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("host:") + strlen("host:"));
			trimSp(v);
			t.host = v;
		}
		if (tokens[start].find("server_name:") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("server_name:") + strlen("server_name:"));
			trimSp(v);
			t.name = v;
		}
		if (tokens[start].find("listen:") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("listen:") + strlen("listen:"));
			trimSp(v);
			t.port = atoi(v.c_str());
		}
		/// fininsh'
		exit(1);
	}
}

t_serv Config::parseTokens(size_t& i, std::vector<std::string>& tokens)
{
	t_serv server;
	initServer(server);

	// std::string blockServ;
	size_t pos;
	if ((pos = findNth(tokens, i)))
	{
		valueForServer(tokens, pos, i, server);
		i = pos;
	}
	else{
		// blockServ = tokens.substr(i, tokens.size());
		i = pos;
	}
	return server; // 
}

int Config::parse(std::string fileName)
{
	std::vector<std::string> tokens = configRead(fileName);
	// for(size_t i = 0; i < tokens.size(); i++)
	// {
	// 	std::cout << tokens[i] << std::endl;
	// }
	for(size_t i = 0; i < tokens.size(); i++)
	{
		if (tokens[i] == "server")
		{
			++i;
			if (tokens[i] != "{")
			{
				std::cerr << RED << "Error: expected '{' after server directive." << NORMAL << std::endl;
				exit(1);
			}
			++i;
			t_serv server = parseTokens(i, tokens);
			if (tokens[i] != "}")
			{
				std::cerr << RED << "Error: expected '}' after server directive." << NORMAL << std::endl;
				exit(1);
			}
			++i;
			this->servers.push_back(server);
			exit(1);
		}
		else
		{
			std::cerr << RED << "Error: unknown directive [" << tokens[i] << "]" << NORMAL << std::endl;
			exit(1);
		}

	}
	exit(0);
}

#endif