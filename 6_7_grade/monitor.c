#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 8080
#define DEFAULT_IP "127.0.0.1"
#define TOTAL_HEIRS 8

int main(int argc, char const *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    int port = DEFAULT_PORT;
    char ip[15] = DEFAULT_IP;

    if (argc >= 3) {
        strcpy(ip, argv[1]);
        port = atoi(argv[2]);
    }

    // Creating socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IP address from text to binary form
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < TOTAL_HEIRS; i++) {
        int heir_number, inheritance_part;

        if (recv(sock, &heir_number, sizeof(heir_number), 0) <= 0) {
            perror("recv heir_number failed");
            continue;
        }
        heir_number = ntohl(heir_number);

        if (recv(sock, &inheritance_part, sizeof(inheritance_part), 0) <= 0) {
            perror("recv inheritance_part failed");
            continue;
        }
        inheritance_part = ntohl(inheritance_part);

        int expected_inheritance, received_inheritance;

        if (recv(sock, &expected_inheritance, sizeof(expected_inheritance), 0) <= 0) {
            perror("recv expected_inheritance failed");
            continue;
        }
        expected_inheritance = ntohl(expected_inheritance);

        if (recv(sock, &received_inheritance, sizeof(received_inheritance), 0) <= 0) {
            perror("recv received_inheritance failed");
            continue;
        }
        received_inheritance = ntohl(received_inheritance);

        if (expected_inheritance != received_inheritance) {
            printf("Heir %d: expected %d, received %d. Lawyer lied on this heir!\n", heir_number, expected_inheritance,
                   received_inheritance);
        } else {
            printf("Heir %d: expected %d, received %d.\n", heir_number, expected_inheritance, received_inheritance);
        }
    }
    return 0;
}
