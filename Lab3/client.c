//
//  main.c
//  lab3
//
//  Created by Marinette Chen on 2019-03-03.
//  Copyright ?? 2019 Marinette Chen. All rights reserved.
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
#include <math.h>
#define SERVERPORT "5550"
#define MYPORT SERVERPORT    // the port users will be connecting to
#define MAXBUFLEN 100

double time_elapsed(struct timeval before, struct timeval after) {
        return (after.tv_sec-before.tv_sec)*pow(10,-6)+(after.tv_usec - before.tv_usec);
}
// get sockaddr, IPv4 or IPv6:

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

int talker(int argc, char *argv[]) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    char *msg = argv[3];
    if (argc != 4) {
        fprintf(stderr, "usage: talker hostname message\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    int yes = 1;
    

    // loop through all the results and make a socket
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        // lose the pesky "Address already in use" error message
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
            perror("setsockopt(SO_REUSEADDR) failed");

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }
    struct timeval timeBefore, timeAfter;
    gettimeofday(&timeBefore, NULL);
    if ((numbytes = sendto(sockfd, msg, strlen(msg), 0,
            p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

    printf("talker: sent %d bits to %s\n", numbytes*8, argv[1]);

printf("listener: waiting to recvfrom...\n");

    addr_len = sizeof their_addr;
  
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
            (struct sockaddr *) &their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    gettimeofday(&timeAfter, NULL);

    double timeElapsedSec = time_elapsed(timeBefore, timeAfter);
    printf("listener: got packet from %s in %lf seconds\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s), timeElapsedSec);
    printf("RTT is (microseconds): %lf\n", timeElapsedSec);
    printf("listener: packet is %d bits long\n", numbytes*8);
    buf[numbytes] = '\0';
    printf("listener: packet contains \"%s\"\n", buf);
    freeaddrinfo(servinfo);

    close(sockfd);
    
    return 0;
}

int main(int argc, char *argv[]) {

    talker(argc, argv);

}
