#include "server.h"

extern int errno;

using namespace std;

mutex m;

void backgroundCommands(Server *s) {
	cout << "inside thread" << endl;
	string input;
	// while bind command
	while(1)
	{
		cout << "server> ";
		getline(cin, input);
		// while the command is bind
		if (strstr(input.c_str(), "bind"))
		{
			m.lock();
			s->addLinkedQueue(input);
			m.unlock();
		}
		else
		{
			cout << "Command not recognized" << endl;
		}
		//sleep(10);
	}
}

void listeningToClient(Server *s)
{
	int connfd = 0;	
	char recv[4096], message[4096];
	memset(recv, 0, sizeof(recv));
	memset(message, 0, sizeof(message));
	// accept and connect to client
	while(1)
	{
		//connect to client
		m.lock();
		if((connfd = accept(s->getListenfd(), (struct sockaddr*)NULL, NULL)) < 0)
		{
			cout << "accept error" << endl;
			perror("accept");
		}
		m.unlock();

		// read from client
		while(read(connfd, recv, sizeof(recv)) > 0)
		{
			// handles put command
			// places the received message in each queue specified by client
			if (strstr(recv, "put"))
			{
				cout << "Received put command from client" << endl;	
				m.lock();
				s->handlePut(recv, connfd);
				m.unlock();
			}
			// handles the list command
			// list command only takes 1 queue name as an input
			else if (strstr(recv, "list"))
			{
				cout << "List command received from client" << endl;
				m.lock();
				s->handleList(recv, connfd);
				m.unlock();
			}
			// handles get command
			else if (strstr(recv, "get"))
			{
				cout << "Get command received from client" << endl;
				m.lock();
				s->handleGet(recv, connfd);
				m.unlock();
			}
			else if (strstr(recv, "bind"))
			{
				cout << "bind command received" << endl;
				m.lock();
				s->handleBind(recv, connfd);
				m.unlock();
			}
			else if (strstr(recv, "subscribe"))
			{
				cout << "subscribe command received" << endl;
				m.lock();
				s->handleSubscriber(recv, connfd);
				m.unlock();
			}
			else if (strstr(recv, "publish"))
			{
				cout << "publish command received" << endl;
				m.lock();
				s->handlePublisher(recv, connfd);
				m.unlock();
			}
			else
				cout << "Command not recognized" << endl;
			//clear command buffer
			memset(recv, 0, sizeof(recv));
		}
		close(connfd);
		cout << "Connection to socket closed." << endl;
     }
}

int main(int argc, char *argv[])
{
	int n;

	// check arguments
	if (argc != 4 || strcmp(argv[1], "create") || (strcmp(argv[2], "queue") && strcmp(argv[2], "exchange")))
	{
		cout << "Usage: " << argv[0] << " create <server type> <server name>" << endl;
		cout << argv[1] << argv[2] << endl;
		return 1;
	}

	cout << "checking arguments" <<endl;
	Server server = Server(argv[3], argv[2]);

	cout << "server created" << endl;

	thread t1(backgroundCommands, &server);
	thread t2(listeningToClient, &server);

	t1.join();
	t2.join();
}
