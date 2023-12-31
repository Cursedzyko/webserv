#include "Request.hpp"

Request::Request(std::multimap<std::string, Location> &l): _q(false), 
_cgi(false), _buf(new char[RECV_BUFFER_SIZE + 1]),_parseStat(STARTL),
 _errorCode(0), _locationMap(l), _bodySize(0), _isChunkSize(false),
_chunkSize(0), _isReqDone(false),  _cgiNum(0), _con(false),
 _maxBodySize(0),  _bodyH(false), _envCGI(NULL)
{
}

void Request::setResource(std::string res)
{
	_uri = res;
}

bool Request::getQ()
{
	return _q;
}

HeaderMap& Request::getBodyHeaders()
{
	return _bodyHeaders;
}

bool Request::getBodyH()
{
	return _bodyH;
}

char	*Request::getBuffer(void) const {
	return _buf;
}

bool Request::getCGIB()
{
	return _cgi;
}


void	Request::setErrorStatus(const int s) {
	_errorCode = s;
}



std::string& Request::getBody()
{
	return _body;
}

std::string& Request::getQueryString()
{
	return _queryString;
}

t_serv Request::getServ()
{
	return serv;
}

char** Request::getENV()
{
	return _envCGI;
}

bool Request::getCon()
{
	return this->_con;
}


void Request::resetRequest()
{
	_maxBodySize = 0;
	_headers.clear();
	_bodySize = 0;
	_chunkSize = 0;
	_parseStat = STARTL;
	_transEn.clear();
	_method.clear();
	_version.clear();
	_uri.clear();
	_uriCGI.clear();
	_queryString.clear();
	_body.clear();
	_tmpBuffer.clear();
	_isChunkSize = false;
	_isReqDone = false;
	_errorCode = 0;
	_q = false;
	_cgi = false;
	_location = NULL;
	_scriptPath.clear();
	_boundary.clear();
	_bodyH = false;
	_bodyHeaders.clear();
}

int Request::getErrorCode()
{
	return _errorCode;
}

Location *Request::getLoc()
{
	std::string tmp;
	std::string tmp1;
	size_t lastSlash;
	size_t len;
	bool isLastSlash;

	isLastSlash = false;
	if (_uri[_uri.length() - 1] != '/')
	{
		isLastSlash = true;
		_uri.push_back('/');
	}
	lastSlash = _uri.find_last_of("/");
	if (lastSlash == std::string::npos)
		throw ErrorException(400, "Bad Request");
	tmp = _uri.substr(0, lastSlash);
	len = std::count(_uri.begin(), _uri.end(), '/');
	for(size_t i = 0; i < len; i++)
	{
		std::multimap<std::string, Location>::iterator j = _locationMap.begin();
		for (; j != _locationMap.end(); j++) {
			if (!tmp.length()) {
				tmp = "/";
			}
			
			if (j->first != "/" && j->first[j->first.length() - 1] == '/') {
				tmp1 = j->first.substr(0, j->first.find_last_of("/"));
			} else {
				tmp1 = j->first;
			}
			
			if (tmp == tmp1) {
				if (isLastSlash) {
					_uri.erase(_uri.size() - 1);
				}
				return &j->second;
			}
		}

		lastSlash = tmp.find_last_of("/", lastSlash);
		tmp = tmp.substr(0, lastSlash);
	}
	return NULL;
}

Location *Request::getLocation()
{
	return _location;
}

void	Request::parsePercent(std::string &s)
{
	std::stringstream 	ss;
	std::string 		tmp;
	int					c;

	for (size_t i = 0; i < s.length(); i++)
	{
		if (s[i] == '%')
		{
			try
			{
				ss << std::hex << s.substr(i + 1, 2);
				ss >> c;
				tmp = s.substr(i + 3);
				s.erase(i);
				s.push_back(static_cast<char>(c));
				s.append(tmp);
				ss.clear();

			}
			catch(const std::exception& e)
			{
				throw ErrorException(400, "Bad Request");
			}
			
		}else if (s[i] == '+')
			s = s.substr(0 , i) + " " + s.substr(i + 1);
	}
}

