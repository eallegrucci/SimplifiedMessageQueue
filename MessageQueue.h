#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H
#include <vector>
#include <iostream>
#include <string>
#include <fstream>

// MessageQueue class to add, retrieve, and count
// the number of message in the queue.
// MessageQueue is a FIFO list implemented to simplify
// the client-server program.
class MessageQueue {
	std::string _name;
	int _messageCount;
	std::vector<std::string> _messages;
public:
	MessageQueue();	
	int getMessageCount() const;
	bool containsMessages() const;
	std::string getMessage();
	void setName(std::string name);
	void addMessage(const std::string &m);
	void updateTempFile();
};


#endif /* MESSAGEQUEUE_H */
