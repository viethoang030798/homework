
DNS Forwarder C++

Program structure has 2 parts, client side take domain input and connection info then send to server, server talk to upstream and retrieve corresponding data.

To use on Linux, open 2 terminal, one for client and one for server:

-------Server side:

g++ server.cpp -o server

./server 1.1.1.1 53

Server take 2 argument, first is upstream server address and second is port.
By default it is set to 1.1.1.1 and 53

------Client side:

g++ client.cpp -o client

./client 127.0.0.1 9000

Client take 2 argument, first is ip and second is port

after run client, input domain name to Client terminal to start query
e.g google.com
