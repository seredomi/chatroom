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
#include <ctime>


using namespace std;

// global variable to store client sockets until we find a better solution
vector<long> clientSockets;



// this function is spawned for every client that connects, and runs until they disconnect
void *ReceiveAndSend(void *cs) {
  const char usernameMsg[] = "Enter a username (must be less than 20 characters): \n";
  string username = "Unkown participant";
  time_t currTime;
  int bytesRecv;
  char timeBuf[6];
  string message;

  // convert pointer thing to actual socket number
  long clientSocket = (long) cs;
  // buffer will store the message as it comes from client
  char buffer[4096];

  send(clientSocket, usernameMsg, 53, 0);
  int unBytesRecv = recv(clientSocket, buffer, 4096, 0);
  if (unBytesRecv == -1) {
    cerr << username << " unable to join chat.\n";
    close(clientSocket); pthread_exit(NULL);
  }
  if (unBytesRecv == 0) {
    cerr << username << " left the chat.\n";
    close(clientSocket); pthread_exit(NULL);
  }
  username = string(buffer);
  cout << username << " joined the chat.\n";


  while (true) {
    // clear the buffer
    memset(buffer, 0, 4096);

    // blocking call waits for a message from client
    bytesRecv = recv(clientSocket, buffer, 4096, 0);
    currTime = time(nullptr);
    //memset(timeBuf, 0, 6);
    strftime(timeBuf, 6, "%H:%M", localtime(&currTime));

    if (bytesRecv == -1) {
      cerr << username << "had a connection issue.\n";
      break;
    }
    if (bytesRecv == 0) {
      cout << username << " left the chat.\n";
      break;
    }

    message = timeBuf + " "s + username + ": "s + buffer;

    cout << "received " << string(buffer, sizeof(buffer)) << '\n'; // just for troubleshooting
    // transmit message to every client in the global vector
    for (auto client : clientSockets)
      send(client, message.c_str(), bytesRecv + unBytesRecv + 8, 0);
  }

  close(clientSocket);
  pthread_exit(NULL);

}



// this could probably get broken up into a function or two
int main(int argc, char **argv) {
  int port;

  int portNumber;
  if (argc < 2) {
    cout << "Enter port number:\n";
    cin >> port;
  }
  else portNumber = stoi(argv[1]);


  // all this just establishes a listening socket
  int listening;
  long clientSocket;

  listening = socket(AF_INET, SOCK_STREAM, 0);
  if (listening == -1) {
    cerr << "Unable to create socket.";
    return -1;
  }
  cout << "Created socket\n";

  struct sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(portNumber);
  inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr); // 0.0.0.0 lets the IP default to the system its on or something

  if (bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1) {
    cerr << "Unable to bind to IP/port.";
    return -1;
  }
  cout << "Binded to IP/port\n";

  if (listen(listening, SOMAXCONN) == -1) {
    cerr << "Unable to listen";
    return -1;
  }

  struct sockaddr_in *listeningSocket = (struct sockaddr_in *) &listening;
  cout << "Hosting chat on IP: " << inet_ntoa(listeningSocket->sin_addr) << " port number: " << listeningSocket->sin_port << '\n';


  // infinitely wait for clients to attatch themselves
  while (true) {
    cout << "Listening for clients...\n";

    sockaddr_in client;
    socklen_t clientSize = sizeof(client);

    // blocking call waits for client
    clientSocket = accept(listening, (struct sockaddr*)&client, &clientSize);

    if (clientSocket == -1) {
      cerr << "Failed to connect to client";
      return -1;
    }

    // add socket to global vector
    clientSockets.push_back(clientSocket);


    cout << "Incoming connection from IP: " << inet_ntoa(client.sin_addr) << ' ' << " port number: " << client.sin_port << '\n';

    // create a thread that waits for messages from that client
    pthread_t newClient;
    int threadCode;
    threadCode = pthread_create(&newClient, NULL, ReceiveAndSend, (void *) clientSocket);
    if (threadCode) {
      cerr << "Failed to create thread";
      return -1;
    }

  }

  pthread_exit(NULL);

}
