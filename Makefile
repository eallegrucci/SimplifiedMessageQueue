all: server.o client.o MessageQueue.o
	g++ -std=c++11 -pthread server.o MessageQueue.o -o server
	g++ -std=c++11 -pthread client.o MessageQueue.o -o client

server.o: server.cpp
	g++ -std=c++11 -pthread -c server.cpp

client.o: client.cpp
	g++ -std=c++11 -pthread -c client.cpp

MessageQueue.o: MessageQueue.cpp
	g++ -std=c++11 -pthread -c MessageQueue.cpp

#Exchange.o: Exchange.cpp
#	g++ -std=c++11 -c Exchange.cpp

debug: server.o client.o
	g++ -std=c++11 -pthread -g -o server server.cpp
	g++ -std=c++11 -pthread -g -o client client.cpp

clean:
	rm -rf *.o *~ server client core
