#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <cassert>
#include <sstream>
#include <thread>
#include <mutex>

class Client {
	struct sockaddr_in _serv_addr;
	int _sockfd;
	std::vector<std::string> _subscriptions;
public:
	Client(std::string ipAddr, std::string port);
	struct sockaddr_in getAddr();
	int getSockfd();
	std::vector<std::string> getSubscriptions();
	bool isSubscribed(const std::string &subscription);
	void get(std::string input);
	void put(std::string input);
	void list(std::string input);
	void subscribe(std::string input);
	void publish(std::string input);
};

using namespace std;

Client::Client(string ipAddr, string port)
{
	_sockfd = 0;
	memset(&_serv_addr, 0, sizeof(_serv_addr));
	
	// open socket
	if((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket error" << endl;
	}

	cout << "socket openned" << endl;

	// set address
	_serv_addr.sin_family = AF_INET;
	_serv_addr.sin_port = atoi(port.c_str()); 

	if(inet_pton(AF_INET, ipAddr.c_str(), &_serv_addr.sin_addr) <= 0)
	{
		cout << "inet_pton error" << endl;
	}

	cout << "inet_pton complete" << endl;
	
	// connect
	if(connect(_sockfd, (struct sockaddr *)&_serv_addr, sizeof(_serv_addr)) < 0)
	{
		cout << "connect error" << endl;
	}

	cout << "client created successfully" << endl;
}

struct sockaddr_in Client::getAddr()
{
	return _serv_addr;
}

int Client::getSockfd()
{
	return _sockfd;
}

vector<string> Client::getSubscriptions()
{
	return _subscriptions;
}

bool Client::isSubscribed(const string &subscription)
{
	for (string s : _subscriptions)
	{
		if (s == subscription)
			return true;
	}
	
	return false;
}

void Client::get(string input)
{
	cout << "inside get" << endl;
	char buff[4096];
	memset(buff, 0, sizeof(buff));
	// sends the server the command
	write(_sockfd, input.c_str(), input.length() + 1);
	// reads the message from the requested queue
	read(_sockfd, buff, sizeof(buff));
	// outputs the message
	cout << buff << endl;
}

void Client::put(string input)
{
	cout << "inside put" << endl;
	const char *message = strstr(input.c_str(), "\"");
	// write the message to the server and the server will
	// distribute it to the named queues it received from this client
	write(_sockfd, input.c_str(), input.length() + 1);
	cout << message << " sent" << endl;
}

void Client::list(string input)
{
	cout << "inside list" << endl;
	char buff[4096];
	memset(buff, 0, sizeof(buff));
	// sends the command to the server
	write(_sockfd, input.c_str(), input.length() + 1);
	// read the number of messages of the named queue from the server
	cout << "written" << endl;
	read(_sockfd, buff, sizeof(buff));
	cout << "read" << endl;
	// outputs the message count
	cout << buff << " messages" << endl;
}

void Client::subscribe(string input)
{
	cout << "subscribe" << endl;
	istringstream iss(input);
	string command, exchangeName, subscription;
	iss >> command >> exchangeName >> subscription;
	if (!isSubscribed(subscription))
	{
		_subscriptions.push_back(subscription);
		write(_sockfd, input.c_str(), input.length() + 1);
		cout << "subscribed to " << subscription << endl;
	}
	else
		cout << "already subscribed to " << subscription << endl;
}

void Client::publish(string input)
{
	cout << "publish" << endl;
	const char *message = strstr(input.c_str(), "\"");
	istringstream iss(input);
	string command, exchangeName, subscription;
	iss >> command >> exchangeName >> subscription;
	write(_sockfd, input.c_str(), input.length() + 1);
	cout << message << " written to " << subscription << endl;
}

#endif /* CLIENT_H */
