#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <math.h>
#include "server.h"

#define SERVERPORT "5550"
#define MYPORT SERVERPORT    // the port users will be connecting to
#define MAXBUFLEN 100


void send_username(client_info *connection) {
    message msg;
    msg.type = USERNAME;
    strncpy(msg.sender , connection->username, (int)sizeof(connection->username));
    strncpy(msg.message , connection->username, (int)sizeof(connection->username));
    if(send(connection->socket, (void*)&msg, sizeof(message), 0)<0) {
        printf("Error sending username to server.\n");
        exit(1);
    }
}
void stop_client(client_info *connection){
    close(connection->socket);
    printf("Closing connection\n");
    exit(0);
}

void connect_to_server(client_info *connection, char *address, char *port) {
    
    while(1){
        
        //CREATE THE SOCKET
        if ((connection->socket = socket(AF_INET, SOCK_STREAM , IPPROTO_TCP)) < 0)
        {
            perror("Could not create socket\n");}
        
        connection->address.sin_addr.s_addr = inet_addr(address);
        connection->address.sin_family = AF_INET;
        connection->address.sin_port = htons(atoi(port));
        
        //CONNECT TO THE SOCKET!
        if (connect(connection->socket, (struct sockaddr*)connection->address, sizeof(connection->address)) == -1){
            printf("Error connecting to socket\n");
            exit(1);
        }
            
        //SEND USERNAME TO THE SERVER
        send_username(connection);
        
        message msg;
        ssize_t recvfrom = recv(connection->socket, &msg, sizeof(msg), 0);
        if (recvfrom<0){
            printf("Error: recieve message from server failed.\n");
            exit(1);
        }
        else if (recvfrom == 1) {
            printf("Username already picked. Please try another.\n");
            continue;
        }
        break; //everything is good so we're gonna do otehr stuff now
    }
    
    printf("Successfully connected to server with credentials %s\n", connection->username);
}

void send_msg(client_info *connection, int msgtype, char *message_in) {
    
    message msg;
    msg.type = msgtype;
    strncpy(msg.message , message_in, (int)sizeof(message_in));
    strncpy(msg.sender , connection->username, (int)sizeof(connection->username));
    
    if(send(connection->socket, (void*)&msg, sizeof(msg), 0)<0) {
        printf("Error sending message to server.\n");
        exit(1);
    }
}

void parser(client_info *connection, char* msg, char* _sendee){
    
    int msgtype;
    message message;
    message.sendee = "";
    if(strcmp(msg, "quit") == 0)
        stop_client(connection);
    else if(strcmp(msg,"send") == 0){
        msgtype = PM;
        message.sendee = _sendee;
    }
    else if(strcmp(msg,"broadcast") == 0)
        msgtype = BROADCAST;
    else if(strcmp(msg,"list") == 0)
        msgtype = LIST;
    else{
        printf("Invalid command\n");
        return;
    }
    
    message.type = msgtype;
    message.message = msg;
    
    //sent thy shit
    if(send(connection->socket, (void*)&message, sizeof(message), 0)<0) {
        printf("Error sending message type %d to server.\n", msgtype);
        exit(1);
    }
    
}

void server_msg_handler(client_info *connection) {
    message msg;
    ssize_t recievedstuff = recv(connection->socket, &msg, sizeof(message),0);
    if(recievedstuff < 0)
        printf("recieving from server failed.\n");
    else if (recievedstuff == 0){
        printf("server disconnected\n");
        close(connection->socket);
        exit(0);
    }
    
    switch(msg.type){
        case LIST:
            printf("%s\n", connection->username);
        case PM:
            if(strcmp(msg.sendee, connection->username) == 0) //ONLY PRINT IF U R THE SENDEE this is kinda bad security-wise but idc bc im not in computer security
                printf("%s: %s\n",msg.sender,msg.message);
        case BROADCAST:
            printf("%s: %s\n",msg.sender,msg.message);
        default:
            printf("Error: not valid message type!\n");
    }
    
}

int main(int argc, char **argv) {
    
    client_info connection;
    fd_set file_descrip;
    
    if(argc != 4) {
        printf("Error: not enough arguments.\n");
        return 1;
    }
    connection.username = argv[3];
    
    connect_to_server(&connection, argv[1], argv[2]);
    
    while(1) {
        printf("Type command to terminal\n");
        fflush(stdin);
        
        //checking stuff
        FD_ZERO(&file_descrip);
        FD_SET(STDIN_FILENO, &file_descrip);
        FD_SET(connection.socket, &file_descrip);
        if(select(2, &file_descrip, NULL, NULL, NULL) < 0) {
            printf("Error: failed to check file descriptions.\n");
        }
        
        if(FD_ISSET(STDIN_FILENO, &file_descrip)) {
            char *message;
            fgets(message,MAX_MESSAGE_LEN,stdin);
            char* command = strtok ( message," " );
            char* sendee = strtok ( message," " );
            parser(&connection, command, sendee);
            
        }
        if(FD_ISSET(connection.socket, &file_descrip)) {
            server_msg_handler(&connection);
        }
            
    }
    
    close(connection.socket);
    return 0;
}
