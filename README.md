
DNS Forwarder C++

To use on Linux, open 2 terminal, one for client and one for server:

Server side:

g++ server.cpp -o server
./server 1.1.1.1 53

Server take 2 argument, first is upstream server and second is port.
By default it is set to 1.1.1.1 and 53

Client side:

g++ client.cpp -o client
./client 127.0.0.1 9000

client take 2 argument, first is ip and second is port

after run client, input domain name to Client terminal to start query
e.g google.com
