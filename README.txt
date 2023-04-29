Nika Daroui and Sereno Dominguez
CS 370 Operating Systems

host.cc is intended to run on the actual computer computer
here's what it does:
    - takes one argument, the listening port on host machine
    - establishes a listening socket, this continually monitors for connections
        - if a client connects, it adds its socket to the global clientSockets vector and spawns a new ReceiveAndSend thread
            - this thread is dedicated to this client
            - it continually waits for messages, when one comes, it is sent to all the clients in the global clientSockets vector


client.cc is intended to run on a pi
here's what it does:
    - takes two arguments, the host ip address and the host listening port, which it tries to connect to
    - it spawns a ReceiveMessages thread, which continually waits on chat updates from the host
    - it also continually prompts the user to enter a message and relays that to the host's ReceiveAndSend

