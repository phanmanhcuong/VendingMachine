#include <stdio.h> 
#include <stdlib.h>
#include <string.h>   
#include <sys/socket.h>   
#include <arpa/inet.h> 
#include <time.h>
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 

#define SHIMSIZE 10

int main(){
	int listenfd = 0, connfd = 0, pid, recvm, c;
    struct sockaddr_in serv_addr, cli_addr; 
    char *clientMessage, *machineID, *brandName, *stock, *date, *month, *year, *hour, *minute, *second;
  	char *shmMessage;
    FILE *historyFile, *inventoryFile;
    key_t key = 1234;
    int shmidWrite, shmidRead, *head, *begin;
    char *invent, *inventMachineID, *inventBrandName, *inventStock, *inventAmount;
    int confirm, stockNumber, msgFlag = 0;
    char character, success[50], error[50];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1){
    	printf("Create socket failed !\n");
    	return 0;
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8888);
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
    	printf("Bind failed !\n");
    	return 0;
    }

    listen(listenfd, 3);
    c = sizeof(struct sockaddr_in);
    for(;;){
    	connfd = accept(listenfd, (struct sockaddr*)&cli_addr, (socklen_t*)&c);
    	pid = fork();
    	if(pid < 0){
    		close(connfd);
    	}
    	else if (pid > 0){
    		close(connfd);
    	}
    	else{
    		if(connfd < 0){
    			printf("Accept failed !\n");
    			return 1;
    		}

    		//read purchased commodity information from socket
    		clientMessage = (char *)malloc(100 *(sizeof(char)));
    		invent = (char *)malloc(100 *(sizeof(char)));
    		for(;;){
    			recvm = recv(connfd, clientMessage, 100, 0);
    			machineID = strtok(clientMessage, ",");
    			brandName = strtok(NULL, ",");
    			stock = strtok(NULL, ",");
    			date = strtok(NULL, ",");
    			month = strtok(NULL, ",");
    			year = strtok(NULL, ",");
    			hour = strtok(NULL, ",");
    			minute = strtok(NULL, ",");
    			second = strtok(NULL, ",");
    			
    			printf("Purchased Commodity: %s\n",brandName);

    			//write to history file
    			if((historyFile = fopen("history.txt", "r+")) == NULL){
    				printf("History file doesn't exist !\n");
    			}

    			//move to end of history file to write new line
    			fseek(historyFile, 0, SEEK_END);
    			fprintf(historyFile, "%s\t%s\t%s/%s/%s %s:%s:%s\n", machineID, brandName, date, month, year, hour, minute, second);
    			fclose(historyFile);

    			//write to inventory file
    			if((inventoryFile = fopen("inventory.txt", "r+")) == NULL){
    				printf("Inventory file doesn't exist !\n");
    			}
    			//read line from file
    			while((fgets(invent, 100, inventoryFile)) != NULL){
    				inventMachineID = strtok(invent, ",");
    				inventBrandName = strtok(NULL, ",");
    				inventStock = strtok(NULL, ",");

    				//compare machineID and brandName
    				if((strcmp(inventMachineID, machineID)) == 0 && (strcmp(inventBrandName, brandName)) == 0){
    					fseek(inventoryFile, -4, SEEK_CUR);
    					while((character = fgetc(inventoryFile)) != '\n'){
    						if(isdigit((int)character)){
    							fseek(inventoryFile, -1, SEEK_CUR);
    							fprintf(inventoryFile, "%s\n", stock);
    							fseek(inventoryFile, 1, SEEK_CUR);
    							//send success message
    							sprintf(success, "%s %s %s", "Purchase", inventBrandName, "Successfully !");
    							write(connfd, success, strlen(success) + 1);
    							confirm = 1;
    							break;
    						}
    					}
    				break;
    				}
    			}

    			//send error message
    			if(confirm == 0){
    				sprintf(error, "%s %s %s", "Purchase", inventBrandName, "failed !");
    				write(connfd, error, strlen(error));
    			}

    			//check inventory figures to replenish if <= 3
    			fseek(inventoryFile, 0, SEEK_SET);
    			while(1){
    				if((fgets(invent, 100, inventoryFile)) == NULL) break;

    				//printf("invent %s", invent);
    				inventMachineID = strtok(invent, ",");
    				inventBrandName = strtok(NULL, ",");
    				if(inventBrandName != NULL){
    					inventStock = strtok(NULL, ",");
    					stockNumber = atoi(inventStock);
    					if(stockNumber <= 3){
    						stockNumber = 10;
    						sprintf(stock, "%d", stockNumber);
    						fseek(inventoryFile, -4, SEEK_CUR);
    						while((character = fgetc(inventoryFile)) != '\n'){
    							if(isdigit((int)character)){
    								fseek(inventoryFile, -1, SEEK_CUR);
    								fprintf(inventoryFile, "%s\n", stock);
    								printf("Delivering inventory !\n");

    								//send message to vending machine to replenish the commodity
    								//sprintf(clientMessage, "%s %s %d", machineID, brandName, stockNumber);
    								//send(connfd, clientMessage, strlen(clientMessage), 0);
    								//write(connfd, clientMessage, strlen(clientMessage));
    								sleep(5);
    								printf("Delivery completely!\n");
    								break;
    							}
    						}
    						msgFlag = 1;
    						break;
    					} 
    				}
    			}
    			// printf("msgFlag %d\n", msgFlag);
    			// if(msgFlag == 0){
    			// 	send(connfd, "ok", 2, 0);
    			// }

    			fclose(inventoryFile);

    			if(recvm == 0){
    				printf("Client disconnected !\n");
    			}
    			else if (recvm ==-1){
    				printf("recv failed !\n");
    			}



    			//access and write data to shared memory
    			// if((shmidWrite = shmget(key, SHIMSIZE, IPC_CREAT | 0666 )) < 0){
    			// 	printf("write shmget failed !\n");
    			// }

    			// if((head = shmat(shmidWrite, NULL, 0)) == (int *)-1){
    			// 	printf("write shmat failed !\n");
    			// }

    			// shmMessage = (char *)malloc(100*sizeof(char));
    			// strcpy(shmMessage, machineID);
    			// strcat(shmMessage, " ");
    			// strcat(shmMessage, brandName);
    			// strcat(shmMessage, " ");
    			// strcat(shmMessage, stock);
    			// strcat(shmMessage, "\n");
    			// memcpy(head, shmMessage, strlen(shmMessage));
    		}	
    		close(connfd);
    	}
    }

    //check inventory figures from shared memory
    // while(1){
    // 	if((shmidRead = shmget(key, SHIMSIZE, IPC_CREAT | 0666 )) < 0){
    // 		printf("read shmget failed !\n");
    // 	}

    // 	if((begin = shmat(shmidRead, NULL, 0)) == (int *)-1){
    // 		printf("read shmat failed !\n");
    // 	}


    // }


	return 0;
}