#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>

#define PORT 6789
#define MAXREQ (4096*10124)

char buffer[MAXREQ], body[MAXREQ], msg[MAXREQ];

void error(const char *msg) {
    perror(msg);
    exit(1);
}

//Socket headers for server and client
int sockfd, newsockfd; 
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;

void initalize_server() {

    //Creates the socket
    sockfd = socket(PF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        error("Something went wrong when opening socket");
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    //Binds the socket to a address
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Something went wrong when binding the socket to address");
    }
    
    listen(sockfd, 5);

    int n;
    while (1)
    {
        clilen = sizeof(cli_addr);
        //Accepts a new connection
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) 
        {
            error("Somethin went wrong when accepting a new socket");
        }
        
        bzero(buffer, sizeof(buffer));
        //Reads the HTTP request
        n = read(newsockfd, buffer, sizeof(buffer) - 1);
        
        if (n < 0)
        {
            error("Something went wrong when reading from the socket");
        }

        //Generates responses and send it back
        snprintf(body, sizeof(body),
            "<html>\n<body>\n"
            "<h1>Hello web browser</h1>\nYour request was\n"
            "<pre>%s</pre>\n"
            "</body>\n</html>\n", buffer);

        snprintf(msg, sizeof (msg),
            "HTTP/1.0 200 OK\n"
            "Content-Type: text/html\n"
            "Content-Length: %d\n\n%s", (int) strlen(body), body);

        //Sends the response
        n = write(newsockfd, msg, strlen(msg));

        if (n < 0) 
        {
            error("ERROR writing to socket");
        }

        //Closes the connection
        close(newsockfd); 
    }
    close(sockfd);   
}

int main(int argc, char* argv[]) {
    initalize_server();
    printf("Server running at port: 6789");
}