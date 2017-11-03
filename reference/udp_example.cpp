#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define STDIN 0

int sendall(int server_fd, char *buf, int *len) {
  int total = 0;        // how many bytes we've sent
  int bytesleft = *len; // how many we have left to send
  int n;

  while (total < *len) {
    n = send(server_fd, buf + total, bytesleft, 0);
    fprintf(stdout, "Sendall: %s\n", buf + total);
    if (n == -1) {
      break;
    }
    total += n;
    bytesleft -= n;
  }

  *len = total; // return number actually sent here
  return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

int main(int argc, char *argv[]) {
  int sockfd;
  struct addrinfo hints, *dstinfo = NULL, *srcinfo = NULL, *p = NULL;
  int rv = -1, ret = -1, len = -1, numbytes = 0;
  struct timeval tv;
  char buffer[256] = {0};
  fd_set readfds;

  // don't care about writefds and exceptfds:
  //     select(STDIN+1, &readfds, NULL, NULL, &tv);

  if (argc != 4) {
    fprintf(stderr, "usage: %s dst-hostname dst-udpport src-udpport\n");
    ret = -1;
    goto LBL_RET;
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM; // UDP communication

  /*For destination address*/
  if ((rv = getaddrinfo(argv[1], argv[2], &hints, &dstinfo)) != 0) {
    fprintf(stderr, "getaddrinfo for dest address: %s\n", gai_strerror(rv));
    ret = 1;
    goto LBL_RET;
  }

  // loop through all the results and make a socket
  for (p = dstinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("socket");
      continue;
    }
    /*Taking first entry from getaddrinfo*/
    break;
  }

  /*Failed to get socket to all entries*/
  if (p == NULL) {
    fprintf(stderr, "%s: Failed to get socket\n");
    ret = 2;
    goto LBL_RET;
  }

  /*For source address*/
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM; // UDP communication
  hints.ai_flags = AI_PASSIVE;    // fill in my IP for me
  /*For source address*/
  if ((rv = getaddrinfo(NULL, argv[3], &hints, &srcinfo)) != 0) {
    fprintf(stderr, "getaddrinfo for src address: %s\n", gai_strerror(rv));
    ret = 3;
    goto LBL_RET;
  }

  /*Bind this datagram socket to source address info */
  if ((rv = bind(sockfd, srcinfo->ai_addr, srcinfo->ai_addrlen)) != 0) {
    fprintf(stderr, "bind: %s\n", gai_strerror(rv));
    ret = 3;
    goto LBL_RET;
  }

  /*Connect this datagram socket to destination address info */
  if ((rv = connect(sockfd, p->ai_addr, p->ai_addrlen)) != 0) {
    fprintf(stderr, "connect: %s\n", gai_strerror(rv));
    ret = 3;
    goto LBL_RET;
  }

  while (1) {
    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);
    FD_SET(sockfd, &readfds);

    /*Select timeout at 10s*/
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    select(sockfd + 1, &readfds, NULL, NULL, &tv);

    /*Obey your user, take his inputs*/
    if (FD_ISSET(STDIN, &readfds)) {
      memset(buffer, 0, sizeof(buffer));
      len = 0;
      printf("A key was pressed!\n");
      if (0 >= (len = read(STDIN, buffer, sizeof(buffer)))) {
        perror("read STDIN");
        ret = 4;
        goto LBL_RET;
      }

      fprintf(stdout, ">>%s\n", buffer);

      /*EOM\n implies user wants to exit*/
      if (!strcmp(buffer, "EOM\n")) {
        printf("Received EOM closing\n");
        break;
      }

      /*Sendall will use send to transfer to bound sockfd*/
      if (sendall(sockfd, buffer, &len) == -1) {
        perror("sendall");
        fprintf(stderr, "%s: We only sent %d bytes because of the error!\n",
                argv[0], len);
        ret = 5;
        goto LBL_RET;
      }
    }

    /*We've got something on our socket to read */
    if (FD_ISSET(sockfd, &readfds)) {
      memset(buffer, 0, sizeof(buffer));
      printf("Received something!\n");
      /*recv will use receive to connected sockfd */
      numbytes = recv(sockfd, buffer, sizeof(buffer), 0);
      if (0 == numbytes) {
        printf("Destination closed\n");
        break;
      } else if (-1 == numbytes) {
        /*Could be an ICMP error from remote end*/
        perror("recv");
        printf("Receive error check your firewall settings\n");
        ret = 5;
        goto LBL_RET;
      }
      fprintf(stdout, "<<Number of bytes %d Message: %s\n", numbytes, buffer);
    }

    /*Heartbeat*/
    printf(".\n");
  }

  ret = 0;
LBL_RET:

  if (dstinfo)
    freeaddrinfo(dstinfo);

  if (srcinfo)
    freeaddrinfo(srcinfo);

  close(sockfd);

  return ret;
}