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

// global variable stores client sockets
vector<long> clientSockets;

// this function is spawned for every client that connects, and runs until they disconnect
void *ReceiveAndSend(void *cs) {
  // const messages
  const char usernameMsg[] = "Enter a username:";
  const char welcomeMsg[] = "Welcome to the chat. To send a message, press enter.\n";
  string username = "Unkown participant";
  time_t currTime;
  int bytesRecv;
  char timeBuf[6];
  char buffer[4096];
  string message;

  // convert pointer to long with actual socket number
  long clientSocket = (long) cs;

  // prompt client for username, wait on response and store it
  // update broadcast status to other clients
  send(clientSocket, usernameMsg, 18, 0);
  int unBytesRecv = recv(clientSocket, buffer, 4096, 0);
  if (unBytesRecv == -1) {
    cerr << username << " was unable to join chat.\n";
    close(clientSocket); pthread_exit(NULL);
  }
  if (unBytesRecv == 0) {
    message = username + " has left the chat.\n"s;
    for (auto client : clientSockets)
      send(client, message.c_str(), message.length() + 1, 0);

    close(clientSocket); pthread_exit(NULL);
  }
  username = string(buffer);
  message = "\n" + username + " has joined the chat.\n"s;
  for (auto client : clientSockets)
    send(client, message.c_str(), message.length() + 1, 0);

  // send welcome message to client
  send(clientSocket, welcomeMsg, 54, 0);
  unsigned int msgLength = 4090 - username.length();

  // continually wait for and broadcast messages from client
  while (true) {
    // clear the buffer
    memset(buffer, 0, 4096);

    // blocking call waits for a message from client
    bytesRecv = recv(clientSocket, buffer, msgLength, 0);
    // mark current time
    currTime = time(nullptr);
    strftime(timeBuf, 6, "%H:%M", localtime(&currTime));

    // error messages
    if (bytesRecv == -1) {
      message = username + " had a connection issue.\n"s;
      for (auto client : clientSockets)
        send(client, message.c_str(), message.length() + 1, 0);
      break;
    }
    if (bytesRecv == 0) {
      message = username + " has left the chat.\n"s;
      for (auto client : clientSockets)
        send(client, message.c_str(), message.length() + 1, 0);
      break;
    }

    // successful message retrieval: concatenate with time and username, broadcast to other clients
    cout << username << string(buffer, sizeof(buffer)) << '\n'; // just for troubleshooting
    message = timeBuf + " "s + username + ": "s + buffer;
    for (auto client : clientSockets)
      send(client, message.c_str(), bytesRecv + unBytesRecv + 7, 0);
  }

  close(clientSocket);
  pthread_exit(NULL);

}


int main(int argc, char **argv) {

  // handle args or no args and store port number
  int portNumber;
  if (argc < 2) {
    cout << "Enter port number:\n";
    cin >> portNumber;
  }
  else portNumber = stoi(argv[1]);

  // establish a listening socket
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
  inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr); // 0.0.0.0 lets the IP default to the system its on

  if (bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1) {
    cerr << "Unable to bind to IP/port.";
    return -1;
  }
  cout << "Binded to IP/port\n";

  if (listen(listening, SOMAXCONN) == -1) {
    cerr << "Unable to listen.\n";
    return -1;
  }

  // struct sockaddr_in *listeningSocket = (struct sockaddr_in *) &listening;
  // cout << "Hosting chat on IP: " << inet_ntoa(listeningSocket->sin_addr) << " port number: " << listeningSocket->sin_port << '\n';

  sockaddr_in client;
  socklen_t clientSize = sizeof(client);

  // infinitely wait for clients to attatch themselves
  while (true) {
    cout << "Listening for clients...\n";

    // blocking call waits for client
    clientSocket = accept(listening, (struct sockaddr*)&client, &clientSize);

    if (clientSocket == -1) {
      cerr << "Failed to connect to client";
      return -1;
    }

    // add socket to global clientSockets vector
    clientSockets.push_back(clientSocket);

    // cout << "Incoming connection from IP: " << inet_ntoa(client.sin_addr) << ' ' << " port number: " << client.sin_port << '\n';

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
