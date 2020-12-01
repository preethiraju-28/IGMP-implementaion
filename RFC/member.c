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

int sockfd;
struct sockaddr_in cli_addr, serv_addr;
struct hostent *h;
unsigned char ttl = 1;
char buff[MAX];
int cli_len, serv_len;

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
        printf("Unknown host\n\n");
        exit(0);
    }

    serv_addr.sin_family = h -> h_addrtype;
    memcpy((char*)&serv_addr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    serv_addr.sin_port = htons(PORT);

    if(!IN_MULTICAST(ntohl(serv_addr.sin_addr.s_addr)))
    {
        printf("Given addresss is not multicast\n\n");
        exit(0);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        printf("Unable to open socket\n\n");
        exit(0);
    }

    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    cli_addr.sin_port = htons(0);
    if(bind(sockfd, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0)
    {
        printf("Unable to bind with socket\n\n");
        exit(0);
    }

    
    if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
    {
        printf("Cannot set ttl\n");
        exit(0);
    }
    printf("Enter 'quit' to exit group\n");

    for(;;)
    {
        serv_len = sizeof(serv_addr);
        printf("Message to group : ");
        fgets(buff, sizeof(buff), stdin);
        sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        if(strncmp("quit", buff, 4) == 0)
        {
            printf("exiting the group\n\n");
            exit(0);
        }
    }

}