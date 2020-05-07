#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define PORT 30000

//Primitives
#define PACKETID 0XFFFF
#define CLIENTID 0XFF
#define ENDPACKETID 0XFFFF

//server responses
#define PAID 0XFFFB
#define NOTPAID 0XFFF9
#define NOTEXIST 0XFFFA

//Access permission request packet format
struct requestPacket {
	
	uint16_t packetID;
	uint8_t clientID;
	uint16_t Acc_Per;
	uint8_t segment_No;
	uint8_t length;
	uint8_t technology;
	unsigned int SourceSubscriberNo;
	uint16_t endpacketID;
};

//response packet
struct responsePacket {
	
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_No;
	uint8_t length;
	uint8_t technology;
	unsigned int SourceSubscriberNo;
	uint16_t endpacketID;
};

//print all the packet details
void printPacketDetails(struct requestPacket request) {
	
	printf(" packetID: %x\n",request.packetID);
	printf("Client id : %hhx\n",request.clientID);
	printf("Access permission: %x\n",request.Acc_Per);
	printf("Segment no : %d \n",request.segment_No);
	printf("length %d\n",request.length);
	printf("Technology %d \n", request.technology);
	printf("Subscriber no: %u \n",request.SourceSubscriberNo);
	printf("end of datapacket id : %x \n",request.endpacketID);
}

//create requestPacket with data
struct requestPacket createRequestPacket() {
	
	struct requestPacket request;
	request.packetID = PACKETID;
	request.clientID = CLIENTID;
	request.Acc_Per = 0XFFF8;
	request.endpacketID = ENDPACKETID;
	
	return request;
}


int main(int argc, char**argv){
	
	struct requestPacket request;
	struct responsePacket response;
	char line[30];
	FILE *fp;
	int sockfd,n = 0;
	struct sockaddr_in cliaddr;
	socklen_t addr_size;
	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	int i = 1;
	
	//socket timeout
	struct timeval timer;
	timer.tv_sec = 3;
	timer.tv_usec = 0;	
	
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timer,sizeof(struct timeval));
	int retryCounter = 0;
	if(sockfd < 0) {
		printf("\n ERROR: Socket Failure \n");
	}
	
	bzero(&cliaddr,sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	cliaddr.sin_port=htons(PORT);
	addr_size = sizeof cliaddr;
	request = createRequestPacket();

	char con='y';	
	while(con=='y'){	
		char words[20];
		printf("Please Enter the mobile number:\n");
        	scanf("%s",words);

		n = 0;
		retryCounter = 0;
		printf("\n");
		request.length = strlen(words);
		request.SourceSubscriberNo = (unsigned) atoi(words);
		request.length += strlen(words);
		int tech=0;
		printf("Enter Technology?");
		scanf("%d",&tech);
		request.technology = tech;
		request.segment_No = i;
		
		printPacketDetails(request);
		while(n <= 0 && retryCounter < 3) {
			
			//send and recieve packets
			sendto(sockfd,&request,sizeof(struct requestPacket),0,(struct sockaddr *)&cliaddr,addr_size);
			n = recvfrom(sockfd,&response,sizeof(struct responsePacket),0,NULL,NULL);
			
			if(n <= 0 ) {
				printf("\n ERROR: No response from server\n");
				printf("Sending packet again \n");
				retryCounter++;
			}
			
			else if(n > 0) {
				printf("\nINFO: \n");
				if(response.type == NOTPAID) {
					printf("Subscriber has not paid message \n");
				}
				else if(response.type == NOTEXIST ) {
					printf("Subscriber does not exist on database message \n");
				}
				else if(response.type == PAID) {
					printf("Subscriber is permitted to access the network message \n");

				}
			}
		}
		
		printf("Do you want to continue? y/n \n");
		scanf(" %c",&con);
		
		//no ACK recieved after sending packet 3 times
		if(retryCounter>= 3 ) {
			printf("\n ERROR: Server does not respond \n");
			exit(0);
		}
		printf("\n ---------------------------------------------------------------------- \n");
		i++;
			
	}
}
