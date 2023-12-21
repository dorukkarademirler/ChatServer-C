/* server process */

/* include the necessary header files */
#include<ctype.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>

#include "protocol.h"
#include "libParseMessage.h"
#include "libMessageQueue.h"
#define MAX_CLIENTS 32

typedef struct CLIENT {
    int sockfd; 
    char user[MAX_USER_LEN+1];
    MessageQueue queue;
    char dodo[1024][MAX_MESSAGE_LEN];
} Client;
Client clientList[MAX_CLIENTS];

/**
 * send a single message to client 
 * sockfd: the socket to read from
 * toClient: a buffer containing a null terminated string with length at most 
 * 	     MAX_MESSAGE_LEN-1 characters. We send the message with \n replacing \0
 * 	     for a mximmum message sent of length MAX_MESSAGE_LEN (including \n).
 * return 1, if we have successfully sent the message
 * return 2, if we could not write the message
 */
int sendMessage(int sfd, char *toClient) {
    int numSend = 0;
    int len = strlen(toClient);
    if (len >= MAX_MESSAGE_LEN - 1) {
        return 2;
    }
    char buff[MAX_MESSAGE_LEN];
    sprintf(buff, "%s\n", toClient);  // add newline to the message buffer
    numSend = send(sfd, buff, strlen(buff), 0);
    if (numSend != strlen(buff)) {
        return 2;
    }
    return 1;
}

/**
 * read a single message from the client.
 * sockfd: the socket to read from
 * fromClient: a buffer of MAX_MESSAGE_LEN characters to place the resulting message
 *             the message is converted from newline to null terminated,
 *             that is the trailing \n is replaced with \0
 * return 1, if we have received a newline terminated string
 * return 2, if the socket closed (read returned 0 characters)
 * return 3, if we have read more bytes than allowed for a message by the protocol
 */
int recvMessage(int sfd, char *fromClient){
        int len = recv(sfd, fromClient, MAX_MESSAGE_LEN + 1, 0);
        if(len > MAX_MESSAGE_LEN)return(3);
        if(len == MAX_MESSAGE_LEN && fromClient[len] != '\n')return(3);
        if(len==0)return(2);
        char buffer[MAX_MESSAGE_LEN];
        for (int i = 0; i < len; i++) {
            if (fromClient[i] == '\n') {
                buffer[i] = '\0';
                strcpy(fromClient, buffer);
                return 1;
            }
            buffer[i] = fromClient[i];
        }
        return(1);
}

