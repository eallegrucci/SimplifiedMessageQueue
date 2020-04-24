#ifndef EXCHANGE_H
#define EXCHANGE_H
#include "client.h"
#include <vector>
#include <string>
#include <map>

class Exchange {
	std::vector<std::string> _subscriptions;
	std::map<std::string, std::vector<Client>> _clients;
public:
	Exchange();
	std::vector<std::string> getSubscriptions();
	bool subscriptionExists(std::string name);
	void handlePublish(char *recv, int connfd);
	void handleSubscribe(char *recv, const std::string &ip, const std::string &port);
};

using namespace std;

Exchange::Exchange()
{
}

vector<string> Exchange::getSubscriptions()
{
	return _subscriptions;
}

bool Exchange::subscriptionExists(string name)
{
	for (string s : _subscriptions)
		if (s == name)
			return true;
	return false;
}

void Exchange::handlePublish(char *recv, int connfd)
{
	cout << "handlePublish" << endl;
	// extract the message from the received input
	const char *message = strstr(recv, "\"");
	string command, myname, subname;
	// vector of names to send the message to
	istringstream iss(recv);
	iss >> command >> myname >> subname;
	if (subscriptionExists(subname))
	{
		for (Client c : _clients.at(subname))
		{
			int sockfd = c.getSockfd();
			write(sockfd, message, strlen(message));
			cout << "written" << endl;
		}
	}
	else
	{
		cout << subname << " does not exist" << endl;
		string m = subname + " does not exist";
		write(connfd, m.c_str(), m.length() + 1);
	}
}

void Exchange::handleSubscribe(char *recv, const string &ip, const string &port)
{
	istringstream iss(recv);
	string command, myname, subname;
	iss >> command >> myname >> subname;

	Client c = Client(ip, port);

	if (!subscriptionExists(subname))
	{
		cout << "new subscription " << subname << endl;
		vector<Client> cl;
		cl.push_back(c);
		_clients.insert(pair<string, vector<Client>>(subname, cl));
	}
	else
	{
		cout << "new client add to subscription " << subname << endl;
		_clients.at(subname).push_back(c);
	}
}

#endif /* EXCHANGE_H */
