#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <strings.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 30000
#define ENTRY 10

//response messages
#define PAID 0XFFFB
#define NOTPAID 0XFFF9
#define NOTEXIST 0XFFFA

//Request packet format
struct requestPacket{
	
	uint16_t packetID;
	uint8_t clientID;
	uint16_t Acc_Per;
	uint8_t segment_No;
	uint8_t length;
	uint8_t technology;
	unsigned int SourceSubscriberNo;
	uint16_t endpacketID;
};

//Response packet format
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

// function to create a packet for response
struct responsePacket createResponsePacket(struct requestPacket request) {
	
	struct responsePacket response;
	
	response.packetID = request.packetID;
	response.clientID = request.clientID;
	response.segment_No = request.segment_No;
	response.length = request.length;
	response.technology = request.technology;
	response.SourceSubscriberNo = request.SourceSubscriberNo;
	response.endpacketID = request.endpacketID;

	return response;
}

//print all the packet details
void printPacketDetails(struct requestPacket request) {
	
	printf(" PacketID: %x\n",request.packetID);
	printf(" Client id : %hhx\n",request.clientID);
	printf(" Access permission: %x\n",request.Acc_Per);
	printf(" Segment no : %d \n",request.segment_No);
	printf(" Length %d\n",request.length);
	printf(" Technology %d \n", request.technology);
	printf(" Subscriber no: %u \n",request.SourceSubscriberNo);
	printf(" End of request packet id : %x \n",request.endpacketID);
}

//create map to store file contents
struct Map {
	
	unsigned long subscriberNumber;
	uint8_t technology;
	int status;
};

void readFile(struct Map map[]) {

	//save the file on server
	char line[30];
	int i = 0;
	FILE *fp;

	fp = fopen("Verification_Database.txt", "rt");
	
	if(fp == NULL)
	{
		printf("\n ERROR: File not found \n");
		return;
	}
	
	while(fgets(line, sizeof(line), fp) != NULL)
	{
		char * words;

		words = strtok(line," ");

		map[i].subscriberNumber =(unsigned) atol(words);
		words = strtok(NULL," ");

		map[i].technology = atoi(words);
		words = strtok(NULL," ");

		map[i].status = atoi(words);
		i++;
	}
	fclose(fp);
}

//check status of subscriber
int check(struct Map map[],unsigned int subscriberNumber,uint8_t technology) {
	int value = -1;
	for(int j = 0; j < ENTRY;j++) {
		if(map[j].subscriberNumber == subscriberNumber && map[j].technology == technology) {
			return map[j].status;
		}
	}
	return value;
}


int main(int argc, char**argv){
	
	struct requestPacket request;
	struct responsePacket response;
	struct Map map[ENTRY];
	readFile(map);
	int sockfd,n;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddr.sin_port=htons(PORT);
	bind(sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
	addr_size = sizeof serverAddr;
	printf("\n Server started successfully \n");
	
	while(1) {
		
		//wait and recieve client packet
		n = recvfrom(sockfd,&request,sizeof(struct requestPacket),0,(struct sockaddr *)&serverStorage, &addr_size);
		
		//print the recieved packet details
		printPacketDetails(request);
		
		//to check for ack_timer
		/*if(request.segment_No == 9) {
			exit(0);
		}*/

		if(n > 0 && request.Acc_Per == 0XFFF8) {
			
			response = createResponsePacket(request);

			int value = check(map,request.SourceSubscriberNo,request.technology);
			
			//subscriber has not paid
			if(value == 0) {
				response.type = NOTPAID;
				printf("\n INFO: Subscriber has not paid \n");
			}
			
			//subscriber does not exist on database
			else if(value == -1) {
				printf("\n INFO: Subscriber does not exist on database \n");
				response.type = NOTEXIST;
			}
			
			//subscriber permitted to acces the network
			else if(value == 1) {
				printf("\n INFO: Subscriber permitted to access the network \n");
				response.type = PAID;
			}
			
			//send response packet
			sendto(sockfd,&response,sizeof(struct responsePacket),0,(struct sockaddr *)&serverStorage,addr_size);
		}
		n = 0;
		printf("\n ---------------------------------------------------------------------- \n");
	}
}



