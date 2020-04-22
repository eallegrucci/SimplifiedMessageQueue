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
	MessageQueue _queue = MessageQueue();
	//Exchange *_proxieQueue;
	std::map<std::string, Client> _linkedQueues;
	bool _isExchange;
	struct sockaddr_in _serv_addr;
	int _listenfd;
public:
	Server(char *&name, char *&type);
	std::string getName();
	int getListenfd();
	void addLinkedQueue(std::string name);
	void putQueue(Client c, char *input);
	void getQueue(Client c, char *input, char *message);
	void listQueue(Client c, char *input, char *count);
	void handleGet(char *recv, int connfd);
	void handlePut(char *recv, int connfd);
	void handleList(char *recv, int connfd);
};

using namespace std;

Server::Server(char *&name, char *&type)
{
	_name = name;
	_isExchange = false;
	
	_serv_addr.sin_family = AF_INET;
	cout << _serv_addr.sin_family << endl;
	_serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	cout << _serv_addr.sin_addr.s_addr << endl;
	//change port number check if its occupied
	_serv_addr.sin_port = htons(5000); 

	cout << "_serv_addr set" << endl;
	// create socket
	if((_listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket error" << endl;
	}
	cout << "socket created" << endl;	
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

	cout << "bind done" << endl;
	// listen for the connection
	if (listen(_listenfd, 10) < 0)
	{
		cout << "listen error" << endl;
	}

	cout << "port number: " << _serv_addr.sin_port << endl;
}

string Server::getName()
{
	return _name;
}

int Server::getListenfd()
{
	return _listenfd;
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
	char *ipa;
	ip >> ipa;
	char *p;
	pNum >> p;
	// create a new client linked to the new queue
	Client client = Client(ipa, p);
	//Client *client = Client(ipA, p);
	_linkedQueues.insert(pair<string, Client>(name, client));
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
	if (connect(sockfd, (struct sockaddr *)&s, sizeof(s) < 0))
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

void Server::handleGet(char *recv, int connfd)
{
	string str, command, name;
	istringstream iss(recv);
	iss >> command >> name;
	// if this queue's name matches then send the
	// first message in your queue if your queue is not empty
	if (!_isExchange && (_name == name))
	{
		if (_queue.containsMessages())
		{
			str = _queue.getMessage();
			char *message = new char[str.length() + 1];
			strcpy(message, str.c_str());
			write(connfd, message, strlen(message));
			cout << message << " sent to client" << endl;
		}
		// if your queue is empty send an error message to the client
		else
		{
			cout << "No message in queue to retrieve" << endl;
			write(connfd, "Queue has no messages to retrieve", 50);
		}
	}
	// if this queue is not the same queue
	// check all the queue naem in the LinkedQueues vector
	else
	{
		bool exists = false;
		char message[4096];
		memset(message, 0, sizeof(message));
		
		if (_linkedQueues.find(name) != _linkedQueues.end())
		{
			cout << "Get command forwarded to queue " << name << endl;
			// send the Server a request for the first message in its queue
			getQueue(_linkedQueues.at(name), recv, message);
			// send the message to the client
			write(connfd, message, sizeof(message));
		}
		// if no queue with that name exists
		// send an error message to the client
		else
		{
			cout << "No queue has the name " << name << endl;
			write(connfd, "No queue of that name exists", 50);
		}
	}
}

void Server::handlePut(char *recv, int connfd)
{
	// extract the message from the received input
	const char *message = strstr(recv, "\"");
	string command, name;
	// vector of names to send the message to
	vector<string> names;
	istringstream iss(recv);
	iss >> command;
	// while the input stream still has strings
	// stream it into a name variable and add it to the list of names
	while (iss >> name)
	{
		if (name[0] == '"')
			break;
		names.push_back(name);
		cout << name << endl;
	}
	for (string n : names)
	{
		cout << "Queue name: " << n << endl;
		// if the name received is the name of this queue
		if(!_isExchange && _name == n)
		{
			// add the message to the queue
			_queue.addMessage(message);
			cout << message << " added to " << _name << endl;
		}
		else
		{
			// otherwise loop through the likedQueues vector
			// and if linkedQueues has an object with the same name
			// send that message to that queue
			if (_linkedQueues.find(n) != _linkedQueues.end())
			{
				cout << "Sending to queue " << n << endl;
				putQueue(_linkedQueues.at(n), recv);
			}
			else
			{
				cout << "No queue has the name " << n << endl;
			}
		}
	}
}

void Server::handleList(char *recv, int connfd)
{
	string command, name;
	istringstream iss(recv);
	iss >> command >> name;
	// if this queue's name is the same,
	// send the client this queue's number of message
	if (!_isExchange && _name == name)
	{
		int c = _queue.getMessageCount();
		string str;
		stringstream out;
		out << c;
		str = out.str();
		char *count = new char[str.length() + 1];
		strcpy(count, str.c_str());
		write(connfd, count, sizeof(count));
	}
	// otherwise loop through the vector linkedQueues
	else
	{
		bool exists = false;
		char count[32];
		if (_linkedQueues.find(name) != _linkedQueues.end())
		{
			cout << "List command forwarded to queue " << name << endl;
			listQueue(_linkedQueues.at(name), recv, count);
			write(connfd, count, sizeof(count));
		}
		// if the queue does not exist send an error message to
		// the client
		else
		{
			cout << "No queue has the name " << name << endl;
			write(connfd, "No queue of that name exists", 50);
		}
	}
}

#endif /* SERVER_H */
