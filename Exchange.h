#ifndef EXCHANGE_H
#define EXCHANGE_H
#include "client.h"
#include <vector>
#include <string>
#include <map>

class Exchange {
	std::vector<std::string> _subscriptions;
	std::map<std::string, std::vector<int>> _clients;
public:
	Exchange();
	std::vector<std::string> getSubscriptions();
	bool subscriptionExists(std::string name);
	void handlePublish(char *recv, int connfd);
	void handleSubscribe(char *recv, int connfd);
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
	cout << "inside subscriptionExists" << endl;
	for (string s : _subscriptions)
	{
		cout << s << endl;
		if (s == name)
			return true;
	}
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
	cout << command << endl;
	cout << myname << endl;
	cout << subname << endl;
	cout << message << endl;
	if (subscriptionExists(subname))
	{
		for (int c : _clients.at(subname))
		{
			cout << connfd << endl;
			cout << c << endl;
			write(c, message, strlen(message));
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

void Exchange::handleSubscribe(char *recv, int connfd)
{
	istringstream iss(recv);
	string command, myname, subname;
	iss >> command >> myname >> subname;
	cout << command << endl;
	cout << myname << endl;
	cout << subname << endl;
	cout << connfd << endl;

	if (!subscriptionExists(subname))
	{
		cout << "new subscription " << subname << endl;
		vector<int> client;
		client.push_back(connfd);
		cout << client.at(0);
		_subscriptions.push_back(subname);
		_clients.insert(pair<string, vector<int>>(subname, client));
	}
	else
	{
		cout << "new client add to subscription " << subname << endl;
		_clients.at(subname).push_back(connfd);
	}
}

#endif /* EXCHANGE_H */