void	Request::parseUri(void)
{
	size_t pos;

	pos = _uri.find("?");
	if (pos != std::string::npos)
	{
		_queryString = _uri.substr(pos + 1);
		_q = true;
		_uri.erase(pos);
	}
	parsePercent(_uri);
	if (_queryString.empty())
		_q = false;
}

void	Request::validateStartLine(void)
{
	_location = getLoc();
	if (_location == NULL)
	{
		throw ErrorException(404, "Not Found");
	}
	if (_version != "HTTP/1.1")
		throw ErrorException(505, "Http Version Not Supported");
	std::map<std::string, bool>::const_iterator i = _location->methods.begin();
	for(; i != _location->methods.end(); i++)
	{
		if (i->first == _method){
			if (!i->second)
				throw ErrorException(405, "Method Not Allowed");
			break;
		}
	}
	if (i == _location->methods.end())
		throw ErrorException(400, "Bad Request");
	if (_version != "HTTP/1.1")
		throw ErrorException(505, "Http Version Not Supported");
	_maxBodySize = _location->getBodySize();
	parseUri();
}

void	Request::saveStartLine(std::string startLine)
{
	size_t pos;

	if (!startLine.length())
		throw ErrorException(400, "Bad Request! No Header of the request!");
	pos = startLine.find(' ');
	if (pos == std::string::npos)
		throw ErrorException(400, "Bad Request!No Separaters in the Head of the request!");
	_method = startLine.substr(0, pos);
	startLine.erase (0, skipWhiteSpaces(startLine, pos));

	pos = startLine.find(' ');
	if (pos == std::string::npos)
		throw ErrorException(400, "Bad Request!No Separaters in the Head of the request!");
	_uri = startLine.substr(0, pos);
	startLine.erase (0, pos);
	_version = startLine;
	_version.erase(std::remove_if(_version.begin(),
		_version.end(), &isCharWhiteSpace), _version.end());


	validateStartLine();
	_parseStat = HEADERL;
}

void	Request::saveHeaderLine(std::string headerLine)
{
	size_t colPos;
	std::string headerKey;
	std::string headerVal;

	headerLine.erase(std::remove_if(headerLine.begin(),
		headerLine.end(), &isCharWhiteSpace), headerLine.end());
	if (!headerLine.length())
	{
		if (_headers.find("Host") == _headers.end())
			throw ErrorException(400, "Bad Request");
		if (_headers.find("Transfer-Encoding") == _headers.end()
			&& _headers.find("Content-Length") == _headers.end())
			_parseStat = END_STAT;
		else
			_parseStat = BODYL;
		return ;
	}
	colPos = headerLine.find(":");
	if (colPos == std::string::npos)
		throw ErrorException(400, "Bad Request! No separater \":\"");
	headerKey = headerLine.substr(0, colPos);
	headerVal = headerLine.substr(colPos + 1);
	_headers.insert(std::pair<std::string, std::string> (headerKey, headerVal));
	if (headerKey == "Content-Length")
		_bodySize = static_cast<int>(std::atol(headerVal.c_str()));
	if (headerKey == "Transfer-Encoding")
		_transEn = headerVal;
	if (headerKey == "Connection")
		_connection = headerVal;
	return ;
}

void	Request::saveStartLineHeaders(std::string &data)
{
	size_t linePos;

	linePos = data.find("\n");
	while(linePos != std::string::npos 
	&& (_parseStat != BODYL && _parseStat != END_STAT))
	{
		if (_parseStat == STARTL)
		{
			saveStartLine(data.substr(0, linePos));
			data.erase(0, linePos + 1);
		}
		if (_parseStat == HEADERL)
		{
			linePos = data.find("\n");
			saveHeaderLine(data.substr(0, linePos));
			data.erase(0, linePos + 1);
		}
		linePos = data.find("\n");
	}
}

