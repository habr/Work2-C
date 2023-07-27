#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ev.h>

#define BUFFER_SIZE 1024

struct echo_watcher {
    struct ev_io io;
    int fd;
};

static void echo_read_cb(EV_P_ struct ev_io *watcher, int revents);
static void accept_cb(EV_P_ struct ev_io *watcher, int revents);
static void *echo_thread(void *arg);

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    struct ev_loop *loop = EV_DEFAULT;

    int sd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        perror("bind");
        return 1;
    }

    if (listen(sd, SOMAXCONN) != 0) {
        perror("listen");
        return 1;
    }

    struct ev_io w_accept;
    ev_io_init(&w_accept, accept_cb, sd, EV_READ);
    ev_io_start(loop, &w_accept);

    ev_run(loop, 0);

    return 0;
}

static void accept_cb(EV_P_ struct ev_io *watcher, int revents) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_sd = accept(watcher->fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_sd < 0) {
        perror("accept error");
        return;
    }

    struct echo_watcher *client_watcher = (struct echo_watcher*) malloc(sizeof(struct echo_watcher));
    client_watcher->fd = client_sd;

    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, &echo_thread, client_watcher) != 0) {
        perror("pthread_create");
        return;
    }

    pthread_detach(thread_id);
}

static void *echo_thread(void *arg) {
    struct echo_watcher *w = (struct echo_watcher *) arg;

    char buffer[BUFFER_SIZE];
    ssize_t nread = read(w->fd, buffer, BUFFER_SIZE);

    if (nread < 0) {
        perror("read error");
        return NULL;
    } else if (nread == 0) {
        close(w->fd);
        free(w);
        return NULL;
    } else {
        // Reverse the buffer
        for (int i = 0; i < nread / 2; i++) {
            char temp = buffer[i];
            buffer[i] = buffer[nread - i - 1];
            buffer[nread - i - 1] = temp;
        }

        // Send the reversed data back to client
        write(w->fd, buffer, nread);
    }

    return NULL;
}
