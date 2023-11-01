#include "Server.hpp"
#include <cstdio>

// #include <iostream>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <signal.h>



Server::Server()
{
    servers = std::vector<t_serv>();
}

Server::~Server(){
}

#define WRITE_END 1
#define READ_END 0

// int handleFileUpload(const std::string& filename, const std::string& fileContent, const size_t file_size, const size_t upload_header_size)
// {
//     std::string full_path = filename;
//     // std::cout << "PLOAD PATH " << std::endl;

//     // std::cout << filename << std::endl;

//     std::ofstream outputFile(filename.c_str(), std::ios::binary | std::ios::trunc);
//     // std::ofstream outputFile(filename, std::ios::binary);
//     if (outputFile.is_open()) {
//         // outputFile << fileContent;
//     // size_t file_size = ds_clients.at(client_fd).request.rfind(boundary) - upload_header_size;

//     // Write the uploaded file data to the file
//     outputFile.write(fileContent.substr(upload_header_size, file_size).c_str(), file_size);

//         outputFile.close();
//         // std::cout << "File uploaded successfully.\n";
//         return (true);
//     } else {
//         std::cerr << "Failed to upload the file.\n";
//         return (false);
//     }
// }


std::string extractBoundary(const std::string& contentTypeHeader)
{
    std::string boundary;

    size_t boundaryPos = contentTypeHeader.find("boundary=");
    if (boundaryPos != std::string::npos)
    {
        boundaryPos += 9; // Move to the start of the boundary
        size_t semicolonPos = contentTypeHeader.find(";", boundaryPos);
        if (semicolonPos != std::string::npos) {
            // Extract the boundary value up to the semicolon
            boundary = contentTypeHeader.substr(boundaryPos, semicolonPos - boundaryPos);
        } else {
            // If no semicolon found, extract the boundary value until the end of the string
            boundary = contentTypeHeader.substr(boundaryPos);
        }
    }
    return boundary;
}




std::string extractFilename(const std::string& contentDispositionHeader) {
    std::string filename;

    size_t filenamePos = contentDispositionHeader.find("filename=");
    if (filenamePos != std::string::npos) {
        filenamePos += 10; // Move to the start of the filename
        size_t endPos = contentDispositionHeader.find("\"", filenamePos);
        if (endPos != std::string::npos) {
            // Extract the filename
            filename = contentDispositionHeader.substr(filenamePos, endPos - filenamePos);
        }
    }

    return filename;
}

// void Server::sendPostResponse(class Client *client, int fd, std::string filepath)
//  {
//     // std::cout << "IN POST RESP ----- \n\n";

//     size_t upload_header_size;
//     std::string upload_header;
//     std::string filename = extractFilename(client->getReq().getBody());
//     filename = client->getReq().getLoc()->root + filename;
//     upload_header_size = client->getReq().getBody().find("\r\n\r\n") + 4;
//     upload_header = client->getReq().getBody().substr(0, upload_header_size);

//     // std::ofstream createFile(file_path.c_str(), std::ios::binary | std::ios::trunc);

//     std::map<std::string, std::string> tmp;
//     std::ifstream dmm(client->getReq().getBody());

//     std::string boundary = extractBoundary(client->getReq().getHeaders()["Content-Type"]);


//     tmp = client->getReq().getHeaders();
//     std::string requestBody = client->getReq().getBody();

//     size_t file_size = requestBody.rfind("\r\n--" + boundary + "--") - upload_header_size;

//     if (handleFileUpload(filename, requestBody, file_size, upload_header_size))
// 	{
// 		client->getResp()->status_code = "201 Created";
// 		client->getResp()->content_type = "text/plain";
// 		client->getResp()->body = "File was uploaded succesfully";
// 	}
// 	else
// 	{
// 		client->getResp()->exec_err_code = 500;
// 		dmm.close();
// 		throw(returnError());

// 	}
// 	dmm.close();
//     // client.sendResponse("text/plain");
// }

