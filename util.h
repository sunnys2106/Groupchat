#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>

struct sockaddr_in* createIPv4Addr(char *ip, int port);

#endif