void	Request::parseChunkSize(std::string &data)
{
	std::stringstream ss;
	size_t pos;
	
	if (!data.length())
	{
		_parseStat = END_STAT;
		return ;
	}
	pos = data.find("\r\n");
	if (pos == std::string::npos)
		return ;
	ss << std::hex << data.substr(0, pos);
	ss >> _chunkSize;
	if (!_chunkSize)
		_parseStat = END_STAT;
	_isChunkSize = true;
	data.erase(0, pos + 2);
}

void	Request::parseChunkedBody(std::string &data)
{
	size_t i = 0;

	while (i < data.length() && _chunkSize) {
		if (i > 0 && data[i] == '\n' && data[i - 1] == '\r') {
			_body.push_back('\n');
		} else if (data[i] != '\r') {
			_body.push_back(data[i]);
		}
		i++;
		_chunkSize--;
	}
	if (!_chunkSize)
	{
		_isChunkSize = false;
		i += 2;
	}
	data.erase(0, i);
}

void	Request::saveChunkedBody(std::string &data)
{
	while(_parseStat != END_STAT)
	{
		if (!_isChunkSize)
			parseChunkSize(data);
		if (_isChunkSize && _parseStat != END_STAT)
			parseChunkedBody(data);
	}
}

void	Request::saveSimpleBody(std::string &data)
{
	size_t bodySize;

	bodySize = static_cast<size_t>(std::atol(_headers["Content-Length"].c_str()));
	if (bodySize > this->_maxBodySize)
		throw ErrorException(413, "Request Entity Too Large");
	if (_body.length() + data.length() > this->_maxBodySize)
		throw ErrorException(413, "Request Entity Too Large");
    if (!_boundary.empty()) {
        size_t startPos;
        std::string endBoundary = "--" + _boundary + "--"; // The end boundary
        std::string standardBoundary = "--" + _boundary; // The standard boundary

        while ((startPos = data.find(standardBoundary)) != std::string::npos) {
            // Append everything up to the boundary to _body
            
            // Check for end boundary
            if (data.substr(startPos, endBoundary.length()) == endBoundary) {
                data.erase(startPos, endBoundary.length()); // Erase end boundary
				_parseStat = END_STAT;
				_body.append(data);
                break;
            } else {
                data.erase(startPos, standardBoundary.length()); // Erase standard boundary
            }
        }
    } 
	else {
		_body.append(data);
		data.clear();
	}

	if (!_boundary.empty())
	{
		std::string sBoundary = "--" + _boundary;
		std::string endBoundary = "--" + _boundary + "--";
		size_t pos;
		while((pos = _body.find(endBoundary)) != std::string::npos)
		{
			_body.erase(pos, endBoundary.length());
			_parseStat = END_STAT;
		}
		while((pos = _body.find(sBoundary)) != std::string::npos)
		{
			_body.erase(pos, sBoundary.length());
		}
	}


	size_t headerEndPos = _body.find("\r\n\r\n");
    if (headerEndPos != std::string::npos) {
        // Extract and save the headers in a map
        std::string headerData = _body.substr(0, headerEndPos);
        std::istringstream headerStream(headerData);
        std::string headerLine;
        while (std::getline(headerStream, headerLine, '\n')) {
            size_t colonPos = headerLine.find(':');
            if (colonPos != std::string::npos) {
                std::string key = headerLine.substr(0, colonPos);
                std::string value = headerLine.substr(colonPos + 1);
                // Remove leading and trailing whitespaces from the key and value
                key = key.substr(0, key.find_last_not_of(" \t") + 1);
                value = value.substr(value.find_first_not_of(" \t"));
                _bodyHeaders[key] = value;
				_bodyH = true;
            }
        }
		_body = _body.substr(headerEndPos + 4);
	}
	if (_body.length() == bodySize)
		_parseStat = END_STAT;
}

std::string Request::extractBoundary()
{
	if (_headers.count("Content-Type") == 0) {
        return "";
    }
	std::string contentType = _headers["Content-Type"];
	size_t startPos = contentType.find("boundary=");
	if (startPos == std::string::npos)
		return "";
	return contentType.substr(startPos + 9);
}

