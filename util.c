#include "util.h"

struct sockaddr_in* createIPv4Addr(char *ip, int port){
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    if(!address){
        perror("malloc failed for sockaddr_in");
        return NULL; 
    }

    memset(address, 0, sizeof(struct sockaddr_in));  
    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    if(strlen(ip) == 0){
        address->sin_addr.s_addr = INADDR_ANY;
    }else{
        if(inet_pton(AF_INET, ip, &address->sin_addr.s_addr) != 1){
            fprintf(stderr, "Invalid IP address format: %s\n", ip);
            free(address);  
            return NULL;
        }
    }

    return address;
}



