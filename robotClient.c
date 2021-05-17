#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"




// This is the main function that simulates the "life" of the robot
// The code will exit whenever the robot fails to communicate with the server
int main() {
  // ... ADD SOME VARIABLE HERE ... //
  int                 clientSocket, addrSize, bytesReceived;
  struct sockaddr_in  clientAddr;
  char                id;
  unsigned char       buffer[80];   // stores sent and received data
  int                 direction;
  char                sign;
  int                 x;
  int                 y;
  char                spinDirection = 0;
  
  // Set up the random seed
  srand(time(NULL));
  
  // Register with the server
  clientSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (clientSocket < 0) {
    printf("*** CLIENT ERROR: Could open socket.\n");
    exit(-1);
  }
  
  //Set up the address
  memset(&clientAddr, 0, sizeof(clientAddr));
  clientAddr.sin_family = AF_INET;
  clientAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
  clientAddr.sin_port = htons((unsigned short) SERVER_PORT);

  // Send register command to server.  Get back response data
  buffer[0] = REGISTER;
  sendto(clientSocket, buffer, sizeof(char), 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
  addrSize = sizeof(clientAddr);
  bytesReceived = recvfrom(clientSocket, buffer, 80, 0, (struct sockaddr *) &clientAddr, &addrSize);
  if(buffer[0] == NOT_OK){
  	exit(-1);
  }
  id = buffer[1];
  direction = buffer[2];
  sign = buffer[3];
  x = buffer[4] * 100 + buffer[5];
  y = buffer[6] * 100 + buffer[7];
  //If sign is 1, it is negative
  if(sign == 1){
  	direction = -direction;
  }
  // and store it.   If denied registration, then quit.
  
  
  // Go into an infinite loop exhibiting the robot behavior
  while (1) {
  
    // Check if can move forward
    buffer[0] = CHECK_COLLISION;
    buffer[1] = id;
    buffer[2] = direction;
    buffer[3] = 0;
    if(direction < 0){
    	buffer[2] = -direction;
    	buffer[3] = 1;
    }
    //Split the x and y values into low and high order
	buffer[4] = x / 100;
	buffer[5] = x % 100;
	buffer[6] = y / 100;
	buffer[7] = y % 100;
	//Send update to server
    sendto(clientSocket, buffer, REQUEST_SIZE, 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
    bytesReceived = recvfrom(clientSocket, buffer, 1, 0, (struct sockaddr *) &clientAddr, &addrSize);
    // Get response from server.
    // If ok, move forward
    if(buffer[0] == OK){
    	x += ROBOT_SPEED * cos((double) direction / 180.0 * PI);
    	y += ROBOT_SPEED *sin((double) direction / 180.0 * PI);
    	spinDirection = 0;
    }
    //If server shutdown, break out of the infinite loop
    else if(buffer[0] == LOST_CONTACT){
    	break;
    }
    // Otherwise, we could not move forward, so make a turn.
    else{
		//if robot is  stationary, choose a random 
		// direction to start turning.
		if(spinDirection == 0){
			spinDirection = rand() % 2 + 1;
		}
		//Otherwise, from the last time we collided, keep
		// turning in the same direction 
		if(spinDirection == 1){
			direction -= ROBOT_TURN_ANGLE;
			if(direction < -180){
				direction += 360;
			}
		}
		else if(spinDirection == 2){
			direction += ROBOT_TURN_ANGLE;
			if(direction > 180){
				direction -= 360;
			}
		}
    }
    // Send update to server
    buffer[0] = STATUS_UPDATE;
    buffer[1] = id;
    buffer[2] = direction;
    //buffer[3] stores the sign of the direction (1 is -direction, 0 is +direction)
    buffer[3] = 0;
    //Check if direction of robot is going left or right
    if(direction < 0){
    	buffer[2] = -direction;
    	buffer[3] = 1;
    }
	buffer[4] = x / 100;
	buffer[5] = x % 100;
	buffer[6] = y / 100;
	buffer[7] = y % 100;
    sendto(clientSocket, buffer, REQUEST_SIZE, 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
    
    // Uncomment line below to slow things down a bit 
    usleep(10000);   //change back to 10000 after testing
  }
}
