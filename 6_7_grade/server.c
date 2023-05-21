#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#define DEFAULT_PORT 8080
#define DEFAULT_IP "127.0.0.1"
#define TOTAL_HEIRS 8

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, monitor_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
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

    // Accept monitor
    printf("Waiting for monitor\n");
    if ((monitor_socket = accept(server_fd, (struct sockaddr *) &address, &addrlen)) < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Monitor connected\n");

    // Accept heirs and distribute inheritance
    for (int i = 0; i < TOTAL_HEIRS; i++) {
        printf("Waiting for heir %d\n", i + 1);
        if ((new_socket = accept(server_fd, (struct sockaddr *) &address, &addrlen)) < 0) {
            perror("accept failed");
            continue;
        }

        int heir_part;
        if (read(new_socket, &heir_part, sizeof(heir_part)) <= 0) {
            perror("read heir_part failed");
            continue;
        }
        heir_part = ntohl(heir_part);
        expected_inheritance[i] = heir_part;

        if ((i + 1) == liar_heir) {
            heir_part -= 10000;
        }
        received_inheritance[i] = heir_part;
        total_received += heir_part;

        printf("Received inheritance from heir %d\n", i + 1);
        int heir_index = htonl(i + 1);

        if (send(monitor_socket, &heir_index, sizeof(heir_index), 0) == -1) {
            perror("send heir_index failed");
            continue;
        }

        int heir_part_network = htonl(heir_part);
        if (send(monitor_socket, &heir_part_network, sizeof(heir_part_network), 0) == -1) {
            perror("send heir_part failed");
            continue;
        }

        // Send expected and received inheritance parts
        int expected_inheritance_network = htonl(expected_inheritance[i]);
        if (send(monitor_socket, &expected_inheritance_network, sizeof(expected_inheritance_network), 0) == -1) {
            perror("send expected_inheritance failed");
            continue;
        }
        int received_inheritance_network = htonl(received_inheritance[i]);
        if (send(monitor_socket, &received_inheritance_network, sizeof(received_inheritance_network), 0) == -1) {
            perror("send received_inheritance failed");
            continue;
        }
    }
    return 0;
}
