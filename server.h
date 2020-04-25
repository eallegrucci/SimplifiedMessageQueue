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
#include <ifaddrs.h>
#include <sys/types.h>
#include <cassert>
#include <sstream>
#include <mutex>
#include <thread>
#include "MessageQueue.h"
#include "Exchange.h"

class Server {
	std::string _name;
	std::string _hostname;
	std::string _myIP;
	std::string _myPort;
	MessageQueue _queue = MessageQueue();
	Exchange _exchange = Exchange();
	std::map<std::string, Client> _linkedQueues;
	bool _isExchange;
	struct sockaddr_in _serv_addr;
	int _listenfd;
public:
	Server(char *&name, char *&type);
	std::string getName();
	std::string getHostname();
	int getListenfd();
	void addLinkedQueue(std::string name);
	void putQueue(Client c, char *input);
	void getQueue(Client c, char *input, char *message);
	void listQueue(Client c, char *input, char *count);
	void handleGet(char *recv, int connfd);
	void handlePut(char *recv, int connfd);
	void handleList(char *recv, int connfd);
	void handleBind(char *recv, int connfd);
	void handleSubscriber(char *recv, int connfd);
	void handlePublisher(char *recv, int connfd);
};

using namespace std;

Server::Server(char *&name, char *&type)
{
	_name = name;
	if (!strcmp(type, "queue"))
		_isExchange = false;
	else
		_isExchange = true;
	struct hostent *host;
	char hostname[256];
	char *IPbuff;
	gethostname(hostname, 256);

	_hostname = hostname;

	host = gethostbyname(hostname);
	IPbuff = inet_ntoa(*((struct in_addr *)host->h_addr_list[0]));
	_myIP = IPbuff;
	
	_serv_addr.sin_family = AF_INET;
	_serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	
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

	_myPort = to_string(_serv_addr.sin_port);

	cout << "port number: " << _myPort << endl;
}

string Server::getName()
{
	return _name;
}

string Server::getHostname()
{
	return _hostname;
}

int Server::getListenfd()
{
	return _listenfd;
}

void Server::addLinkedQueue(string input)
{
	// add the new Server to the linkedQueues vector
	cout << "addLinkedQueue entered" << endl;
	string command, name, ipAddr, portNum;
	char buff[10];
	// add a new Server item to the linkedQueues list
	istringstream iss(input);
	cout << "iss stream" << endl;
	iss >> command >> name >> ipAddr >> portNum;
	cout << command << endl;
	cout << name << endl;
	cout << ipAddr << endl;
	cout << portNum << endl;
	cout << "server input parsed" << endl;
	// create a new client linked to the new queue
	Client client = Client(ipAddr, portNum);
	cout << "client add in addLinkedQueue" << endl;
	_linkedQueues.insert(pair<string, Client>(name, client));
	cout << "client added to linked queue" << endl;
	string new_input = command + " " + _name + " " + _myIP + " " + _myPort;
	cout << new_input << endl;
	write(client.getSockfd(), new_input.c_str(), new_input.length() + 1);
	cout << "wrote bind to other queue" << endl;
	read(client.getSockfd(), buff, sizeof(buff));
	cout << "read bound from other queue" << endl;
	if (strstr(buff, "bound"))
		cout << name << " linked to " << _name << " successfully" << endl;
	else
		cout << "bind to " << name << " unsuccessful" << endl;
}

// putQueue(Server, char *)
// turns the proxie Server into a client so it can communicate with
// other servers and send the "message" to be placed into the queues
// specified by the client
void Server::putQueue(Client c, char *info)
{
	int sockfd = c.getSockfd();
	cout << "putQueue" << endl;
		
	write(sockfd, info, strlen(info));
	cout << "written" << endl;
}

// listQueue(Server, char *, char *)
// turns this proxie server into a client so it can communicate with other
// queues and requesr the messages count of the specified queue from the client
void Server::listQueue(Client c, char *info, char *count)
{
	int sockfd = c.getSockfd();
	cout << "listQueue" << endl;
	write(sockfd, info, strlen(info));
	cout << info << " written" << endl;
	read(sockfd, count, sizeof(count));
	cout << count << " read" << endl;
}

// getQueue(Server, char *, char *)
// turns the proxie server into a client so it can communicate with the queue
// specified by the client and retrieve a message from the queue so it can
// relay it to the client
void Server::getQueue(Client c, char *info, char *message)
{
	int sockfd = c.getSockfd();
	cout << "getQueue" << endl;

	write(sockfd, info, strlen(info));
	cout << "written" << endl;
	char buff[4096];
	memset(buff, 0, sizeof(buff));
	read(sockfd, buff, sizeof(buff));
	strcpy(message, buff);
	cout << "read" << endl;
}

void Server::handleGet(char *recv, int connfd)
{
	cout << "handleGet" << endl;
	string str, command, name;
	istringstream iss(recv);
	iss >> command >> name;
	// if this queue's name matches then send the
	// first message in your queue if your queue is not empty
	if (!_isExchange && (_name == name))
	{
		cout << "get my queue" << endl;
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
		cout << "check other queue" << endl;
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
	cout << "handlePut" << endl;
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
	cout << "going to loop through names" << endl;
	for (string n : names)
	{
		cout << "Queue name: " << n << endl;
		// if the name received is the name of this queue
		if(!_isExchange && _name == n)
		{
			cout << "add message to me" << endl;
			// add the message to the queue
			_queue.addMessage(message);
			cout << message << " added to " << _name << endl;
		}
		else
		{
			// otherwise loop through the likedQueues vector
			// and if linkedQueues has an object with the same name
			// send that message to that queue
			string mes = command + " " + n + " " + message;
			cout << mes << endl;
			char *c = new char[mes.length() + 1];
			strcpy(c, mes.c_str());
			if (_linkedQueues.find(n) != _linkedQueues.end())
			{
				cout << "Sending to queue " << n << endl;
				putQueue(_linkedQueues.at(n), c);
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
	cout << "handleList" << endl;
	string command, name;
	istringstream iss(recv);
	iss >> command >> name;
	// if this queue's name is the same,
	// send the client this queue's number of message
	if (!_isExchange && _name == name)
	{
		cout << "queue is me" << endl;
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
		cout << "check other queues" << endl;
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

void Server::handleBind(char *recv, int connfd)
{
	cout << "inside handleBind" << endl;
	istringstream iss(recv);
	string command, name, ipAddr, portNum;
	iss >> command >> name >> ipAddr >> portNum;
	cout << command << endl;
	cout << name << endl;
	cout << ipAddr << endl;
	cout << portNum << endl;
	cout << "string parsed" << endl;
	// create a new client linked to the new queue
	Client client = Client(ipAddr, portNum);
	cout << "client created" << endl;
	_linkedQueues.insert(pair<string, Client>(name, client));
	cout << "client added to linked queue" << endl;
	write(connfd, "bound", 10);
	cout << "wrote bound to other queue" << endl;
	cout << name << " linked to " << _name << " successfully" << endl;	
}

void Server::handleSubscriber(char *recv, int connfd)
{
	if (_isExchange)
	{
		_exchange.handleSubscribe(recv, connfd);
	}
	else
	{
		cout << "I am not an exchange server so I can not subscribe" << endl;
	}
}

void Server::handlePublisher(char *recv, int connfd)
{
	if (_isExchange)
	{
		_exchange.handlePublish(recv, connfd);
	}
	else
	{
		cout << "I am not an exchange server so I can not publish" << endl;
	}
}

#endif /* SERVER_H */
