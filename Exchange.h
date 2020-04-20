#ifndef EXCHANGE_H
#define EXCHANGE_H
#include "client.h"
#include "MessageQueue.h"
#include <vector>
#include <string>


class Exchange {
	// vector of clients
	std::string _name;
	std::vector<Client> _clients;
	std::vector<MessageQueue> _queues;
public:
	Exchange(std::string name);
	Client getClient();
	MessageQueue getQueue();
	void published();
	void subscriptions();
};

using namespace std;

Exchange::Exchange(string name)
{
	_name = name;
}

#endif /* EXCHANGE_H */
