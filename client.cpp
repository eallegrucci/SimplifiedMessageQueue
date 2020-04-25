#include "client.h"

using namespace std;

void inputCommands(Client *c)
{
	cout << "thread input commands" << endl;
	string input;
	
	//Reading user input once connected
	while(1) {
		cout << "client> ";
		getline(cin, input);
		
		// handles get command
		if (strstr(input.c_str(), "get"))
		{
			cout << "get" << endl;
			c->get(input);
		}
		// handles the put command
		else if (strstr(input.c_str(), "put"))
		{
			cout << "put" << endl;
			c->put(input);
		}
		// handles list command
		else if (strstr(input.c_str(), "list"))
		{
			cout << "list" << endl;
			c->list(input);
		}
		// handles subscribe command
		else if (strstr(input.c_str(), "subscribe"))
		{
			cout << "subscribe" << endl;
			c->subscribe(input);
		}
		else if (strstr(input.c_str(), "publish"))
		{
			cout << "publish" << endl;
			c->publish(input);
		}
		// handles inexceptable commands
		else
		{
			cout << "That command does not exist" << endl;
		}
	}
}

void listeningToExchange(Client *c)
{
	cout << "thread listening to exchange" << endl;
	char buff[4096];
	memset(buff, 0, sizeof(buff));

	while(1)
	{
		read(c->getSockfd(), buff, sizeof(buff));
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
	Client client1 = Client(ip, port);
	//Client client2 = Client(ip, port);
	
	cout << "Connected to IP: " << argv[1] << " with Port: " << argv[2] << endl;
	
	//this thread is waiting for the user to input commands
	thread t1(inputCommands, &client1);
	// this thread is wait to read from the exchange server is is connected to
	thread t2(listeningToExchange, &client1);

	t1.join();
	t2.join();
	
	return 0;
}