bool	Request::saveRequestData(ssize_t recv)
{
	std::string data;

	data = _tmpBuffer;

	_buf[recv] = '\0';
	data.append(_buf, recv);

	if (_parseStat == END_STAT)
		resetRequest();
	if (_parseStat == STARTL || _parseStat == HEADERL)
	{
		saveStartLineHeaders(data);
		_boundary = extractBoundary();
	}
	if (_parseStat == BODYL)
	{
		if (_transEn == "chunked")
			saveChunkedBody(data);
		else
			saveSimpleBody(data);
	}
	_tmpBuffer = data;
	if (_parseStat == END_STAT)
		_isReqDone = true;
	return _isReqDone;
}

std::string Request::validateURI(std::string &fullPath, uint8_t mode)
{
	std::string tmp;
	if (mode == 2)// dir
	{
		tmp = fullPath + "/" + _location->getIndex();
		if (isDirOrFile(tmp.c_str()) == 1){
			if (!access(tmp.c_str(), R_OK))
				return tmp;
		}
		if (_location->getAutoInd() == "on")
		{
			_errorCode = 1;
			return fullPath;
		}
		if (errno == EACCES || _location->getAutoInd() == "off")
			_errorCode = 403;
		else
			_errorCode = 404;
		return tmp;
	}
	else if (mode == 1) //file
	{
		if (!access(fullPath.c_str(), R_OK))
			return fullPath;
		if (errno == EACCES)
			_errorCode = 403;
		else
			_errorCode = 404;
		_errorCode = 403;
		return fullPath;
	}
	if (_method == "POST")
		_errorCode = 201;
	else
		_errorCode = 404;
	return fullPath;
}

std::string Request::getURI()
{
	std::string	fullPath;
	uint8_t mode;
	std::string tmp;

	if (_location && _location->redir.second.length())
	{
		_errorCode = _location->redir.first;
		fullPath =  _location->redir.second;
		if (fullPath.find(_location->getRoot()) == std::string::npos)
			fullPath = _location->getRoot() + fullPath;
		return fullPath;
	}
	_errorCode = 200;
	if (_cgi == true)
	{
		if (_scriptPath.find(_location->getRoot()) == std::string::npos)
		{
			fullPath = _location->getRoot() + _scriptPath;
		}
		else
		{
			fullPath = _scriptPath;
		}
	}
	else
	{
		if (_uri.find(_location->getRoot()) == std::string::npos)
			fullPath = _location->getRoot() + _uri;
		else
			fullPath = _uri;
		
	}
	for(size_t i = 0; i < fullPath.length() - 1; i++)
	{
		if (fullPath[i] == '/' && fullPath[i + 1] == '/')
			fullPath.erase(i + 1, 1);
	}
	if (fullPath[fullPath.length() - 1] == '/')
		fullPath.erase(fullPath.size() - 1);
	mode = isDirOrFile(fullPath.c_str());
	if (mode == 0 && _method != "POST")
	{
		_errorCode = 404;
		return "unkown url";
	}
	return (validateURI(fullPath, mode));
}

void Request::getUriEncodedBody()
{
	std::string			tmp;
	size_t 				size;
	size_t 				pos;
	std::pair<std::string, std::string> pair;

	tmp = _body;
	size = std::count(tmp.begin(), tmp.end(), '&');
	for(size_t i = 0; i < size; i++)
	{
		pos = tmp.find("&");
		pair.first = tmp.substr(0, tmp.find("="));
		parsePercent(pair.first);
		pair.second = tmp.substr(tmp.find("=") + 1, pos - pair.first.length() - 1);
		parsePercent(pair.second);
		_headers.insert(pair);
		tmp.erase(0 , pos + 1);
	}
}

