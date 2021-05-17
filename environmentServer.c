#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "simulator.h"


Environment    environment;  // The environment that contains all the robots





// Handle client requests coming in through the server socket.  This code should run
// indefinitiely.  It should repeatedly grab an incoming messages and process them. 
// The requests that may be handled are STOP, REGISTER, CHECK_COLLISION and STATUS_UPDATE.   
// Upon receiving a STOP request, the server should get ready to shut down but must
// first wait until all robot clients have been informed of the shutdown.   Then it 
// should exit gracefully.  
void *handleIncomingRequests(void *e) {
    char   online = 1;
	// ... ADD SOME VARIABLE HERE... //
  	int                 serverSocket;
    struct sockaddr_in  serverAddr, clientAddr;
    int                 status, addrSize, bytesReceived;
    fd_set              readfds, writefds;
    unsigned char       buffer[80];
    char*               response = "OK";
    int                 totalActive = 0;
    int                 id;
    int                 direction;
    int                 sign;
    int                 x;
    int                 y;
  	Environment *env = (Environment*)e;

  	// Initialize the server
  	//Open the socket
  	serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket < 0) {
    	printf("*** SERVER ERROR: Could not open socket.\n");
    	exit(-1);
    }
    //Create the address
    memset(&serverAddr, 0, sizeof(serverAddr)); // zeros the struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons((unsigned short) SERVER_PORT);
  	
  	//Bind the socket
  	status = bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
  	if (status < 0) {
    	printf("*** SERVER ERROR: Could not bind socket.\n");
    	exit(-1);
  	}

  	// Wait for clients now
	while (online) {
		// ... WRITE YOUR CODE HERE ... //
		//Select a socket request
    	FD_ZERO(&readfds);
    	FD_SET(serverSocket, &readfds);
    	FD_ZERO(&writefds);
    	FD_SET(serverSocket, &writefds);
    	//printf("Server: Waiting for packet...\n");
    	status = select(FD_SETSIZE, &readfds, &writefds, NULL, NULL);
    	if (status == 0) {
      		// Timeout occurred, no client ready
    	}
    	//Check if there was an error selecting the socket
    	else if (status < 0) {
    	  	printf("*** SERVER ERROR: Could not select socket.\n");
   		   	exit(-1);
    	}
    	//If everything worked out properly (no errors)
    	else {
    	  //Get the size of clientAddr
    	  addrSize = sizeof(clientAddr);
    	  //Receive the buffer from client
    	  bytesReceived = recvfrom(serverSocket, &buffer, sizeof(buffer), 0, (struct sockaddr *) &clientAddr, &addrSize);
    	  	//Should have received at least 1 byte
      		if (bytesReceived > 0) {
      			//Process the request
       		 	//printf("SERVER: Received client request: %d\n", buffer[0]);
       		 	//Handle STOP request
       		 	if(buffer[0] == STOP){
       		 		//Sent the shutDown signal
       		 		env->shutDown = 1;
       		 		//totalActive will store the number of robots that still need to be shut down
       		 		totalActive = env->numRobots;
       		 		printf("SERVER: Sending \"%s\" to client\n", response);
       		 		//Send STOP process
      				sendto(serverSocket, response, strlen(response), 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
      				//Check if there are any robots left to shut down
      				if(totalActive == 0){
      					break;
      				}
       		 	}
       		 	//Handle REGISTER request to register a new robot into the environment
       		 	else if(buffer[0] == REGISTER){
       		 		//Check if we can add more robots (if numRobots is less than 20)
       		 		if(env->numRobots == MAX_ROBOTS || env->shutDown == 1){
       		 			printf("SERVER: Sending NOT_OK to client\n");
       		 			buffer[0] = NOT_OK;
       		 			//Send NOT_OK process
      					sendto(serverSocket, buffer, sizeof(char), 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
       		 		}
       		 		else{
       		 			//Set appropriate elements for buffer
       		 			buffer[0] = OK;
       		 			buffer[1] = env->numRobots;
       		 			buffer[2] = rand() % 181;
       		 			env->robots[env->numRobots].direction = buffer[2];
       		 			buffer[3] = rand() % 2;
       		 			if(buffer[3] == 1){
       		 				env->robots[env->numRobots].direction = -buffer[2];
       		 			}
       		 			//Unneccessary, can keep it x and y at 0 for start value
       		 			x = 15;
       		 			y = 15;
       		 			//flag to see if robots collide upon registering
       		 			char collides = 1;
       		 			//Continue until robots don't collide upon registration
       		 			while(collides == 1){
       		 				collides = 0;
       		 				x = rand() % (ENV_SIZE - 2*ROBOT_RADIUS + 1) + ROBOT_RADIUS;
       		 				y = rand() % (ENV_SIZE - 2*ROBOT_RADIUS + 1) + ROBOT_RADIUS;
       		 				for(int i = 0; i < env->numRobots; i++){
    							int d = sqrt(pow(x - env->robots[i].x, 2) + pow(y - env->robots[i].y, 2));
    							if(d < 2*ROBOT_RADIUS){
    								collides = 1;
    								break;
    							}
    						}
    					}
    					//Set appropriate x and y values in high and low end order
       		 			buffer[4] = x / 100;
       		 			buffer[5] = x % 100;
       		 			env->robots[env->numRobots].x = x;
       		 			buffer[6] = y / 100;
       		 			buffer[7] = y % 100;
       		 			env->robots[env->numRobots].y = y;
       		 			env->numRobots++;
       		 			printf("SERVER: Registering robot (%d, %d)\n", buffer[4] * 100 + buffer[5], buffer[6] * 100 + buffer[7]);
       		 			printf("(%d, %d)\n", x, y);
       		 			//Send registered robot
       		 			sendto(serverSocket, buffer, REQUEST_SIZE, 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
       		 		}
       		 	}
       		 	//Handle CHECK_COLLISION request
       		 	else if(buffer[0] == CHECK_COLLISION){
       		 		//Check if the environment has been shutdown first
       		 		if(env->shutDown == 1){
       		 			printf("SERVER: Lost contact\n");
       		 			buffer[0] = LOST_CONTACT;
       		 			//Send updated robot
       		 			sendto(serverSocket, buffer, REQUEST_SIZE, 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
       		 			totalActive--;
       		 			if(totalActive == 0){
       		 				break;
       		 			}
       		 		}
       		 		//Set appropriate elements for buffer
       		 		id = buffer[1];
  					direction = buffer[2];
 				    if(buffer[3] == 1){
 				    	direction = -direction;
 				    }
  					x = buffer[4] * 100 + buffer[5];
  					y = buffer[6] * 100 + buffer[7];
  					x += ROBOT_SPEED * cos((double) direction / 180.0 * PI);
    				y += ROBOT_SPEED *sin((double) direction / 180.0 * PI);
    				//Handle robots going out of bounds
    				if(x < ROBOT_RADIUS || y < ROBOT_RADIUS || x > ENV_SIZE - ROBOT_RADIUS || y > ENV_SIZE - ROBOT_RADIUS){
    					buffer[0] = NOT_OK_BOUNDARY;
    					//Send updated robot
    					sendto(serverSocket, buffer, REQUEST_SIZE, 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
    				}
    				//Handle robot collisions
    				else{
    					char flag = 0;
    					for(int i = 0; i < env->numRobots; i++){
    						if(i != id){
    							int d = sqrt(pow(x - env->robots[i].x, 2) + pow(y - env->robots[i].y, 2));
    							if(d < 2*ROBOT_RADIUS){
    								flag = 1;
    								break;
    							}
    						}
    					}
    					if(flag == 1){
    						buffer[0] = NOT_OK_COLLIDE;
    					}
    					else{
    						buffer[0] = OK;
    					}
    					//Send the updated robot
    					sendto(serverSocket, buffer, REQUEST_SIZE, 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
    				}
       		 	}
       		 	//Handle STATUS_UPDATE
       		 	else if(buffer[0] == STATUS_UPDATE){
       		 		//Set appropriate elements for buffer
       		 		id = buffer[1];
  					direction = buffer[2];
  					x = buffer[4] * 100 + buffer[5];
  					y = buffer[6] * 100 + buffer[7];
  					//If sign is 1, it is negative
  					if(buffer[3] == 1){
  						direction = -direction;
  					}
  					env->robots[id].x = x;
  					env->robots[id].y = y;
  					env->robots[id].direction = direction;
       		 	}
      			
      		}
      	}
	}
		
		
		
  	// ... WRITE ANY CLEANUP CODE HERE ... //
  	//Close server socket
  	close(serverSocket);
  	printf("SERVER: Shutting down.\n");
  	
}




int main() {
	pthread_t t1, t2;
	sem_t semaphor;
	
	// So far, the environment is NOT shut down
	environment.shutDown = 0;
  
	// Set up the random seed
	srand(time(NULL));

	// Spawn an infinite loop to handle incoming requests and update the display

		pthread_create(&t1, NULL, handleIncomingRequests, &environment);
		pthread_create(&t2, NULL, redraw, &environment);


	// Wait for the update and draw threads to complete
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
}
