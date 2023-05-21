#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#define DEFAULT_PORT 8080
#define MONITOR_PORT 8081
#define DEFAULT_IP "127.0.0.1"
#define TOTAL_HEIRS 8
#define MAX_MONITORS 16

int monitor_sockets[MAX_MONITORS] = {0};  // array of monitor socket descriptors
int monitor_status[MAX_MONITORS] = {0};  // array of monitor statuses (1 for active, 0 for inactive)

int main(int argc, char const *argv[]) {
    int new_socket, monitor_socket;
    int total_received = 0;
    int liar_heir = 0;
    int expected_inheritance[TOTAL_HEIRS];
    int received_inheritance[TOTAL_HEIRS];
    int client_port = DEFAULT_PORT;
    int monitor_port = MONITOR_PORT;
    char ip[15] = DEFAULT_IP;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    if (argc == 2) {
        liar_heir = atoi(argv[1]);
        if (liar_heir < 0 || liar_heir > TOTAL_HEIRS) {
            printf("Invalid liar heir number. It should be between 0 and %d.\n", TOTAL_HEIRS);
            return -1;
        }
    }

    if (argc == 5) {
        liar_heir = atoi(argv[1]);
        if (liar_heir < 0 || liar_heir > TOTAL_HEIRS) {
            printf("Invalid liar heir number. It should be between 0 and %d.\n", TOTAL_HEIRS);
            return -1;
        }
        strcpy(ip, argv[2]);
        client_port = atoi(argv[3]);
        monitor_port = atoi(argv[4]);
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(client_port);

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    int monitor_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (monitor_server_fd == 0) {
        perror("monitor socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_port = htons(monitor_port);
    if (bind(monitor_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("monitor bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(monitor_server_fd, 3) < 0) {
        perror("monitor listen failed");
        exit(EXIT_FAILURE);
    }

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

        // Accept new monitors and update current ones
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(monitor_server_fd, &read_fds);
        for (int j = 0; j < MAX_MONITORS; j++) {
            if (monitor_status[j] == 1) {
                FD_SET(monitor_sockets[j], &read_fds);
            }
        }
        struct timeval tv = {0};
        if (select(FD_SETSIZE, &read_fds, NULL, NULL, &tv) > 0) {
            for (int j = 0; j < MAX_MONITORS; j++) {
                if (FD_ISSET(monitor_sockets[j], &read_fds)) {
                    int buf;
                    if (recv(monitor_sockets[j], &buf, sizeof(buf), MSG_PEEK | MSG_DONTWAIT) == 0) {
                        printf("Monitor %d disconnected\n", j + 1);
                        monitor_status[j] = 0;
                        close(monitor_sockets[j]);
                    }
                }
            }
            if (FD_ISSET(monitor_server_fd, &read_fds)) {
                if ((monitor_socket = accept(monitor_server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                    perror("monitor accept failed");
                } else {
                    for (int j = 0; j < MAX_MONITORS; j++) {
                        if (monitor_status[j] == 0) {
                            monitor_sockets[j] = monitor_socket;
                            monitor_status[j] = 1;
                            printf("Monitor %d connected\n", j + 1);
                            break;
                        }
                    }
                }
            }
        }

        int heir_index = htonl(i + 1);
        int heir_part_network = htonl(heir_part);
        int expected_part_network = htonl(expected_inheritance[i]);
        int received_part_network = htonl(received_inheritance[i]);
        for (int j = 0; j < MAX_MONITORS; j++) {
            if (monitor_status[j] == 1) {
                if (send(monitor_sockets[j], &heir_index, sizeof(heir_index), 0) < 0 ||
                    send(monitor_sockets[j], &heir_part_network, sizeof(heir_part_network), 0) < 0 ||
                    send(monitor_sockets[j], &expected_part_network, sizeof(expected_part_network), 0) < 0 ||
                    send(monitor_sockets[j], &received_part_network, sizeof(received_part_network), 0) < 0) {
                    perror("send failed");
                    monitor_status[j] = 0;
                    close(monitor_sockets[j]);
                }
            }
        }

        close(new_socket);
    }
    return 0;
}
