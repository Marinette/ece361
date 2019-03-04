//
//  main.c
//  lab3
//
//  Created by Marinette Chen on 2019-03-03.
//  Copyright © 2019 Marinette Chen. All rights reserved.
//
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

#define SERVERPORT "5550"
#define MYPORT SERVERPORT    // the port users will be connecting to
#define MAXBUFLEN 100

double time_elapsed(struct timeval before, struct timeval after, int micro) {
    if(micro)
        return after.tv_usec - before.tv_usec;
    else
        return after.tv_usec - before.tv_sec;
}
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int talker(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    
    if (argc != 3) {
        fprintf(stderr,"usage: talker hostname message\n");
        exit(1);
    }
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    
    if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }
    struct timeval timeBefore, timeAfter;
    gettimeofday(&timeBefore, NULL);
    if ((numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0,
                           p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }
    gettimeofday(&timeAfter, NULL);
    
    freeaddrinfo(servinfo);
    double timeElapsedSec = time_elapsed(timeBefore, timeAfter, 0);
    double timeElapsedMicro = time_elapsed(timeBefore, timeAfter, 1);
    printf("talker: sent %d bytes to %s in %lf seconds\n", numbytes, argv[1], timeElapsedSec);
    printf("tprop(time to server) is %lf microseconds\n", timeElapsedMicro);
    printf("transmission rate: %lf\n", numbytes*8/timeElapsedSec);
    close(sockfd);
    
    return 0;
}

int listener(int argc, char *argv[]) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }
        
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }
        
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }
    
    freeaddrinfo(servinfo);
    
    printf("listener: waiting to recvfrom...\n");
    
    addr_len = sizeof their_addr;
    struct timeval timeBefore, timeAfter;
    gettimeofday(&timeBefore, NULL);
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
                             (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    gettimeofday(&timeAfter, NULL);
    
    double timeElapsedSec = time_elapsed(timeBefore, timeAfter, 0);
    double timeElapsedMicro = time_elapsed(timeBefore, timeAfter, 1);
    printf("listener: got packet from %s in %lf seconds\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s), timeElapsedSec);
    printf("propagation time from server is: %lf microseconds\n",timeElapsedMicro);
    printf("listener: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    printf("listener: packet contains \"%s\"\n", buf);
    
    close(sockfd);
    
    return numbytes;
}


int main(int argc, char *argv[]) {
    
    talker(argc, argv);
    listener(argc, argv);
    
}
