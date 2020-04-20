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

	Server server = Server(argv[3], argv[2]);

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
