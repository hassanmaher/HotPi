/* picolor - a unix socket client for changing RGB lights on raspberry-pi's gpio.
 *
 * Copyright (c) 2012 Karl Lattimer.
 * Author: Karl Lattimer <karl@qdh.org.uk>
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

void usage(char *error, char *binaryPath) {
    if (error) {
        printf ("Error: %s\n", error);
    }
    printf ("Usage: %s <socket> [options] <target colour>\n", binaryPath);
    printf ("Options:\n");
    printf (" -i        Change colour immediately\n");
    printf (" -d <ms>   Set the delay in milliseconds between steps\n");
    printf (" -l <s>    Set the time it should take to change to the new colour\n");
    printf (" -r        Print the current LED colour");
    printf("Example: %s /var/run/picolor \\#420088\n", binaryPath);
}

int main(int argc, char **argv) {
    int sock;
    struct sockaddr_un server;
    char buf[5];

    if (argc < 2) {
        usage("Too Few Arguments", argv[0]);
        return 1;
    }

    int r,g,b, delay = 1, duration;
    int instant = FALSE;

    int i = 2;
    for (i = 2; i < argc; i++) {
        if (strncmp(argv[i], "-i", 2) == 0) {
            instant = TRUE;
        } else if (strncmp(argv[i], "-d", 2) == 0) {
            delay = atoi(argv[i + 1]);
            if (delay > 255) delay = 255;
            i++;
        } else if (strncmp(argv[i], "-l", 2) == 0) {
            duration = atoi(argv[i + 1]) * 1000;
            delay = (int)roundf(duration / 255.0f);
            i++;
        } else if (strncmp(argv[i], "-r", 2) == 0) {
            buf[0] = '\x68';
            if (connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
                if (sock) close(sock);
                perror("picolor: Error connecting stream socket");
                return 3;
            }

            if (write(sock, buf, 1) < 0)
                perror("picolor: Error writing on stream socket");
                
            int rval = read(sock, buf, 5);
            int r,g,b;
            if (rval >= 3) {
                r = buf[0];
                g = buf[1];
                b = buf[2];
                printf("#%2x%2x%2x\n", r,g,b);
            }

            if (sock) close(sock);
            return 0;
        }
    }

    if (instant) {
	    buf[0] = '\x32';
    } else {
	    buf[0] = '\x42';
    }

    if (sscanf(argv[argc - 1], "#%2x%2x%2x", &r, &g, &b) == 3) {
        buf[1] = (char)r;
        buf[2] = (char)g;
        buf[3] = (char)b;
        buf[4] = (char)delay;
    } else {
        usage("Cannot parse colour: \"%s\"", argv[argc - 1]);
        return 1;
    }

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("picolor: Error opening stream socket");
        return 2;
    }

    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, argv[1]);

    if (connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
        close(sock);
        perror("picolor: Error connecting stream socket");
        return 3;
    }

    if (write(sock, buf, sizeof(buf)) < 0)
        perror("picolor: Error writing on stream socket");

    if (sock) close(sock);
    return 0;
}