int handleFileUpload(const std::string& filename, const std::string& fileContent, const size_t file_size)
{
    // std::string full_path = filename;

    std::ofstream outputFile(filename.c_str(), std::ios::binary | std::ios::trunc);
    if (outputFile.is_open()) {

    // Write the uploaded file data to the file
    	outputFile.write(fileContent.c_str(), file_size);

        outputFile.close();
        // std::cout << "File uploaded successfully.\n";
        return (true);
    } else {
        std::cerr << "Failed to upload the file.\n";
        return (false);
    }
}

void Server::sendPostResponse(class Client *client, int fd, std::string filepath)
 {
    // std::cout << "IN POST RESP ----- \n\n";

    // std::ofstream createFile(file_path.c_str(), std::ios::binary | std::ios::trunc);

    // std::ifstream dmm(client->getReq().getBody());

    // std::string boundary = extractBoundary(client->getReq().getHeaders()["Content-Type"]);


	std::string filename = client->getReq().getBodyHeaders()["filename"];
    std::string requestBody = client->getReq().getBody();

    // size_t file_size = requestBody.rfind("\r\n--" + boundary + "--") - upload_header_size;

    if (handleFileUpload(filename, requestBody, requestBody.size()))
	{
		client->getResp()->status_code = "201 Created";
		client->getResp()->content_type = "text/plain";
		client->getResp()->body = "File was uploaded succesfully";
	}
	else
	{
		client->getResp()->exec_err_code = 500;
		// dmm.close();
		throw(returnError());

	}
	// dmm.close();
    // client.sendResponse("text/plain");
}

bool Server::sendDeleteResponse(class Client *client, int fd, std::string filepath)
{
	int i = std::remove(filepath.c_str());
	if (i != 0)
	{
		// client->error_code = 409;
		// client->getReq().setErrorStatus(409);
		// client->checkError();
		client->getResp()->exec_err_code = 409;
		throw(returnError());

		// fds_clients.at(client_fd).setError("409");
		return 1;
	}
    client->getResp()->status_code = "204 No Content";
	client->getResp()->body = "Deleted successfully";

	client->getResp()->content_type = "text/html";
	// additional_info.clear();
	// client->getResp()->content_len = 0;

    // client->getResp()->sendResponse("text/plain");
	// client->getResp()->response_complete = true;
	return 0;
}

void Server::sendHTMLResponse(class Client *client, int fd, std::string filepath)
{
    std::string content_type;
    // Read the content of the HTML file
    // std::cout << " \n In html \n";
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filepath << std::endl;
    }

    std::string extention = filepath.substr(filepath.rfind(".") + 1);
    if (extention == "html")
        content_type = "text/html";
    else if (extention == "css")
        content_type = "text/css";
    else if (extention == "txt")
        content_type = "text/plain";
    else if (extention == "ico")
        content_type = "image/x-icon";
    else if (extention == "jpg" || extention == "jpeg")
        content_type = "image/jpeg";
    else if (extention == "png")
        content_type = "image/png";
    else if (extention == "gif")
        content_type = "image/gif";
    else if (extention == "pdf")
        content_type = "application/pdf";
    else if (extention == "mp3")
        content_type = "audio/mpeg";
    else if (extention == "mp4")
        content_type = "audio/mpeg";
    else if (extention == "avi")
        content_type = "video/x-msvideo";
    else
        content_type = "application/octet-stream";

    // client->filename = filepath;
    client->getResp()->content_type = content_type;
    client->getResp()->filename = filepath;

	if (client->getReq().getErrorCode() == 301)
	{
		client->getResp()->status_code = "301";
	}

    // client.fd = fd;
    // client.sendResponse(content_type);

    file.close();
	// std::cerr << client->filename << std::endl;

    // std::cerr << "\n after file closing\n" << std::endl;
}

static volatile sig_atomic_t timeoutOccurred = 0;

void handleTimeout(int signum) {
    (void)signum;
    // std::cerr << "\n\n ---- timeout handling   \n";

    timeoutOccurred = 1;
}


