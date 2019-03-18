#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "server.h"
  // port we're listening on

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void init_connection_tracker(client_info *connections) {
    for (int i = 0; i < MAX_USERS;i++){
        connections[i].username = NULL;
        connections[i].socket = 0;
    }
}

void send_msg_client(int socket, message * msg) {
    
    if(send(socket, (void*)&msg, sizeof(msg), 0)<0) {
        printf("Error sending message to client.\n");
        exit(1);
    }
}

int main(int argc, char **argv)
{
    
    if(argc != 2) {
        printf("Error: Program requires port id only. Try again\n");
        exit(1);
    }
    client_info connections[MAX_USERS];
    init_connection_tracker(&connections);
    char* PORT = argv[1];
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    
    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    
    message * buf;    // buffer for client data
    int nbytes;
    
    char remoteIP[INET6_ADDRSTRLEN];
    
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, rv;
    
    struct addrinfo hints, *ai, *p;
    
    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);
    
    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        
        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }
        
        break;
    }
    
    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }
    
    freeaddrinfo(ai); // all done with this
    
    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }
    
    // add the listener to the master set
    FD_SET(listener, &master);
    
    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one
    
    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }
        
        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,(struct sockaddr *)&remoteaddr,&addrlen);
                    
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                               "socket %d\n",
                               inet_ntop(remoteaddr.ss_family,
                                         get_in_addr((struct sockaddr*)&remoteaddr),
                                         remoteIP, INET6_ADDRSTRLEN),
                               newfd);
                        
                        //add the socket info to the client tracker
                        for (int i = 0; i < MAX_USERS; i++) {
                            if(connections[i].socket == 0)
                                connections[i].socket = newfd;
                        }
                    }
                } else {
                    // handle data from a client
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        for (int k = 0; k < MAX_USERS; k++) {
                            //handling data from the user
                            if(connections[k].socket !=0 && FD_ISSET(connections[k].socket, &master)) {
                    
                                int socket_send = 0;
                                    switch(buf->type) {
                                        case DISCONNECT:
                                            for(int l = 0; l < MAX_USERS;l++) { //find client in connections
                                                if (connections[l].socket != 0 && !strcmp(connections[l].username, buf->sender)) { //check valid
                                                    close(connections[l].socket);
                                                    printf("Closed connection with client.");
                                                    connections[l].socket = 0; //free up spot
                                                }
                                            }
                                        case BROADCAST:
                                            for(int l = 0; l < MAX_USERS;l++){
                                                if(send(connections[l].socket, (void*)&buf, sizeof(buf), 0)<0) {
                                                    printf("Error sending broadcast to clients.\n");
                                                    exit(1);
                                                }
                                            }
                                        case PM:
                                            for(int l =0; l< MAX_USERS; l++) {
                                                if(connections[l].socket !=0 && connections[l].username == buf -> sendee ) {
                                                    socket_send = connections[l].socket;
                                                    send_msg_client(socket_send, buf);
                                                    break;
                                                }
                                            }
                                            printf("Error: user with username %s not found.\n",buf->sendee);
                                        case LIST:
                                            printf("Users connected to server:\n");
                                            for(int k =0; k< MAX_USERS; k++) {
                                                if(connections[i].socket!= 0)
                                                    printf("%s\n",connections[i].username);
                                            }
                                            break;
                                        case USERNAME:
                                            strcpy(connections[i].username, buf->message);
                                            break;
                                    }
                                
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}
