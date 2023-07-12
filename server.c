/*
AUTHOR				SOFTWARE NAME				DATE
JOHAN BABLO TOMA		SERVER-CLIENT ARCHITECHTURE		2023-05-09

Description
How to make the c-file executable is through using the terminal, then using gcc-command and its flags -pthread and -o to create the executable of client/server.
it should be something like this in the correct directory with this command "gcc client.c -pthread -o client" or "gcc server.c -pthread -o server". Server side
program should be first executed then the client side. The server listens for connections from clients and when client connects it initiates the communication.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 8080
#define WINDOWSIZE 8
#define FRAMECOUNT 32

/*
 * SYN: 0
 * SYN-ACK: 1
 * ACK: 2
 * FIN: 3
 * FIN-ACK: 4
 * NACK:5
 * SLIDING WINDOW FRAME MESSAGE:6
 * */

typedef struct {
    int flag;
    int seq;
    int crc[16];
    char data[30];
}frame;

typedef struct {
    struct sockaddr_in sockaddr;
    int sock;
    int size_addr;
    int seq;
    int frame_sent_count;
    int windowsize;
}track_info;

//declarations
void CreateFrames(frame* frames, int flag, int seq);
int checkFrames(frame* frames, int flag, int seq);

void CreateFrames(frame* frames, int flag, int seq){
    frames->flag = flag;
    frames->seq = seq;
}

int checkFrames(frame* frames, int flag, int seq) {

    if(frames->flag == flag && frames->seq == seq) {
        return 1;
    }
    else {
        if(frames->seq != seq) {
            printf("Incorrect sequence number on FRAME %d.\n", seq + 1);
        }
        else {
            return 0;
        }
    }
}

int main() {

    int sockfd;
    int value;
    struct sockaddr_in server_addr, client_addr;

    if((sockfd = socket(AF_INET, SOCK_DGRAM,0 )) == 0) {
        perror("Socket could not be created\n");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed to associate socket, network or system-related issues exist.\n");
        exit(EXIT_FAILURE);
    }

    printf("---------------------------------------\nESTABLISHING CONNECTION\n---------------------------------------\n");
    printf("Server is awaiting connection\n");

    frame frame_to_send;
    frame frame_to_recv;
    int len = sizeof(client_addr);

    if(recvfrom(sockfd, &frame_to_recv, sizeof(frame), 0, (struct sockaddr*)&client_addr, &len) < 0) {
        perror("Could not execute recvfrom either invalid socket, message to long, timeout or network error.\n");
        exit(EXIT_FAILURE);
    }

    if(checkFrames(&frame_to_recv, 0, 0) == 1) {

        printf("Received SYN\n");
        CreateFrames(&frame_to_send, 1, 0);

        if(sendto(sockfd, &frame_to_send, sizeof(frame), 0, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0) {
            perror("Could not execute sendto either invalid socket, connection lost or dest address is invalid.\n");
            exit(EXIT_FAILURE);
        }

        printf("Sending SYN-ACK\n");

        if (recvfrom(sockfd, &frame_to_recv, sizeof(frame), 0, (struct sockaddr*)&client_addr, &len) < 0) {
            perror("Could not execute recvfrom either invalid socket, message to long, timeout or network error.\n");
            exit(EXIT_FAILURE);
        }
        else {
            if (checkFrames(&frame_to_recv, 2, 0) == 1) {
                printf("Received ACK\n");
                printf("The connection has now fully been established.\n");
            }
        }
    }
    else {
        printf("Corrupted/Error on Frame\n");
    }

    printf("---------------------------------------\nSLIDING WINDOW\n---------------------------------------\n");

    printf("---------------------------------------\nINITIALIZING CONNECTION TEARDOWN\n---------------------------------------\n");

    if (recvfrom(sockfd, &frame_to_recv, sizeof(frame), 0, (struct sockaddr*)&client_addr, &len) < 0) {
        perror("Could not execute recvfrom either invalid socket, message to long, timeout or network error.\n");
        exit(EXIT_FAILURE);
    }

    if (checkFrames(&frame_to_recv, 3, 0) == 1) {
        printf("Received FIN\n");
        CreateFrames(&frame_to_send, 4, 0);

        if(sendto(sockfd, &frame_to_send, sizeof(frame), 0, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0) {
            perror("Could not execute sendto either invalid socket, connection lost or dest address is invalid.\n");
            exit(EXIT_FAILURE);
        }

        printf("Sending FIN-ACK\n");

        if (recvfrom(sockfd, &frame_to_recv, sizeof(frame), 0, (struct sockaddr*)&client_addr, &len) < 0) {
            perror("Could not execute recvfrom either invalid socket, message to long, timeout or network error.\n");
            exit(EXIT_FAILURE);
        }
        else {
            if (checkFrames(&frame_to_recv, 2, 0) == 1) {
                printf("Received ACK\n");
                printf("Teardown complete\n");
            }
        }
    }
    else {
        printf("Corrupted/Error on Frame\n");
    }







    return 0;
}