void Request::makeEnv()
{
	_envCGI = new char*[_headers.size() + 1];

	_envCGI[_headers.size()] = NULL;
	std::map<std::string, std::string>::iterator i = _headers.begin();
	for(int j = 0; i != _headers.end(); i++, j++)
		_envCGI[j] = strdup((i->first + "=" + i->second).c_str());
}

bool  Request::checkCGI()
{
	std::multimap<std::string,std::string> tmp = _location->getCGI();
	std::string ext = "";
	if (_uri.find(".") != std::string::npos)
		ext = "." + _uri.substr(_uri.find_last_of('.') + 1);
	std::string _scriptName =  _uri.substr(_uri.find_last_of('/') + 1);
	if (tmp.empty() && ext == ".py") {
    	_cgi = false;
        throw ErrorException(500, "Server configuration error: File is present but not configured in CGI map.");
	}
	std::multimap<std::string,std::string>::const_iterator i = tmp.begin();
	for(; i != tmp.end(); i++)
	{
		if (ext == i->first)
		{
			_scriptPath = i->second + _scriptName;
			int n = access(_scriptPath.c_str(), X_OK);
			if (n == -1)
			{
				std::cerr << "Error: " << strerror(errno) << std::endl;
				_cgiNum = -1;
				break;
			}
			_cgiNum++;
		}
	}
	if (_cgiNum > 0)
	{
		_cgi = true;
		_uriCGI = getURI();
		getUriEncodedBody();
		_headers.insert(std::pair<std::string, std::string>("QUERY_STRING", _queryString));
		_headers.insert(std::pair<std::string, std::string>("REQUEST_METHOD", _method));
		_headers.insert(std::pair<std::string, std::string>("PATH_INFO", _uriCGI));
		_headers.insert(std::pair<std::string, std::string>("PATH_TRANSLATED", _location->getRoot()));
		_headers.insert(std::pair<std::string, std::string>("SERVER_PROTOCOL", "HTTP/1.1"));
		makeEnv();
	}
	else if (_cgiNum < 0){
		_cgi = false;
		throw ErrorException(502, "Bad Gateway");
	}
	else
	{
		_cgi = false;
		_errorCode = 200;
		_uri = getURI();
		if (_uri.find(_location->getRoot()) == std::string::npos)
			_uri = _location->getRoot() + _uri;
		for(size_t i = 0; i < _uri.length() - 1; i++)
		{
			if (_uri[i] == '/' && _uri[i + 1] == '/')
				_uri.erase(i + 1, 1);
		}
	}
	if (_connection == "close")
	{
		_con = true;
	}
	return _cgi;
}


void Request::print()
{
	std::cout << "Method: " << getMethod() << "\n";
    std::cout << "Resource: " << getResource() << "\n";
    std::cout << "Version: " << getVersion() << "\n";
	std::cout << "QueryString: " << getQueryString() << "\n" << "\n";


	std::cout << "Boundary: " << _boundary << "\n" << "\n";
	std::cout << "Body: " << getBody() << "\n" << "\n";
	std::cout << "Error_code: " << _errorCode << std::endl;

	HeaderMap tmp = getHeaders();
	std::cout << "THISSS " << _errorCode << std::endl;

	std::cout << "Headers: " << std::endl;
	for (HeaderMap::iterator it = tmp.begin(); it != tmp.end(); ++it) {
		std::cout << it->first << " = " << it->second << "\n";
	}

	HeaderMap tmp1 = getBodyHeaders();
	std::cout << "Body Header Bool :" << getBodyH() << std::endl;
	std::cout << "Body Header: " << std::endl;
	for (HeaderMap::iterator it = tmp1.begin(); it != tmp1.end(); ++it) {
		std::cout << it->first << " = " << it->second << "\n";
	}
}

std::string& Request::getMethod()
{
	return this->_method;
}
std::string& Request::getResource()
{
	return this->_uri;
}
std::string& Request::getVersion()
{
	return this->_version;
}
HeaderMap& Request::getHeaders()
{
	return this->_headers;
}

std::string& Request::getUriCGI()
{
	return _uriCGI;
}

Request::~Request()
{

}
