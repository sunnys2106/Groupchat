#include "util.h"
#include <pthread.h>
#include <stdlib.h>

#define PORT 2000
#define BACKLOG 10

int acceptedSockets[BACKLOG];
int acceptedSocketCount = 0;
pthread_mutex_t socketListLock = PTHREAD_MUTEX_INITIALIZER;  

void handleIncomingData(void *arg){
    int socketFD = *(int *)arg;
    free(arg);  
    char buffer[1024];

    while(1){
        ssize_t amountReceived = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
        if(amountReceived > 0){
            buffer[amountReceived] = 0;
            printf("Message received from %s\n", buffer);

            pthread_mutex_lock(&socketListLock);
            for(int i=0;i<acceptedSocketCount;i++){
                if(acceptedSockets[i] != socketFD){
                    ssize_t amountSent = send(acceptedSockets[i], buffer, amountReceived, 0);
                    if(amountSent == -1){
                        perror("Failed to send data to client");
                    }
                }
            }
            pthread_mutex_unlock(&socketListLock);
        }else{
            if(amountReceived == 0){
                printf("Client disconnected.\n");
            }else{
                perror("recv failed");
            }
            break;
        }
    }

     //remove client socket from list
    pthread_mutex_lock(&socketListLock);
    for(int i = 0; i < acceptedSocketCount; i++){
        if(acceptedSockets[i] == socketFD){
            //move last to current
            acceptedSockets[i] = acceptedSockets[acceptedSocketCount - 1]; 
            acceptedSocketCount--;
            break;
        }
    }
    pthread_mutex_unlock(&socketListLock);

    close(socketFD);
    pthread_exit(NULL);
}

int main(){
     //create server socket
    int serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocketFD == -1){
        perror("Failed to create server socket");
        exit(EXIT_FAILURE);
    }

    //set socket options to allow reusing the port
    int opt = 1;
    if(setsockopt(serverSocketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
        perror("setsockopt failed");
        close(serverSocketFD);
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in *address = createIPv4Addr("", PORT);
    if(!address){
        fprintf(stderr, "Failed to create server address\n");
        close(serverSocketFD);
        exit(EXIT_FAILURE);
    }

    //bind socket to address
    if(bind(serverSocketFD, (struct sockaddr *)address, sizeof(*address)) == -1){
        perror("bind failed");
        free(address);
        close(serverSocketFD);
        exit(EXIT_FAILURE);
    }
    printf("Server bind successful.\n");
    free(address);


    //start listening for connections
    if(listen(serverSocketFD, BACKLOG) == -1){
        perror("listen failed");
        close(serverSocketFD);
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on port %d...\n", PORT);

    while(1){
        struct sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);
        int *clientSocketFD = malloc(sizeof(int));  
        if(!clientSocketFD){
            fprintf(stderr, "malloc failed for client socket FD\n");
            continue;
        }

        *clientSocketFD = accept(serverSocketFD, (struct sockaddr *)&clientAddress, &clientAddressSize);
        if(*clientSocketFD == -1){
            perror("accept failed");
            free(clientSocketFD);
            continue;
        }

        pthread_mutex_lock(&socketListLock);
        if(acceptedSocketCount < BACKLOG){
            acceptedSockets[acceptedSocketCount++] = *clientSocketFD;
            printf("Client connected.\n");

            pthread_t id;
            if(pthread_create(&id, NULL, (void *)handleIncomingData, clientSocketFD) != 0){
                perror("pthread_create failed");
                free(clientSocketFD);
                close(*clientSocketFD);
            }else{
                pthread_detach(id);
            }
        }else{
            fprintf(stderr, "Server full, rejecting client.\n");
            close(*clientSocketFD);
            free(clientSocketFD);
        }
        pthread_mutex_unlock(&socketListLock);
    }

    shutdown(serverSocketFD, SHUT_RDWR);
    close(serverSocketFD);

    return 0;
}