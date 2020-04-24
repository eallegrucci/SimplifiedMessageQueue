#include "client.h"

using namespace std;

mutex m;

void inputCommands(Client *c)
{
	string input;
	char buff[4096];
	memset(buff, 0, sizeof(buff));
	
	//Reading user input once connected
	while(1) {
		cout << "client> ";
		getline(cin, input);
		
		// handles get command
		if (strstr(input.c_str(), "get"))
		{
			m.lock();
			c->get(input);
			m.unlock();
		}
		// handles the put command
		else if (strstr(input.c_str(), "put"))
		{
			m.lock();
			c->put(input);
			m.unlock();
		}
		// handles list command
		else if (strstr(input.c_str(), "list"))
		{
			m.lock();
			c->list(input);
			m.unlock();
		}
		// handles subscribe command
		else if (strstr(input.c_str(), "subscribe"))
		{
			m.lock();
			c->subscribe(input);
			m.unlock();
		}
		else if (strstr(input.c_str(), "publish"))
		{
			m.lock();
			c->publish(input);
			m.unlock();
		}
		// handles inexceptable commands
		else
		{
			cout << "That command does not exist" << endl;
		}
		// clear buff
		memset(buff, 0, sizeof(buff));
	}
}

void listeningToExchange(Client *c)
{
	char buff[4096];
	memset(buff, 0, sizeof(buff));

	while(1)
	{
		m.lock();
		read(c->getSockfd(), buff, sizeof(buff));
		m.unlock();
		cout << buff << endl;
		memset(buff, 0, sizeof(buff));
	}
}


int main(int argc, char *argv[])
{
	// checking the arguments
	if(argc != 3)
	{
		cout << "Usage: " << argv[0] << " <ip of server> <Port number of server>" << endl;
		return 1;
	}

	cout << argv[1] << " " << argv[2] << endl;

	string ip(argv[1]), port(argv[2]);
	Client client = Client(ip, port);
	
	cout << "Connected to IP: " << argv[1] << " with Port: " << argv[2] << endl;
	
	//this thread is waiting for the user to input commands
	thread t1(inputCommands, &client);
	// this thread is wait to read from the exchange server is is connected to
	thread t2(listeningToExchange, &client);

	t1.join();
	t2.join();
	
	return 0;
}
