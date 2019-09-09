#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


#define DEST_0 0xFF
#define DEST_1 0xFF
#define DEST_2 0xFF
#define DEST_3 0xFF
#define DEST_4 0xFF
#define DEST_5 0xFF



void enumerateif(void);
void get_name(void);
void send_payload(void);
void listen_payload(void);
char name[32];
int NAME_COUNT;
char ifname[IFNAMSIZ];
uint8_t MY_MAC[6] = {0};
//struct sockaddr_ll device;
char BUFFER[1024];


int main(){
    
    pthread_t receive_thread;

    enumerateif();
    get_name();
    pthread_create(&receive_thread, NULL, &listen_payload, NULL);
    send_payload();
    pthread_join(receive_thread, NULL);
    
    
    return 0;
}




void listen_payload(){
    int sockfd;
    int sockopt;
    struct ifreq ifopts;
    uint8_t payload[1024];
    int i;
    
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))==-1){
        perror("receive: socket");
        exit(-1);
    }
    strncpy(ifopts.ifr_name, ifname, IFNAMSIZ-1);
    ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
    ifopts.ifr_flags |= IFF_PROMISC;
    ioctl(sockfd, SIOCSIFFLAGS, &ifopts);
    
    
    // new try
    if (ioctl(sockfd, SIOCGIFHWADDR, &ifopts) < 0)
        perror("SIOCGIFHWADDR");

    for (i=0; i<6; i++)
        MY_MAC[i] = ((uint8_t *)&ifopts.ifr_hwaddr.sa_data)[i];
    
        
    // new try end
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifname, IFNAMSIZ-1) == -1)  {
        perror("SO_BINDTODEVICE");
        close(sockfd);
        exit(EXIT_FAILURE);
    }


    while(1){
        int total = 0;
        total = recvfrom(sockfd, payload, 1024, 0, NULL, NULL);
         
        if( payload[0]== DEST_0 && payload[1]== DEST_1 && 
            payload[2]== DEST_2 && payload[3]== DEST_3 &&
            payload[4]== DEST_4 && payload[5]== DEST_5 &&
            (payload[6] != MY_MAC[0] || payload[7] != MY_MAC[1] ||
            payload[8] != MY_MAC[2] || payload[9] != MY_MAC[3] ||
            payload[10] != MY_MAC[4] || payload[11] != MY_MAC[5]) 
            && payload[12] == 0x08 && payload[13] == 0x01){
            
            printf("<%02x:%02x:%02x:%02x:%02x:%02x> ", payload[6],
                    payload[7], payload[8], payload[9], payload[10],
                    payload[11]);
            for(i=14;i<total;i++)
                printf("%c", payload[i]);
            printf("\n");
            fflush(stdout);
                
            
        }
    }
    
}


void send_payload(){
    int sockfd;
    int i;
    struct ifreq if_idx;
    struct ifreq if_mac;
    int tx_len;
    char payload[1024];
    struct sockaddr_ll sock_address;
    
    
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP+1)))<0){
        perror("socket");
    }
    
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, ifname, IFNAMSIZ-1);
    if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
        perror("SIOCGIFINDEX");

    
    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, ifname, IFNAMSIZ-1);
    if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
        perror("SIOCGIFHWADDR");

    


    while(1){
        memset(payload, 0, 1024);
        tx_len=0;
        int ch;

        payload[tx_len++] = DEST_0;
        payload[tx_len++] = DEST_1;
        payload[tx_len++] = DEST_2;
        payload[tx_len++] = DEST_3;
        payload[tx_len++] = DEST_4;
        payload[tx_len++] = DEST_5;
        payload[tx_len++] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
        payload[tx_len++] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
        payload[tx_len++] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
        payload[tx_len++] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
        payload[tx_len++] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
        payload[tx_len++] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
        payload[tx_len++] = 0x08;
        payload[tx_len++] = 0x01;
         
        // name
        payload[tx_len++] = '[';
        for(i=0;i<NAME_COUNT;i++)
            payload[tx_len++] = name[i];
        payload[tx_len++] = ']';
        payload[tx_len++] = ':';
        payload[tx_len++] = ' ';

        
        // msg input
        while((ch = getchar())!='\n' && tx_len++)
            payload[tx_len++] = ch;
        
        if(tx_len==(18+NAME_COUNT))
            continue;

        sock_address.sll_ifindex = if_idx.ifr_ifindex;
        sock_address.sll_halen = ETH_ALEN;
        sock_address.sll_family = AF_PACKET;
        sock_address.sll_addr[0] = DEST_0;
        sock_address.sll_addr[1] = DEST_1;
        sock_address.sll_addr[2] = DEST_2;
        sock_address.sll_addr[3] = DEST_3;
        sock_address.sll_addr[4] = DEST_4;
        sock_address.sll_addr[5] = DEST_5;
        

        if (sendto(sockfd, payload, tx_len, 0, 
                    (struct sockaddr *)&sock_address,
                    sizeof(struct sockaddr_ll))<0)
            printf("Send failed\n");

        
    }

}






