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
	}
}

void listeningToClient(Server *s)
{
	char recv[4096], message[4096];
	memset(recv, 0, sizeof(recv));
	memset(message, 0, sizeof(message));
	// accept and connect to client
	
	int opt = true;
	int connfd = 0;
	int max_sd = 0;
	int sd;
	fd_set readfds, activefds;
	//vector<int> clientSockets;
	int activity;
	m.lock();
	int masterSocket = s->getMasterSocket();
	m.unlock();

	struct sockaddr_in addr = s->getAddr();
	size_t size = sizeof(addr);
	FD_ZERO(&activefds);
	FD_SET(masterSocket, &activefds);   

	while (1)
	{
		readfds = activefds;
		cout << "Beginning of select..." << endl;
		activity = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
		cout << "Detected activity..." << endl;

		if ((activity < 0) && (errno != EINTR))
		{
			cout << "select error" << endl;
		}

		for (int sock = 0; sock < FD_SETSIZE; sock++)
		{

			if (FD_ISSET(sock,&readfds))
			{
				cout << "Processsing file descriptor " << sock << endl;
			     //if (FD_ISSET(masterSocket, &readfds))
			     if (sock == masterSocket)
			     {
			     	cout << "fd_isset" << endl;
			     	if((connfd = accept(masterSocket, (struct sockaddr *)&addr, (socklen_t*)&size)) < 0)
			     	{
			     		cout << "accept error" << endl;
			     		perror("accept");
			     	}
			     	FD_SET(connfd, &activefds);
			     	cout << connfd << " added to file descriptors list" << endl;
			     }
			     else
				{
					cout << "Waiting for read from " << sock << endl;
					int val = 0;

					if(val = read(sock, recv, sizeof(recv)) > 0)
					{
						cout << "fd_isset and read received" << endl;
						// handles put command
						// places the received message in each queue specified by client
						if (strstr(recv, "put"))
						{
							cout << "Received put command from client" << endl;	
							m.lock();
							s->handlePut(recv, sock);
							m.unlock();
						}
						// handles the list command
						// list command only takes 1 queue name as an input
						else if (strstr(recv, "list"))
						{
							cout << "List command received from client" << endl;
							m.lock();
							s->handleList(recv, sock);
							m.unlock();
						}
						// handles get command
						else if (strstr(recv, "get"))
						{
							cout << "Get command received from client" << endl;
							m.lock();
							s->handleGet(recv, sock);
							m.unlock();
						}
						else if (strstr(recv, "bind"))
						{
							cout << "bind command received" << endl;
							m.lock();
							s->handleBind(recv, sock);
							m.unlock();
						}
						else if (strstr(recv, "subscribe"))
						{
							cout << "subscribe command received" << endl;
							m.lock();
							s->handleSubscriber(recv, sock);
							m.unlock();
						}
						else if (strstr(recv, "publish"))
						{
							cout << "publish command received" << endl;
							m.lock();
							s->handlePublisher(recv, sock);
							m.unlock();
						}
						else
							cout << "Command not recognized" << endl;
						//clear command buffer
						memset(recv, 0, sizeof(recv));
					}
					if (val == 0)
					{
						cout << "nothing to read" << endl;
						close(sock);
						FD_CLR (sock, &activefds);
						cout << "Connection to socket closed." << endl;
					}
				}
			}
		}
	}
}

int main(int argc, char *argv[])
{
	int connfd;

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

	return 0;
}
