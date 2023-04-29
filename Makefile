##
# chatroom
#

CXX := g++
CXXFLAGS := -Wall -lpthread

all:
	g++ -Wall -lpthread host.cc -o host
	g++ -Wall -lpthread client.cc -o client

clean:
	rm host client

# end