void enumerateif(void){
    struct ifaddrs *addrs, *head, *tmp;
    struct sockaddr_ll *sock_ll;
    struct sockaddr_in *sa;

    getifaddrs(&addrs);
    head = addrs;
    char macp[INET6_ADDRSTRLEN];
    char interface[IFNAMSIZ];
    char temp_address[16];
    char ip_1[3], ip_2[3], ip_3[3], ip_4[3];
    char ba_1[3], ba_2[3], ba_3[3], ba_4[3];
    long int netmask;
    int i, len;
    int flag = 0;
    int total = 0;



    printf("Enumerated network interfaces:\n");
    while(head){
        if (head->ifa_addr->sa_family == AF_INET){
            sa = (struct sockaddr_in *)head->ifa_addr;
            strcpy(temp_address,inet_ntoa(sa->sin_addr));
//            /*
            if (strcmp(temp_address,"127.0.0.1")==0){
                head = head->ifa_next;
                continue;
            }
//            */
            sscanf(temp_address, "%[^.].%[^.].%[^.].%[^.]", ip_1, ip_2, ip_3, ip_4); 
            sa = (struct sockaddr_in *)head->ifa_netmask;
            netmask = (sa->sin_addr).s_addr;
            sa = (struct sockaddr_in *)head->ifa_broadaddr;
            strcpy(temp_address,inet_ntoa(sa->sin_addr));
            sscanf(temp_address, "%[^.].%[^.].%[^.].%[^.]", ba_1, ba_2, ba_3, ba_4);
            memset(interface,0,32);
            strcpy(interface, head->ifa_name);
            tmp = addrs;
            while(tmp){
                if (tmp->ifa_addr->sa_family != AF_PACKET){
                    tmp = tmp->ifa_next;
                    break;
                }
                if (strcmp(head->ifa_name, tmp->ifa_name)==0){
                
                sock_ll = (struct sockaddr_ll *)tmp->ifa_addr;
                len = 0;
                for(i = 0; i < 6; i++)
                    len+=sprintf(macp+len,"%02X%s",sock_ll->sll_addr[i],i < 5 ? ":":"");
                }
                tmp = tmp->ifa_next;
            }
            printf("%d - %-10s ", sock_ll->sll_ifindex, interface);
            printf("%03d.%03d.%03d.%03d %#lx (%03d.%03d.%03d.%03d) %s\n", 
                atoi(ip_1), atoi(ip_2), atoi(ip_3), atoi(ip_4),
                ntohl(netmask), 
                atoi(ba_1), atoi(ba_2), atoi(ba_3), atoi(ba_4), 
                macp);
            if(!flag){
                flag = 1;
                strncpy(ifname, interface, IFNAMSIZ-1);
            }
            total += 1;
        }
        head = head->ifa_next;
    }

    freeifaddrs(addrs);
    if(total==0){
        printf("There is no available interface to use\n");
        printf("Please check your network\n");
        exit(-1);
    }
}

void get_name(void){
    int i=0;
    
    printf("Enter your name: ");
    fgets(name, 32, stdin);
    while(name[i] != '\n' && name[i] != '\0')
        i++;

    if (name[i] == '\n')
        name[i] = '\0';
    else{
        while(getchar()!='\n')
            continue;
    }
    NAME_COUNT = strlen(name);
    printf("Welcome, \'%s\'!\n", name);
    
}

