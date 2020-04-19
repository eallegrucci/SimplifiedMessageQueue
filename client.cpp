#include "client.h"
#include "MessageQueue.h"

using namespace std;

int main(int, char *[]);

int main(int argc, char *argv[])
{
	int sockfd = 0, n;
	string message = "";

	// checking the arguments
	if(argc != 3)
	{
		cout << "Usage: " << argv[0] << " <ip of server> <Port number of server>" << endl;
		return 1;
	}

	cout << argv[1] << " " << argv[2] << endl;

	Client client = Client(argv[1], argv[2]);
	
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
			client.get(input);
		}
		// handles the put command
		else if (strstr(input.c_str(), "put"))
		{
			client.put(input);
		}
		// handles list command
		else if (strstr(input.c_str(), "list"))
		{
			client.list(input);
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
