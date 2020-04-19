#include "MessageQueue.h"

using namespace std;

// constructor
MessageQueue::MessageQueue(string name)
{
	_name = name;
	_messageCount = 0;
}

// getName()
// returns the name of the queue
string MessageQueue::getName() const
{
	return _name;
}

// getMessageCount()
// returns the number of messages in the queue
int MessageQueue::getMessageCount() const
{
	return _messageCount;
}

// containsMessages()
// returns true if yes, false otherwise
bool MessageQueue::containsMessages() const
{
	if (_messageCount > 0)
		return true;
	return false;
}


// getMessage()
// returns the first message in the queue and removes
// it from the queue
string MessageQueue::getMessage()
{
	if (_messageCount == 0)
	{
		cout << "no messages to get inside " << _name << endl;
		return "";
	}
	string m = _messages.front();
	_messages.erase(_messages.begin());
	_messageCount--;
	return m;
}

// addMessage()
// adds a message to the end of the queue
void MessageQueue::addMessage(const string &m)
{
	_messages.push_back(m);
	_messageCount++;
}
