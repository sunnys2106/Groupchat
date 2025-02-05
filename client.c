#include "util.h"
#include <stdlib.h>
#include <pthread.h>

void* listenForMessages(void *arg){
    int socketFD = *(int *)arg;
    char buffer[1024];

    while(1){
        ssize_t amountReceived = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
        if(amountReceived > 0){
            buffer[amountReceived] = 0;
            printf("%s\n", buffer);
        }else{
            if(amountReceived == 0){
                printf("Server disconnected.\n");
            }else{
                perror("recv failed");
            }
            break;
        }
    }

    return NULL;
}


int main(){
    //create client socket
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFD == -1){
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    //create server address
    struct sockaddr_in *address = createIPv4Addr("127.0.0.1", 2000);
    if(!address){
        fprintf(stderr, "Failed to create server address\n");
        close(socketFD);
        exit(EXIT_FAILURE);
    }

    //connect to server
    int result = connect(socketFD, (struct sockaddr *)address, sizeof(*address));
    free(address);  
    if(result == -1){
        perror("Connection to server failed");
        close(socketFD);
        exit(EXIT_FAILURE);
    }
    printf("Client connection successful\n");

    //create new thread to listen for messages from server
    pthread_t id;
    if(pthread_create(&id, NULL, listenForMessages, &socketFD) != 0){
        perror("pthread_create failed");
        close(socketFD);
        exit(EXIT_FAILURE);
    }
    pthread_detach(id);  

    char *name = NULL;
    size_t nameSize = 0;
    printf("Enter your name:\n");
    ssize_t nameCount = getline(&name, &nameSize, stdin);
    if(nameCount == -1){
        perror("Failed to read name");
        free(name);
        close(socketFD);
        exit(EXIT_FAILURE);
    }
    name[nameCount - 1] = '\0';  

    char *line = NULL;
    size_t lineSize = 0;
    printf("Type your messages (or 'exit' to terminate):\n");

    char buffer[1024];
    while(1){   
        ssize_t charCount = getline(&line, &lineSize, stdin);
        if(charCount == -1){
            perror("Failed to read input");
            break;
        }
        line[charCount - 1] = '\0';  

        if(strcmp(line, "exit") == 0){
            break;
        }

        if(snprintf(buffer, sizeof(buffer), "%s: %s", name, line) >= sizeof(buffer)){
            fprintf(stderr, "Message too long, not sent.\n");
            continue;
        }

        //send data to server
        ssize_t amountSent = send(socketFD, buffer, strlen(buffer), 0);
        if(amountSent == -1){
            perror("Failed to send data");
            break;
        }
    }

    free(name);
    free(line);
    close(socketFD);
    printf("Client shutting down.\n");

    return 0;
}