void cleanEnv(Client *client)
{

    if (client->getReq().getENV() != nullptr) {
        std::cerr << "freeing env in des "  << std::endl;
        for (size_t i = 0; client->getReq().getENV()[i] != nullptr; ++i) {
            if (client->getReq().getENV()[i])
                free(client->getReq().getENV()[i]);
        }
        delete[] client->getReq().getENV();
        // client->getReq()._envCGI = nullptr;
    }
}

bool Server::launchCgi(Client *client, int fd)
{

    const int timeoutDuration = 3;

    int infile = 0;
	client->getResp()->exec_err_code = 500;

    // std::cerr << "\n\n ***** in CGI   \n";


    std::string string_filename = "output_file" + client->getClienIP();
	client->getResp()->filename = string_filename;

    const char* out_filename = string_filename.c_str();

    int outfile = open(out_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (outfile == -1) {
        std::cerr << "cgi: Error opening the outfile.\n";
		return 1;

    }

    int pipe_d[2];
    if (pipe(pipe_d) == -1) {
        std::cerr << "Pipe Error\n";
		return 1;

    }

    write(pipe_d[WRITE_END], client->getReq().getBody().c_str(), client->getReq().getBody().size());
    close(pipe_d[WRITE_END]);

    int cgi_pid = fork();
    if (cgi_pid < 0) {
        close(pipe_d[READ_END]);
        close(outfile);
        std::cerr << "Error with fork\n";
		return 1;
    }

    if (cgi_pid == 0) {
        timeoutOccurred = 0;

        dup2(outfile, STDOUT_FILENO);
        close(outfile);

        if (client->getQuer() == false) {
            dup2(infile, STDIN_FILENO);
            close(infile);
        }
        char* script_path = (char*)(client->getReq().getUriCGI().c_str());
        
		// std::cerr << "sSCRIPT PATH" << "\n";

        // std::cerr << script_path << "\n";

        const char* path_to_py = "/usr/local/bin/python3";
        // /Users/cgreenpo/our_webserv/Config/cgi-bin/script_get.py
        // char* script_path = "/Users/kris/our_webserv2/Config/cgi-bin/script_timeout.py"; // warning


        char* _args[] = {const_cast<char*>(path_to_py), const_cast<char*>(script_path), nullptr};

        dup2(pipe_d[READ_END], STDIN_FILENO);
        close(pipe_d[READ_END]);

        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = handleTimeout;

        if (sigaction(SIGALRM, &sa, NULL) == -1) {
            perror("sigaction");
			return 1;

        }
        alarm(timeoutDuration);

        if ((execve(_args[0], _args, client->getReq().getENV())) == -1) {
            std::cerr << "\n cgi: error with execution\n";
			return 1;
        }

    }

    close(pipe_d[READ_END]);
    int status;
    pid_t terminatedPid = waitpid(cgi_pid, &status, 0);
    if (terminatedPid == -1) {
        std::cerr << "cgi: error with process handling\n";
		return 1;
    }

    if (WIFEXITED(status))
    {
        std::cerr << "Child process exited with status: " << WEXITSTATUS(status) << std::endl;
    }
    else if (WIFSIGNALED(status))
    {
        std::cerr << "Child process terminated due to signal: " << WTERMSIG(status) << std::endl;
		return 1;
    }
    client->getResp()->content_type = "text/html";

    alarm(0);
	return 0;
}


void removeSocket(int fd, std::vector<int>& connected_fds,
                  std::vector<int>& sockets_to_remove,
                  std::map<int, Client*>& fd_to_clients,
                  std::vector<pollfd>& pollfds,
                  const std::vector<int>& listenfds) {

    // Remove the socket from connected_fds
    for (size_t j = 0; j < connected_fds.size(); ++j) {
        if (connected_fds[j] == fd) {
            connected_fds.erase(connected_fds.begin() + j);
            break;
        }
    }

    // Remove the socket from sockets_to_remove
    std::vector<int>::iterator it = std::find(sockets_to_remove.begin(), sockets_to_remove.end(), fd);
    if (it != sockets_to_remove.end()) {
        sockets_to_remove.erase(it);
    }

    // Remove the socket from fd_to_clients
    std::map<int, Client*>::iterator clp = fd_to_clients.find(fd);
    if (clp != fd_to_clients.end()) {
		delete clp->second->_resp;
        delete clp->second;
        fd_to_clients.erase(clp);
    }

    // Remove the socket from pollfds
    // bool deleted = false;
    for (std::vector<pollfd>::iterator it = pollfds.begin() + listenfds.size(); it != pollfds.end(); ++it) {
        if (it->fd && it->fd == fd) {
            pollfds.erase(it);
            // deleted = true;
            break;
        }
    }
}

void Server::setUp(std::vector<t_serv>& s)
{
    servers = s;
    std::vector<int> port_numbers;
	std::map<int, int> fd_to_port;
	std::map<int, t_serv*> port_to_serv;

    int i, j;

    i = 0;
    // std::cout << "Server size:" << servers.size() << std::endl;
    while(i < servers.size())
    {
        j = 0;
        while (j < servers[i].port.size())
        {
            port_numbers.push_back((int)servers[i].port[j]);
            port_to_serv.insert(std::make_pair((int)servers[i].port[j], &(servers[i])));
            j++;
        }
        i++;
    }


    std::vector<int> listenfds;
    struct sockaddr_in servaddr;

    std::vector<pollfd> pollfds;
    std::map<int, int> conn_to_listen;

    for (size_t i = 0; i < port_numbers.size(); ++i) {
        int listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if (listenfd < 0) 
        {
            std::cerr << "Failed to create socket. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }

        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(port_numbers[i]);
		int optval = 1;
		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) 
        {
            std::cerr << "Failed to bind to port " << port_numbers[i] << ". errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }

        if (listen(listenfd, 10) < 0) {
            std::cerr << "Failed to listen on socket. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }
        fd_to_port.insert(std::make_pair(listenfd, port_numbers[i]));
        pollfd listen_pollfd;
        listen_pollfd.fd = listenfd;
        listen_pollfd.events = POLLIN;  // Check for incoming data
        listen_pollfd.revents = 0;
        pollfds.push_back(listen_pollfd);
        listenfds.push_back(listenfd);
    }
    std::vector<int> connected_fds;
    while (1)
	{

		// std::cerr << "in main LOOP" << '\n';

        int activity = poll(&pollfds[0], pollfds.size(), -1); // Wait indefinitely until an event occurs
        if (activity < 0) {
            std::cerr << "Error in poll. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }
        for (size_t i = 0; i < listenfds.size(); ++i)
		{
			// std::cerr << "in main LOOP" << '\n';
            if (pollfds[i].revents & POLLIN) 
            {
                std::cout << "\n\nWaiting for a connection on port " << port_numbers[i] << std::endl;
                int connfd = accept(listenfds[i], NULL, NULL);
                if (connfd < 0) 
                {
                    std::cerr << "Failed to accept connection. errno: " << errno << std::endl;
                    exit(EXIT_FAILURE);
                }
                connected_fds.push_back(connfd);
                // std::cout << "Client connected on port: " << port_numbers[i] << std::endl;
                pollfd new_pollfd; // Add the new connected socket to the pollfds array
                new_pollfd.fd = connfd;
                new_pollfd.events = POLLIN; // Check for incoming data
                pollfds.push_back(new_pollfd);
                conn_to_listen.insert(std::make_pair(connfd, listenfds[i]));

            }
        }
		// std::unordered_map<int, Client>::iterator client_it;

        std::vector<int> sockets_to_remove;// Check connected client sockets for incoming data and handle them separately
        for (size_t i = listenfds.size(); i < pollfds.size(); ++i)
		{
            // std::cerr << "\nIN THE LOOP" << '\n';
            int fd = pollfds[i].fd;
			if (pollfds[i].revents & POLLIN & fd_to_clients.count(pollfds[i].fd) > 0)
			{
				try
				{
					fd_to_clients.find(fd)->second->readRequest();
					if (fd_to_clients.find(fd)->second->getToServe() == true)
						pollfds[i].events = POLLOUT;
				}
				catch(const std::exception& e)
				{
					std::cerr << e.what() << '\n';
					fd_to_clients.find(fd)->second->checkError();
					sockets_to_remove.push_back(pollfds[i].fd);
				}
				

			}
            else if (pollfds[i].revents & POLLIN & fd_to_clients.count(pollfds[i].fd) == 0) 
            {

                char client_ip[INET_ADDRSTRLEN];
                if(inet_ntop(AF_INET, &(servaddr.sin_addr), client_ip, INET_ADDRSTRLEN) == NULL)
                {
                    std::cerr << "Error converting IP address to string: " << strerror(errno) << std::endl;
                    exit(1);// need to change
                }
                // std::cout << "HEL YEAH!" << std::endl;
                int listnfd_tmp = conn_to_listen[pollfds[i].fd];

                int port_tmp = fd_to_port[listnfd_tmp];
                t_serv  *serv_tmp = port_to_serv[port_tmp];
                if (serv_tmp)
				{
                    Client* myCl = new Client(pollfds[i].fd, client_ip, *serv_tmp);
                    Response* myResp = new Response();
                    myCl->_resp = myResp;
					fd_to_clients.insert(std::make_pair(pollfds[i].fd, myCl));
					if (myCl->getIsClosed() == true)
					{
						//REMOVE THE CLIENT IF IS_CLOSED == TRUE
						sockets_to_remove.push_back(pollfds[i].fd);
					}
					else
					{

						try
						{

							myCl->readRequest();
							myCl->print();
							myCl->pollstruct = &(pollfds[i]);

							if (myCl->checkError())
							{
								// std::cout << "I Am HERE \n";
								if (myCl->getReq().getCGIB())
								{
									// cleanEnv(myCl);
									if (launchCgi(myCl, fd))
									{
										myCl->getResp()->exec_err = true;
										remove(myCl->getResp()->filename.c_str());
										throw(returnError());
									}
								}
								else if (myCl->getReq().getMethod() == "GET")
									sendHTMLResponse(myCl, fd, myCl->getReq().getResource());
								else if (myCl->getReq().getMethod() == "POST")
									sendPostResponse(myCl, fd, myCl->getReq().getResource());
								else if (myCl->getReq().getMethod() == "DELETE")
									sendDeleteResponse(myCl, fd, myCl->getReq().getResource());
								
								if (myCl->getToServe() == true)
									pollfds[i].events = POLLOUT;
								myCl->getResp()->_target_fd = fd;

							}
						}
						catch (const std::exception& e)
						{
							std::cerr << e.what() << '\n';
							myCl->checkError();
							sockets_to_remove.push_back(pollfds[i].fd);

						}
					}
                }
            }
			else if (pollfds[i].revents & POLLOUT)
            {

				client_it  = this->fd_to_clients.find(fd);
				if (client_it != this->fd_to_clients.end() && !client_it->second->getResp()->response_complete)
				{
					// std::cerr << "POLLOUTTTT send" << '\n';
                	if (client_it->second->getResp()->sendResponse(client_it->second->getResp()->content_type) == 1)
					{
						client_it->second->getResp()->exec_err_code = 500;
						client_it->second->checkError();
						sockets_to_remove.push_back(pollfds[i].fd);

					}

					client_it = fd_to_clients.end();
					// exit(0);
                }
				else
				{
					// std::cerr << "POLLOUTTTT remove" << '\n';

					if (client_it->second->getReq().getCGIB())
					{
						remove(client_it->second->getResp()->filename.c_str());
						std::cerr << client_it->second->getResp()->filename.c_str() << '\n';
					}
					sockets_to_remove.push_back(pollfds[i].fd);

				}

            }

        }
 		for (size_t i = 0; i < sockets_to_remove.size(); ++i)
		{
        	// std::cerr << "in Removing" << '\n';

        	int fd = sockets_to_remove[i];
			if (sockets_to_remove.size() > 1)
				i--;
        	removeSocket(fd, connected_fds, sockets_to_remove, fd_to_clients, pollfds, listenfds);
            close(fd);

   		}
     }
}
