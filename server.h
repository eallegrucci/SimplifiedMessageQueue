#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <cassert>
#include <sstream>
#include "MessageQueue.h"
#include "Exchange.h"
#include "client.h"

class Server {
	std::string _name;
	MessageQueue _queue;
	Exchange _proxieQueue;
	std::map<std::string, Client> _linkedQueues;
	bool _isExchange;
	struct sockaddr_in _serv_addr;
	int _listenfd;
public:
	Server(char *&name, char *&type);
	void addLinkedQueue(std::string name);
	void putQueue(
};

using namespace std;

Server::Server(char *&name, char *&type)
{
	_name = name;

	if (strstr(type, "queue"))
	{
		_isExchange = false;
		_queue = MessageQueue(name);
		_proxieQueue = NULL;
	}
	else
	{
		_isExchange = true;
		_queue = NULL;
		_proxieQueue = Exchange(name);
	}

	_serv_addr.sin_family = AF_INET;
	_serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	//change port number check if its occupied
	_serv_addr.sin_port = htons(5000); 

	// create socket
	if((_listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket error" << endl;
	}
	
	int i = 0;
	// while the ind is unsuccessful change the port number
	// exit the loop once the bind is successful or after 1000 unsuccessful tries
	while(bind(_listenfd, (struct sockaddr*)&_serv_addr, sizeof(_serv_addr)) < 0)
	{
		if (i > 1000)
		{
			cout << "bind error" << endl;
		}
		_serv_addr.sin_port++;
		i++;
	}

	// listen for the connection
	if (listen(_listenfd, 10) < 0)
	{
		cout << "listen error" << endl;
	}

	cout << "port number: " << _serv_addr.sin_port << endl;
}

void Server::addLinkedQueue(string input)
{
	// add the new Server to the linkedQueues vector
	string command, name, ipAddr, portNum;
	// add a new Server item to the linkedQueues list
	istringstream iss(input);
	iss >> command >> name >> ipAddr >> portNum;
	stringstream pNum(portNum);
	stringstream ip(ipAddr);
	// create a new client linked to the new queue
	Client client = Client(ip, pNum);	
	_linkedQueues.insert(pair<string, Client>(name.c_str(), client.c_str()));
	cout << name << " linked to " << _name << " successfully" << endl;

}

// putQueue(Server, char *)
// turns the proxie Server into a client so it can communicate with
// other servers and send the "message" to be placed into the queues
// specified by the client
void Server::putQueue(Client c, char *info)
{
	int sockfd = 0;
	
	sockaddr_in s = c.getAddr();
	// create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket error" << endl;
		return;
	}
	
	// connect
	if (connect(sockfd, (struct sockaddr *)&s, sizeof(s) < 0)
	{
		cout << "connect error: " << strerror(errno) << endl;
		return;
	}
	
	write(sockfd, info, strlen(info));
	
	close(sockfd);
}

// listQueue(Server, char *, char *)
// turns this proxie server into a client so it can communicate with other
// queues and requesr the messages count of the specified queue from the client
void Server::listQueue(Client c, char *info, char *count)
{
	int sockfd = 0;
	
	sockaddr_in s = c.getAddr();
	// create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket error" << endl;
		return;
	}
	
	// connect
	if (connect(sockfd, (struct sockaddr *)&s, sizeof(s)) < 0)
	{
		cout << "connect error: " << strerror(errno) << endl;
		return;
	}

	write(sockfd, info, strlen(info));
	
	read(sockfd, count, sizeof(count));
	
	close(sockfd);
}

// getQueue(Server, char *, char *)
// turns the proxie server into a client so it can communicate with the queue
// specified by the client and retrieve a message from the queue so it can
// relay it to the client
void Server::getQueue(Client c, char *info, char *message)
{
	int sockfd = 0;
	
	struct sockaddr_in s = c.getAddr();

	// create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket error" << endl;
		return;
	}
	
	// connect
	if (connect(sockfd, (struct sockaddr *)&s, sizeof(s)) < 0)
	{
		cout << "connect error: " << strerror(errno) << endl;
		return;
	}

	write(sockfd, info, strlen(info));
	
	char buff[4096];
	memset(buff, 0, sizeof(buff));
	read(sockfd, buff, sizeof(buff));
	strcpy(message, buff);
	
	close(sockfd);
}


#endif /* SERVER_H */
