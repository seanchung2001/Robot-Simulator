#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"


int main() {
  int                 clientSocket, addrSize, bytesReceived;
  struct sockaddr_in  clientAddr;
  unsigned char       buffer[80];   // stores sent and received data
  buffer[0] = STOP;

  // Create socket
  clientSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (clientSocket < 0) {
	printf("*** CLIENT ERROR: Could open socket.\n");
	exit(-1);
  }

  // Setup address 
  memset(&clientAddr, 0, sizeof(clientAddr));
  clientAddr.sin_family = AF_INET;
  clientAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
  clientAddr.sin_port = htons((unsigned short) SERVER_PORT);
	//Indicate the environment and robot are shutting down
	printf("SHUTDOWN: Sent shutDown\n");
	

	// Send command string to server
	sendto(clientSocket, buffer, sizeof(unsigned char), 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));

	// Get response from server, should be "OK"
	addrSize = sizeof(clientAddr);
	bytesReceived = recvfrom(clientSocket, buffer, 80, 0, (struct sockaddr *) &clientAddr, &addrSize);
	buffer[bytesReceived] = 0; // put a 0 at the end so we can display the string
	printf("SHUTDOWN: Got back response \"%s\" from server.\n", buffer);


  close(clientSocket);  // Don't forget to close the socket !
  printf("SHUTDOWN: Shutting down.\n");

}
