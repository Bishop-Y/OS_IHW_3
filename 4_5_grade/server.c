#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#define DEFAULT_PORT 8080
#define DEFAULT_IP "127.0.0.1"
#define TOTAL_HEIRS 8

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int total_expected = 0;
    int total_received = 0; // total received by the heirs
    int liar_heir = 0; // the heir on which the server will lie
    int expected_inheritance[TOTAL_HEIRS]; // Expected inheritance for each heir
    int received_inheritance[TOTAL_HEIRS]; // Actual inheritance received by each heir
    int port = DEFAULT_PORT;
    char ip[15] = DEFAULT_IP;
    if (argc == 2) {
        liar_heir = atoi(argv[1]);
        if (liar_heir < 0 || liar_heir > TOTAL_HEIRS) {
            printf("Invalid liar heir number. It should be between 0 and %d.\n", TOTAL_HEIRS);
            return -1;
        }
    }
    if (argc == 4) {
        liar_heir = atoi(argv[1]);
        if (liar_heir < 0 || liar_heir > TOTAL_HEIRS) {
            printf("Invalid liar heir number. It should be between 0 and %d.\n", TOTAL_HEIRS);
            return -1;
        }
        strcpy(ip, argv[2]);
        port = atoi(argv[3]);
    }

    // Creating socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Binding the socket to the port
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < TOTAL_HEIRS; i++) {
        printf("Waiting for heir %d\n", i + 1);
        if ((new_socket = accept(server_fd, (struct sockaddr *) &address, &addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
        // Receive inheritance part from the heir
        int heir_part;
        read(new_socket, &heir_part, sizeof(heir_part));
        heir_part = ntohl(heir_part);
        expected_inheritance[i] = heir_part;
        total_expected += heir_part;
        // Lie on the specified heir
        if ((i + 1) == liar_heir) {
            heir_part -= 10000; // reduce the heir part by 10000
        }
        received_inheritance[i] = heir_part;
        total_received += heir_part;
        printf("Received inheritance from heir %d\n", i + 1);
    }

    if (total_received != total_expected) {
        printf("\n\nInheritance distribution error! Received %d, expected %d!\n\n", total_received, total_expected);
    } else {
        printf("\n\nInheritance distributed correctly.\n\n");
    }

    // Output expected and received inheritance for each heir
    for (int i = 0; i < TOTAL_HEIRS; i++) {
        if (expected_inheritance[i] != received_inheritance[i]) {
            printf("Heir %d: expected %d, received %d. Lawyer lied on this heir!\n\n", i + 1, expected_inheritance[i],
                   received_inheritance[i]);
        } else {
            printf("Heir %d: expected %d, received %d.\n\n", i + 1, expected_inheritance[i], received_inheritance[i]);
        }
    }

    return 0;
}
