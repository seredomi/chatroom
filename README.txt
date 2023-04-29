Nika Daroui and Sereno Dominguez
CS 370 Operating Systems



hey man, heres my description of the project so far...


host.cc is meant to run on the actual computer computer
here's what it does:
    - takes one argument, the listening port on host machine
    - establishes a listening socket, this continually monitors for connections
        - if a client connects, it adds its socket to the global clientSockets vector and spawns a new ReceiveAndSend thread
            - this thread is dedicated to this client
            - it continually waits for messages, when one comes, it is sent to all the clients in the global clientSockets vector


client.cc is meant to run on a pi, but i'm pretty sure it can run on any posix machine
here's what it does:
    - takes two arguments, the host ip address and the host listening port, which it tries to connect to
    - it spawns a ReceiveMessages thread, which continually waits on chat updates from the host
    - it also continually prompts the user to enter a message and relays that to the host's ReceiveAndSend



things i think we can implement in the next 2-3 days (feel free to add to this list and we can just cross things off as we go):

- better prompts?
- testing on Mac, testing on CSU computers

things that might be a little more tricky but are still plausible:

- keep a message log? (probably requires like a mutex or some shared memory safeguard thing)
- chatroom name?
- create multiple chatrooms? (might be overkill)
