
DNS Forwarder C++ (compiled file included)

Program structure has 2 parts, client side take domain input and connection info then send to server, server talk to upstream to quering and retrieve corresponding data.




****To build, open 2 terminal, one for client and one for server, input these commands:
```
g++ server.cpp -o server
```
```
g++ client.cpp -o client
```



****To run the compiled files, input these commands:

---On Server side---
```
./server 1.1.1.1 53
```

Server take 2 argument, first is upstream server address and second is port.
By default it is set to 1.1.1.1 and 53

---On Client side---
```
./client 127.0.0.1 9000
```
Client take 2 argument, first is ip and second is port

After run server and client, input domain name to Client terminal to start query

e.g google.com
