#include <stdio.h> 
#include <stdlib.h>
#include <string.h>   
#include <sys/socket.h>   
#include <arpa/inet.h> 
#include <time.h>
typedef struct Commodity_t{
    int machineID[9];
    char brandName[9][32];
    int  stock[9];
}Commodity;

void initializeCommodity(Commodity *commodity);

int main(){
	Commodity commodity;
	char* commoditySales;
	int sockfd, commodity_bought, id, i;
	struct sockaddr_in system;
	pid_t pid;
	time_t rawtime;
    struct tm* currentTime;
    char *msgMachineID, *msgStock, *msgYear, *msgMonth, *msgDate,*msgHour, *msgMinute, *msgSecond; 
    char *replyMessage, *replyMachineID, *replyBrandName, *replyStock;
	char *replyMessage2, *replyMachineID2, *replyBrandName2, *replyStock2;
	initializeCommodity(&commodity);

	//equipMain
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		printf("Create socket failed !\n");
		return 0;
	}

	system.sin_addr.s_addr = inet_addr("127.0.0.1");
	system.sin_family = AF_INET;
	system.sin_port = htons(8888);

	//connect to system
	if(connect(sockfd, (struct sockaddr *)&system, sizeof(system))<0){
		printf("Connect to system failed !\n");
		return 0;
	}

	printf("May 1: nha D1\n0. Coca\n1. Milo\n2. Lavie\nMay 2: nha D2\n3. Pepsi\n4. Revive\n5. Nutri\nMay 3: nha D9\n6. Ovantine\n7. Sting\n8. Aquarius\n");

	//create commodity Sales
	pid = fork();
		while(1){
			if(pid == 0){
				printf("Choose commodity: ");
				scanf("%d", &commodity_bought);
				commodity.stock[commodity_bought]--;
				if(commodity.stock[commodity_bought] < 0){
					printf("Sold out !\n");
					break;
				}
				//get current time to send to system
				time(&rawtime);
		    	currentTime = localtime(&rawtime);
				
				msgMachineID = (char *)malloc(sizeof(char));
				msgStock = (char *)malloc(2*(sizeof(char)));
				msgYear = (char *)malloc(4*(sizeof(char)));
				msgMonth = (char *)malloc(2*(sizeof(char)));
				msgDate = (char *)malloc(2*(sizeof(char)));
				msgHour = (char *)malloc(2*(sizeof(char)));
				msgMinute = (char *)malloc(2*(sizeof(char)));
				msgSecond = (char *)malloc(2*(sizeof(char)));
				commoditySales = (char *)malloc(100*(sizeof(char)));
				//create commoditySales information
				sprintf(msgMachineID, "%d", commodity.machineID[commodity_bought]);
				sprintf(msgStock, "%d", commodity.stock[commodity_bought]);
				sprintf(msgYear, "%d", currentTime->tm_year + 1900);
				sprintf(msgMonth, "%d", currentTime->tm_mon + 1);
				sprintf(msgDate, "%d", currentTime->tm_mday);
				sprintf(msgHour, "%d", currentTime->tm_hour);
				sprintf(msgMinute, "%d", currentTime->tm_min);
				sprintf(msgSecond, "%d", currentTime->tm_sec);
				sprintf(commoditySales, "%s,%s,%s,%s,%s,%s,%s,%s,%s", 
					msgMachineID, commodity.brandName[commodity_bought], msgStock, msgDate, msgMonth, msgYear, msgHour, msgMinute, msgSecond);
				
				if(send(sockfd, commoditySales, strlen(commoditySales)+1, 0) < 0){
					printf("Can't send commodity Sales\n");
				}

				//receive reply
				replyMessage = (char *)malloc(100 * sizeof(char));
				if(recv(sockfd, replyMessage, 100, 0) < 0){
					printf("recv failed !\n");
					break;
				}
				printf("System reply: %s\n", replyMessage);

				//replenish commodity if <= 3
				if(commodity.stock[commodity_bought] <= 3){
					printf("Receiving commodity\n");
					sleep(5);
					commodity.stock[commodity_bought] = 10;
					printf("Commodity has been replenished\n");
				}
				//determine which reply has been received
				// replyMessage2 = (char *)malloc(100 * sizeof(char));
				// if(recv(sockfd, replyMessage2, 100, 0) > 0){
				// 	printf("replyMessage2 %s\n", replyMessage2);
				// 	replyMachineID2 = strtok(replyMessage2, " ");
				// 	replyBrandName2 = strtok(NULL, " ");
				// 	if(replyBrandName2 != NULL){ 	//message to replenish commodity
				// 		replyStock2 = strtok(NULL, " ");
				// 		//find machine id and brandname
				// 		for(i=0; i<9; i++){
				// 			id = atoi(replyMachineID2);
				// 			if(id == commodity.stock[i] && (strcmp(replyBrandName2, commodity.brandName[i]))==0){
				// 				commodity.stock[i] = atoi(replyStock2);
				// 				printf("Receiving commodity\n");
				// 				sleep(5);
				// 				printf("Commodity has been replenished\n");
				// 			} 
				// 		}
				// 	}
				// }
			}
		}
		close(sockfd);
	
	return 0;
}

void initializeCommodity(Commodity *commodity){
	int iMachineID[9] = {1, 1, 1, 2, 2, 2, 3, 3, 3};
	char iBrandName[9][32] = {"Coca", "Milo", "Lavie", "Pepsi", "Revive", "Nutri", "Ovantine", "Sting", "Aquarius"};
	int  iStock[9] = {10, 10, 10, 10, 10, 10, 10, 10, 10};
	int i;
	for (i = 0; i < 9; i ++){
		commodity->machineID[i] = iMachineID[i];
		strcpy(commodity->brandName[i], iBrandName[i]);
		commodity->stock[i] = iStock[i];
	}
}