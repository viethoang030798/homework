#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
using namespace std;
//Server side

const std::string DNS_SERVER = "1.1.1.1";
   const int PORT = 53;
DNS::DNS()
{
 std::cout << "DNS Server default: " << DNS_SERVER << "\n\n";
}

void DNS::get(std::string url,std::string dns, int port)
{
 build_message(url);

 std::streamsize resp_sz = send_dns_request(dns,port);

 auto dns_response_header = reinterpret_cast<DNS_header*>(msg_buffer);
 auto qname = &msg_buffer[sizeof(DNS_header)];
 std::cout
   << "- Query\n"
   << "Name: "
   << qname_to_url(qname) << std::endl
   << "Answers RRs: "
   << ntohs(dns_response_header->AnswerRRs) << std::endl
   << "Authority RRs: "
   << ntohs(dns_response_header->AuthorityRRs) << std::endl
   << "Additional RRs: "
   << ntohs(dns_response_header->AdditionalRRs) << std::endl;

 // Pointer to first answer
 unsigned char* answer_ptr = reinterpret_cast<unsigned char*>(
   &msg_buffer[sizeof(DNS_header) +
   strlen(reinterpret_cast<char*>(qname)) + 1 +
   sizeof(QData)]);

 // Looping through all answers
 for (int i = 0, count = ntohs(dns_response_header->AnswerRRs); i < count; i++) {
   ResourceRecord* answer = reinterpret_cast<ResourceRecord*>(
     reinterpret_cast<char*>(answer_ptr) +
     strlen(reinterpret_cast<char*>(qname)) + 1);
   std::cout
     << "\n- Answer(" << i + 1 << ")\n"
     << "Type: " << ntohs(answer->rr_type) << std::endl
     << "Class: " << ntohs(answer->rr_class) << std::endl
     << "TTL: " << ntohl(answer->rr_ttl) << std::endl
     << "Data length: " << ntohs(answer->rr_rdlength) << std::endl
     << "Address: " << read_ip(answer) << std::endl;

   // Set pointer to next answer
   if (i < count - 1) {
     answer_ptr += sizeof(ResourceRecord) + sizeof(IP);
   }
 }
}

std::string DNS::read_ip(ResourceRecord* rr)
{
 std::stringstream res;
 IP* ip = reinterpret_cast<IP*>((unsigned char*)rr + sizeof(rr) + 2);

 for (int i = 0; i < 4; i++) {
   if (i != 0) res << ".";
   res << static_cast<int>((*ip)[i]);
 }
 return res.str();
}

/**
* Creating a socket and sending the contents of msg_buffer to it.
* Then reading the response back into msg_buffer.
* Returns size of the recived data.
**/
std::streamsize DNS::send_dns_request(std::string dns, int port)
{
 int socket_desc;
 struct sockaddr_in socket_addr;
 unsigned long socket_length;

 socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
 if (socket_desc == -1) {
   std::cerr << "Failed to create socket\n";
   exit(1);
 }

 socket_addr.sin_family = AF_INET;
 socket_addr.sin_port = htons(PORT);
 socket_addr.sin_addr.s_addr = inet_addr(dns.c_str());

 auto send_sz = sendto(
   socket_desc,
   msg_buffer,
   sizeof(DNS_header) + strlen(reinterpret_cast<char*>(qname)) + 1 + sizeof(QData),
   0,
   reinterpret_cast<struct sockaddr*>(&socket_addr),
   sizeof(socket_addr)
 );
 if (send_sz == -1) {
   std::cerr << "Error sending message to socket\n";
   close(socket_desc);
   exit(1);
 }

 socket_length = sizeof(socket_addr);
 auto recv_sz = recvfrom(
   socket_desc,
   msg_buffer,
   60000,
   0,
   reinterpret_cast<struct sockaddr*>(&socket_addr),
   reinterpret_cast<socklen_t*>(&socket_addr)
 );
 if (recv_sz == -1) {
   std::cerr << "Error reading from socket\n";
   close(socket_desc);
   exit(1);
 }

 // std::cout.write((const char*)msg_buffer, recv_sz);

 close(socket_desc);

 return recv_sz;
}

