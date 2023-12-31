#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "mainIn.hpp"

typedef std::map<std::string, std::string> HeaderMap;

struct Location;


class Request 
{
	private:
		std::string _method;
		std::string _uri;
		std::string _version;
		std::string _queryString;
		std::string _body;
		t_serv serv;
		bool _q;
		bool _cgi;

		HeaderMap _headers;
		char *_buf;
		std::string _tmpBuffer;
		int _parseStat;
		int _errorCode;
		std::multimap<std::string, Location> &_locationMap;
		Location	*_location;
		int _bodySize;
		std::string _transEn;
		bool _isChunkSize;
		int _chunkSize;
		bool _isReqDone;
		int _cgiNum;
		std::string _uriCGI;
		std::string _scriptPath;
		std::string _connection;
		bool _con;
		size_t _maxBodySize;
		std::string _boundary;
		HeaderMap _bodyHeaders;
		bool _bodyH;

		void	resetRequest();
		void	saveStartLineHeaders(std::string &data);
		void	saveStartLine(std::string startLine);
		void	validateStartLine(void);
		void	parseUri(void);
		void	parsePercent(std::string &s);
		void	saveHeaderLine(std::string headerLine);
		void	saveChunkedBody(std::string &data);
		void	parseChunkSize(std::string &data);
		void	parseChunkedBody(std::string &data);
		void	saveSimpleBody(std::string &data);
		void getUriEncodedBody();
		void makeEnv();
		std::string extractBoundary();
	public:

		char **_envCGI;

		Location *getLoc();
		Request(std::multimap<std::string, Location> &l);
		~Request();
		bool getQ();
		std::string& getMethod();
		std::string& getResource();
		void setResource(std::string res);
		std::string& getVersion();
		std::string& getBody();
		std::string& getQueryString();
		std::string& getUriCGI();
		int getErrorCode();
		bool getCGIB();
		int getPortOfReq();
		t_serv getServ();
		HeaderMap& getHeaders();
		HeaderMap& getBodyHeaders();
		bool getBodyH();
		char** getENV();
		void print();
		bool getCon();
		Location *getLocation();
		

		char	*getBuffer(void) const;
		bool	saveRequestData(ssize_t recv);
		void	setErrorStatus(const int s);
		bool  checkCGI();
		std::string getURI();
		std::string validateURI(std::string &fullPath, uint8_t mode);
};

#endif
