#include "server.h"
#include "MessageQueue.h"

extern int errno;

using namespace std;

// Server struct contyains:
// queue's name, its sockaddr_in, and its socket
typedef struct server {
	char name[100];
	struct sockaddr_in s;
} Server;

// creatServer(string)
// creates and returns a new Server item with the information
// from input
Server createServer(string input)
{
	string command, name, ipAddr, portNum;
	// add a new Server item to the linkedQueues list
	istringstream iss(input);
	iss >> command >> name >> ipAddr >> portNum;
	stringstream pNum(portNum);
	stringstream ip(ipAddr);
	// new Server item
	Server bind;
	// popoulate the new Server's name
	strcpy(bind.name, name.c_str());
	struct sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET;
	int x = 0;
	pNum >> x;
	bindAddr.sin_port = x;
	char ipa[14];
	ip >> ipa;
	if(inet_pton(AF_INET, ipa, &bindAddr.sin_addr) <= 0)
	{
		cout << "inet_pton error" << endl;
		exit(-1);
	}
	// populate the new Server's struct sockaddr_in
	bind.s = bindAddr;
	return bind;
}

// putQueue(Server, char *)
// turns the proxie Server into a client so it can communicate with
// other servers and send the "message" to be placed into the queues
// specified by the client
void putQueue(Server boundQ, char *info)
{
	int sockfd = 0;
	
	// create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket error" << endl;
		return;
	}
	
	// connect
	if (connect(sockfd, (struct sockaddr *)&boundQ.s, sizeof(boundQ.s)) < 0)
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
void listQueue(Server boundQ, char *info, char *count)
{
	int sockfd = 0;
	
	// create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket error" << endl;
		return;
	}
	
	// connect
	if (connect(sockfd, (struct sockaddr *)&boundQ.s, sizeof(boundQ.s)) < 0)
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
void getQueue(Server boundQ, char *info, char *message)
{
	int sockfd = 0;
	
	// create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket error" << endl;
		return;
	}
	
	// connect
	if (connect(sockfd, (struct sockaddr *)&boundQ.s, sizeof(boundQ.s)) < 0)
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

int main(int argc, char *argv[])
{
	int n;
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr;

	// vetor of bound queues
	vector<Server> linkedQueues;

	// check arguments
	if (argc != 4 || strcmp(argv[1], "create") || strcmp(argv[2], "queue"))
	{
		cout << "Usage: " << argv[0] << " create queue <queue name>" << endl;
		cout << argv[1] << argv[2] << endl;
		return 1;
	}
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	//change port number check if its occupied
	serv_addr.sin_port = htons(5000); 

	// queue created
	MessageQueue q = MessageQueue(argv[3]);

	// create socket
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket error" << endl;
		return 1;
	}

	int i = 0;
	// while the ind is unsuccessful change the port number
	// exit the loop once the bind is successful or after 1000 unsuccessful tries
	while(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		if (i > 1000)
		{
			cout << "bind error" << endl;
			return 1;
		}
		serv_addr.sin_port++;
		i++;
	}

	// listen for the connection
	if (listen(listenfd, 10) < 0)
	{
		cout << "listen error" << endl;
		return 1;
	}

	cout << "port number: " << serv_addr.sin_port << endl;
	
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
			// add a new Server item to the linkedQueues list
			Server bind = createServer(input);
			// add the new Server to the linkedQueues vector
			linkedQueues.push_back(bind);
			cout << bind.name << " linked to " << q.getName() << " successfully" << endl;
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
		if((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) < 0)
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
				cout << recv << endl;
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
					if(strcmp(q.getName().c_str(), n.c_str()) == 0)
					{
						// add the message to the queue
						q.addMessage(message);
						cout << message << " added to " << q.getName() << endl;
					}
					else
					{
						// otherwise loop through the likedQueues vector
						// and if linkedQueues has an object with the same name
						// send that message to that queue
						bool exists = false;
						for(Server s : linkedQueues)
						{
							if(strcmp(s.name, n.c_str()) == 0)
							{
								cout << "Sending to queue " << s.name << endl;
								putQueue(s, recv);
								exists = true;
								break;
							}
						}
						if (!exists)
						{
							cout << "No queue has the name " << n << endl;
						}
					}
				}
			}
			// handles the list command
			// list command only takes 1 queue name as an input
			else if (strstr(recv, "list"))
			{
				cout << "List command received from client" << endl;
				string command, name;
				istringstream iss(recv);
				iss >> command >> name;
				// if this queue's name is the same,
				// send the client this queue's number of message
				if (strcmp(q.getName().c_str(), name.c_str()) == 0)
				{
					int c = q.getMessageCount();
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
					for(Server s : linkedQueues)
					{
						// if a Server in linkedQueues has the same name
						if(strcmp(s.name, name.c_str()) == 0)
						{
							cout << "List command forwarded to queue " << s.name << endl;
							listQueue(s, recv, count);
							write(connfd, count, sizeof(count));
							exists = true;
							// and exit the loop
							break;
						}
					}
					// if the queue does not exist send an error message to
					// the client
					if (!exists)
					{
						cout << "No queue has the name " << name << endl;
						write(connfd, "No queue of that name exists", 50);
					}
				}
			}
			// handles get command
			else if (strstr(recv, "get"))
			{
				cout << "Get command received from client" << endl;
				string str, command, name;
				istringstream iss(recv);
				iss >> command >> name;
				// if this queue's name matches then send the
				// first message in your queue if your queue is not empty
				if (strcmp(q.getName().c_str(), name.c_str()) == 0)
				{
					if (q.containsMessages())
					{
						str = q.getMessage();
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
					for(Server s : linkedQueues)
					{
						// if a Server in linkedQueues name matches the read name
						if(strcmp(s.name, name.c_str()) == 0)
						{
							cout << "Get command forwarded to queue " << s.name << endl;
							// send the Server a request for the first message in its queue
							getQueue(s, recv, message);
							// send the message to the client
							write(connfd, message, sizeof(message));
							exists = true;
							break;
						}
					}
					// if no queue with that name exists
					// send an error message to the client
					if (!exists)
					{
						cout << "No queue has the name " << name << endl;
						write(connfd, "No queue of that name exists", 50);
					}
				}
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