/**
* Adding data to dns_header, qname and qdata
* then placing them into msg_buffer
**/
void DNS::build_message(std::string url)
{
 dns_header = reinterpret_cast<DNS_header*>(&msg_buffer);
 dns_header->id = htons(123);
 dns_header->Questions = htons(1);
 dns_header->RecursionDesired = 1;

 qname = &msg_buffer[sizeof(DNS_header)];
 std::string qname_str = url_to_qname(url);
 memcpy(qname, qname_str.c_str(), sizeof(qname_str));

 qdata = reinterpret_cast<QData*>(
   &msg_buffer[sizeof(DNS_header) +
   strlen(reinterpret_cast<char*>(qname)) + 1]);
 qdata->qtype = htons(1);
 qdata->qclass = htons(1);
}

std::string DNS::url_to_qname(std::string& url)
{
 std::string result = "";
 std::string tmp = "";
 int i = 0;
 for (char c : url) {
   if (c == '.') {
     result += static_cast<char>(i) + tmp;
     i = 0;
     tmp = "";
     continue;
   }
   i++;
   tmp += c;
 }
 result += static_cast<char>(i) + tmp + '\0';
 return result;
}

/**
* Reverse of "url_to_qname"
**/
std::string DNS::qname_to_url(unsigned char* qname)
{
 std::string url = "";
 for (int i = 1; qname[i] != '\0'; i++) {
   if (
     (qname[i] >= 48 && qname[i] <= 57) ||
     (qname[i] >= 65 && qname[i] <= 90) ||
     (qname[i] >= 97 && qname[i] <= 122) ||
     qname[i] == 45) {
     url += qname[i];
   } else {
     url += '.';
   }
 }
 return url;
}


int main(int argc, char *argv[])
{

    //grab the port number
    int port = 9000;
    //buffer to send and receive messages with
    char msg[1500];

    //setup a socket and connection tools
    sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);

    //open stream oriented socket with internet address
    //also keep track of the socket descriptor
    int serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSd < 0)
    {
        cerr << "Error establishing the server socket" << endl;
        exit(0);
    }
    //bind the socket to its local address
    int bindStatus = bind(serverSd, (struct sockaddr*) &servAddr,
        sizeof(servAddr));
    if(bindStatus < 0)
    {
        cerr << "Error binding socket to local address" << endl;
        exit(0);
    }
    cout << "Waiting for a client to connect..." << endl;
    //listen for up to 5 requests at a time
    listen(serverSd, 5);
    //receive a request from client using accept
    //we need a new address to connect with the client
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    //accept, create a new socket descriptor to
    //handle the new connection with client
    int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
    if(newSd < 0)
    {
        cerr << "Error accepting request from client!" << endl;
        exit(1);
    }
    cout << "Connected with client!" << endl;
    //lets keep track of the session time
    struct timeval start1, end1;
    gettimeofday(&start1, NULL);
    //also keep track of the amount of data sent as well
    int bytesRead, bytesWritten = 0;
    while(1)
    {
        //receive a message from the client (listen)
        cout << "Awaiting client response..." << endl;
        memset(&msg, 0, sizeof(msg));//clear the buffer
        bytesRead += recv(newSd, (char*)&msg, sizeof(msg), 0);
        if(!strcmp(msg, "exit"))
        {
            cout << "Client has quit the session" << endl;
            break;
        }
        cout << "Client: " << msg << endl;
        string s(msg);
        DNS dns;
        std::string domain = "hs.fi";
        std::string dns_server = "1.1.1.1";
        int port1 = 53;
        if (argc > 1) {
          domain = s;
          dns_server = argv[1];
          port1 = atoi(argv[2]);
          std::cout << "DNS Server: " << dns_server << "\n\n";
          std::cout << "Port: " << port1 << "\n\n";
        }
        try {
          dns.get(domain,dns_server,port1);
        } catch (...) {
          std::cerr << "Error!!!.\n";
          return 1;
        }

        cout << ">";
        string data;
        getline(cin, data);
        memset(&msg, 0, sizeof(msg)); //clear the buffer
        strcpy(msg, data.c_str());
        if(data == "exit")
        {
            //send to the client that server has closed the connection
            send(newSd, (char*)&msg, strlen(msg), 0);
            break;
        }
        //send the message to client
        bytesWritten += send(newSd, (char*)&msg, strlen(msg), 0);
    }
    //close the socket descriptors
    gettimeofday(&end1, NULL);
    close(newSd);
    close(serverSd);
    cout << "********Session********" << endl;
    cout << "Bytes written: " << bytesWritten << " Bytes read: " << bytesRead << endl;
    cout << "Elapsed time: " << (end1.tv_sec - start1.tv_sec)
        << " secs" << endl;
    cout << "Connection closed..." << endl;
    return 0;
}
