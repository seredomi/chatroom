#include <iostream>
#include <vector>
#include <string>
#include <string.h>

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

using namespace std;


// this function monitors for chat updates, and is meant to run alongside the main method that takes user input
void *ReceiveMessages(void *hs) {
  long hostSocket = (long) hs;
  char buffer[906];
  int bytesRecvd;

  while (true) {
    // blocking call waits on message from host
    bytesRecvd = recv(hostSocket, buffer, 4096, 0);
    if (bytesRecvd == -1) {
      cerr << "Connection error.\n";
      break;
    }
    if (bytesRecvd == 0) {
      cerr << "Chat server shut down.\n";
      break;
    }
    cout << string(buffer, bytesRecvd) << '\n';
  }

  close(hostSocket); pthread_exit(NULL);

}


// also could use some tidying up
int main(int argc, char **argv) {

  int port;
  string ip;

  if (argc < 3) {
    cout << "Enter IP:\n";
    cin >> ip;
    cout << "Enter port:\n";
    cin >> port;
  }
  else {
    ip = argv[1];
    port = stoi(argv[2]);
  }

  // create the socket and stuff
  long sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock == -1) {
    cerr << "Unable to create socket\n";
    return -1;
  }

  sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(port);
  inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);

  int connectCode = connect(sock, (sockaddr*) &hint, sizeof(hint));
  if (connectCode == -1) {
    cerr << "Unable to connect\n";
    return -1;
  }

  // run the parallel thread that monitors for chat updates
  pthread_t msgReceiver;
  int threadCode;
  threadCode = pthread_create(&msgReceiver, NULL, ReceiveMessages, (void *) sock);
  if (threadCode) {
    cerr << "Failed to create thread\n";
    return -1;
  }

  // continually prompt user to input message, transmit message back to host
  while (true) {
    string userIn;

    getline(cin, userIn);

    int sendCode = send(sock, userIn.c_str(), userIn.size() + 1, 0);
    if (sendCode == -1) {
        cerr << "Unable to transmit message\n";
        return -1;
    }

  }

  close(sock);
}
