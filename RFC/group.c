#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define PORT 1500
#define MAX 1024

int sockfd, rc;
char buff[MAX];
char reply;
int cli_len;
struct sockaddr_in cli_addr, serv_addr;
struct ip_mreq mreq;
struct in_addr mcast_addr;
struct hostent *h;

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("Format is %s <IPv4 address>\n\n", argv[0]);
        exit(0);
    }

    h = gethostbyname(argv[1]);
    if(h == NULL)
    {
        printf("Unknown group %s\n\n", argv[1]);
        exit(0);
    }

    memcpy(&mcast_addr, h -> h_addr_list[0], h-> h_length);

    if(!IN_MULTICAST(ntohl(mcast_addr.s_addr)))
    {
        printf("Given address is not multicast\n\n");
        exit(0);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        printf("Unable to create socket\n\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    serv_addr.sin_port = htons (PORT);

    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Unable to bind with port\n\n");
        exit(0);
    }

    mreq.imr_multiaddr.s_addr = mcast_addr.s_addr;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    rc = setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
    if(rc < 0)
    {
        printf("Cannot join multicast group\n\n");
        exit(0);
    }

    for(;;)
    {
        cli_len = sizeof(cli_addr);
        bzero(buff, MAX);
        recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)&cli_addr, &cli_len);
        printf("\nMessage from %p : %s\n", (void*)&mreq, buff);
        if(strncmp("quit", buff, 4) == 0)
        {
            
            if((setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&cli_addr, sizeof(cli_addr))) > 0)
            {
                printf("Unable to exit group\n\n");
            }
            else
            {
                printf("%p exited\n\n",(void*)&mreq);
            }
            
        }
        
    }
}
