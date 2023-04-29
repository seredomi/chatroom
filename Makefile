##
# chatroom
#

CXX := g++
CXXFLAGS := -Wall -lpthread

all:
	g++ -Wall -pthread host.cc -o host
	g++ -Wall -pthread client.cc -o client

clean:
	rm host client

# end
