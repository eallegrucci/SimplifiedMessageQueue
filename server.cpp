#include "server.h"
#include "MessageQueue.h"

extern int errno;

using namespace std;

int main(int argc, char *argv[])
{
	int n;
	int listenfd = 0, connfd = 0;

	// check arguments
	if (argc != 4 || strcmp(argv[1], "create") || strcmp(argv[2], "queue"))
	{
		cout << "Usage: " << argv[0] << " create queue <queue name>" << endl;
		cout << argv[1] << argv[2] << endl;
		return 1;
	}
	
	cout << "checking arguments" <<endl;
	Server server = Server(argv[3], argv[2]);

	cout << "server created" << endl;
	string input;
	char recv[4096], message[4096];

	memset(recv, 0, sizeof(recv));
	memset(message, 0, sizeof(message));
	
	// while bind command
	while(1)
	{
		cout << "server> ";
		getline(cin, input);
		// while the command is bind
		if (strstr(input.c_str(), "bind"))
		{
			server.addLinkedQueue(input);
		}
		else
		{
			cout << "Server done binding" << endl;
			break;
		}
	}
	
	// accept and connect to client
	while(1)
	{
		//connect to client
		if((connfd = accept(server.getListenfd(), (struct sockaddr*)NULL, NULL)) < 0)
		{
			cout << "accept error" << endl;
			return 1;
		}

		// read from client
		while(read(connfd, recv, sizeof(recv)) > 0)
		{
			// handles put command
			// places the received message in each queue specified by client
			if (strstr(recv, "put"))
			{
				cout << "Received put command from client" << endl;	
				server.handlePut(recv, connfd);
			}
			// handles the list command
			// list command only takes 1 queue name as an input
			else if (strstr(recv, "list"))
			{
				cout << "List command received from client" << endl;
				server.handleList(recv, connfd);
			}
			// handles get command
			else if (strstr(recv, "get"))
			{
				cout << "Get command received from client" << endl;
				server.handleGet(recv, connfd);
			}
			else
				cout << "Command not recognized" << endl;
			//clear command buffer
			memset(recv, 0, sizeof(recv));
		}
		close(connfd);
		printf("Connection to socket closed.\n");
     }
}
