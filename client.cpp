#include "client.h"
#include "MessageQueue.h"

using namespace std;

int main(int, char *[]);

int main(int argc, char *argv[])
{
	int sockfd = 0, n;
	string message = "";
	struct sockaddr_in serv_addr;

	// checking the arguments
	if(argc != 3)
	{
		cout << "Usage: " << argv[0] << " <ip of server> <Port number of server>" << endl;
		return 1;
	}

	cout << argv[1] << " " << argv[2] << endl;

	memset(&serv_addr, 0, sizeof(serv_addr));
	
	// open socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket error\n");
		return 1;
	}

	cout << "socket openned" << endl;

	// set address
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = atoi(argv[2]); 

	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
	{
		printf("inet_pton error\n");
		return 1;
	}

	cout << "inet_pton complete" << endl;
	
	// connect
	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("connect error\n");
		return 1;
	}

	cout << "Connected to IP: " << argv[1] << " with Port: " << argv[2] << endl;

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
			// sends the server the command
			write(sockfd, input.c_str(), input.length() + 1);
			// reads the message from the requested queue
			read(sockfd, buff, sizeof(buff));
			// outputs the message
			cout << buff << endl;
		}
		// handles the put command
		else if (strstr(input.c_str(), "put"))
		{
			const char *message = strstr(input.c_str(), "\"");
			// write the message to the server and the server will
			// distribute it to the named queues it received from this client
			write(sockfd, input.c_str(), input.length() + 1);
			cout << message << " sent" << endl;
		}
		// handles list command
		else if (strstr(input.c_str(), "list"))
		{
			// sends the command to the server
			write(sockfd, input.c_str(), input.length() + 1);
			// read the number of messages of the named queue from the server
			read(sockfd, buff, sizeof(buff));
			// outputs the message count
			cout << buff << " messages" << endl;
		}
		// handles inexceptable commands
		else
		{
			cout << "That command does not exist" << endl;
		}
		// clear buff
		memset(buff, 0, sizeof(buff));
	}

	close(sockfd);

	return 0;
}
