//
//  server.h
//  Chatroom
//
//  Created by Marinette Chen on 2019-03-17.
//  Copyright © 2019 Marinette Chen. All rights reserved.
//

#ifndef server_h
#define server_h

#include <stdio.h>
#define MAX_MESSAGE_LEN 1000

typedef enum{
    DISCONNECT,
    BROADCAST,
    PM,
    LIST,
    USERNAME
} types;

typedef struct client_info {
    int socket;
    struct sockaddr_in address;
    char *username;
} client_info;

typedef struct message {
    
    char * message;
    int type;
    char * sender;
    char * sendee;
    
} message;
    

#endif /* server_h */