int main (int argc, char ** argv) {
    int sockfd; // Client //clientList[32];
        Client *clientList = malloc(sizeof(Client)*MAX_CLIENTS);
        for(int i = 0; i < MAX_CLIENTS; i++){
                initQueue(&clientList[i].queue);
                clientList[i].sockfd = -1;
                clientList[i].user[0] = '\0';
                clientList[i].dodo[0][0] = '\0';
        }

    if(argc!=2){
            fprintf(stderr, "Usage: %s portNumber\n", argv[0]);
            exit(1);
    }
    int port = atoi(argv[1]);

    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror ("socket call failed");
        exit (1);
    }

    struct sockaddr_in server;
    server.sin_family=AF_INET;          // IPv4 address
    server.sin_addr.s_addr=INADDR_ANY;  // Allow use of any interface
    server.sin_port = htons(port);      // specify port

    if (bind (sockfd, (struct sockaddr *) &server, sizeof(server)) == -1) {
        perror ("bind call failed");
        exit (1);
    }

    if (listen (sockfd, 5) == -1) {
        perror ("listen call failed");
        exit (1);
    }
    fd_set readset, writeset;

    for (;;) {
        int maxfd=sockfd;
        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_SET(sockfd ,&readset);
        int q = 0;

        while (q < MAX_CLIENTS) {
                if (clientList[q].sockfd != -1) {
                        FD_SET(clientList[q].sockfd, &readset);
                        FD_SET(clientList[q].sockfd, &writeset);
                        if (maxfd < clientList[q].sockfd){
                                maxfd = clientList[q].sockfd;
                        }
                }
        q++;
        }
         if(select(maxfd+1, &readset , &writeset , NULL , NULL) == -1){
                        perror("select call failed");
                        exit(1);
         }
        int index;

        if (FD_ISSET(sockfd, &readset)) {
            int newsockfd;
            if ((newsockfd = accept(sockfd, NULL, NULL)) == -1) {
                perror("accept call failed");
                continue;
            }
            if (newsockfd > maxfd) {
                maxfd = newsockfd;
            }

            // Add new client to clientList
            int j;
            for (j = 0; j < MAX_CLIENTS; j++) {
                if (clientList[j].sockfd == -1) {
                    clientList[j].sockfd = newsockfd;
                    break;
                }
            }
        }

        for (int index = 0; index < MAX_CLIENTS; index++) {
                if (clientList[index].sockfd != -1 && FD_ISSET(clientList[index].sockfd, &readset)) {
                        char fromClient[MAX_MESSAGE_LEN];
                        char toClient[MAX_MESSAGE_LEN];
                        // being retvalled
			// sendMessage(clientList[index].sockfd, "getting retvaled");

                        int retVal=recvMessage(clientList[index].sockfd, fromClient);
			char *f[1024] ;
                        if(retVal != 1){
                                close(clientList[index].sockfd);
                                clientList[index].sockfd = -1;
                                char message[MAX_MESSAGE_LEN];
                                clientList[index].user[0] = '\0';
                                while (dequeue(&clientList[index].queue, message)) {
                                        // Do nothing, just dequeue all messages to empty the queue
                                }
                                char bobo[1024][MAX_MESSAGE_LEN];
                                strcpy(*clientList[index].dodo, *bobo);
                         }
                         else {
                                char *part[4];
                                int numParts=parseMessage(fromClient, part);

				 if (numParts==0){
                                        strcpy(toClient,"ERROR");
                                 }
                                 else  if(strcmp(part[0], "list")==0){
					//sendMessage(clientList[index].sockfd, "list_test");
                                        sprintf(toClient, "users:");
                                        int limit = 0;
                                        for(Client *client = clientList; client < clientList + MAX_CLIENTS; client++){
                                                if(client->sockfd > 0 && client->user[0] != '\0'){
                                                        sprintf(toClient + strlen(toClient), "%s ", client->user);
                                                        limit++;
                                                        if(limit >= 10){
                                                                break;
                                                        }
                                                 }
                                         }
                                 }
                                else if(strcmp(part[0], "message")==0){
                                        char *fromUser=part[1];
                                        char *toUser=part[2];
                                        char *message=part[3];
                                        int senderIndex = -1, receiverIndex = -1;
					if(strcmp(clientList[index].user, fromUser) == 0){
                                         	senderIndex = index;
                                        }
                                        for(int z = 0;  z< MAX_CLIENTS; z++){
                                                if(clientList[index].sockfd > 0){
                                                        if(strcmp(clientList[z].user, toUser) == 0){
                                                                receiverIndex = z;
                                                        }
                                                 }
                                         }
                                        if(senderIndex == -1){
                                                sprintf(toClient, "invalidFromUser:%s", fromUser);
                                        }
                                        else if(receiverIndex == -1){
                                                sprintf(toClient, "invalidToUser:%s", toUser);
                                        }
                                        else{
                                                sprintf(toClient, "%s:%s:%s:%s", "message", fromUser, toUser, message);

                                        if(enqueue(&clientList[receiverIndex].queue, toClient)){
                                                        strcpy(toClient, "messageQueued");
                                        }
                                        else{
                                                strcpy(toClient, "messageNotQueued");
                                        }
                                  }
                               }

                                        else if(strcmp(part[0], "getMessage")==0){
                                                        if(dequeue(&clientList[index].queue, toClient));
                                                         else {
                                                                strcpy(toClient, "noMessage");
                                                        }
                                         }
                                        else if(strcmp(part[0], "quit")==0){
                                                        clientList[index].user[0] = '\0';
                                                        strcpy(toClient, "closing");
                                                        char bobo[1024][MAX_MESSAGE_LEN];
                                                        strcpy(clientList[index].dodo,bobo);
                                                        char message[MAX_MESSAGE_LEN];
                                                        while (dequeue(&clientList[index].queue, message)) {
                                                                // Do nothing, just dequeue all messages to empty the queue
                                                        }
                                                        close(clientList[index].sockfd);
                                                        clientList[index].sockfd = -1;
                                        }
                                        else if(strcmp(part[0], "register")==0){
                                                if(clientList[index].user[0] != '\0'){
                                                        strcpy(toClient, "ERROR: User already registered");
                                                } else if(strlen(part[1]) > MAX_USER_LEN){
                                                        strcpy(toClient, "ERROR: Username too long");
                                                } else {
                                                        int isUnique = 1;
                                                        for(int j=0; j<MAX_CLIENTS; j++){
                                                                if(j!=index && strcmp(clientList[j].user, part[1])==0){
                                                                        isUnique = 0;
                                                                        break;
                                                                }
                                                        }
                                                        if(isUnique){
                                                                strcpy(clientList[index].user, part[1]);
                                                                strcpy(toClient, "registered");
                                                        } else {
                                                                strcpy(toClient, "ERROR: Username already taken");
                                                        }
                                        }


                                        }
					sendMessage(clientList[index].sockfd, toClient);
                                        //strcpy(clientList[index].dodo[strlen(clientList[index].dodo)] , toClient);
                                 }
                        }
                }
                                if(FD_ISSET(clientList[index].sockfd, &writeset)){
                                                 for (int w = 0 ; w <  strlen(clientList[index].dodo) ; w++){
                                                        if (clientList[index].dodo[w][0] != '\0'){
                                                                sendMessage(clientList[index].sockfd, clientList[index].dodo[w]);
								  char bobo[1024][MAX_MESSAGE_LEN];
                                				strcpy(clientList[index].dodo[w], bobo[w]);
                                                        }
                                                }
						// empty it after sending
                                 }
                        }
        }





