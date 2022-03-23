#include "../../include/core/mtwwwd.h"

char buffer[MAXREQ], body[MAXREQ], msg[MAXREQ];
char *www_path;
int port;
int num_of_threads;
int num_of_bufslots;
char *root;

BNDBUF *bb;

struct tdata {
    int thread_index;
    pthread_mutex_t *accept_mutex;
};

char buffer[MAXREQ], body[MAXREQ], msg[MAXREQ];
char *file_buf = 0;
char *root;
// socketheaders for server and client
struct sockaddr_in serv_addr, cli_addr;
int sockfd, clientfd;
int n;
socklen_t clilen;

void *handle_request(void *pArg);
char *read_html_file(char *path);



void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// Socket headers for server and client
int sockfd, newsockfd;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;

void initalize_server()
{

    // Creates the socket
    sockfd = socket(PF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        error("Something went wrong when opening socket");
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    printf("Server running at port: 6789");

    // Binds the socket to a address
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Something went wrong when binding the socket to address");
    }

    listen(sockfd, 5);

    pthread_mutex_t gLock;

    struct tdata tdarray[num_of_threads];

    for (int i = 0; i < num_of_threads; i++) {
        tdarray[i].accept_mutex = &gLock;
        tdarray[i].thread_index = i;
    }

    pthread_t tHandles[num_of_threads];
    pthread_mutex_init(&gLock, NULL);

    while (1)
    {
        clilen = sizeof(cli_addr);
        // Accepts a new connection
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

        if (newsockfd < 0)
        {
            error("Somethin went wrong when accepting a new socket");
        }
        bzero(buffer, sizeof(buffer));

        bb_add(bb, newsockfd);

        for (n = 0; n < num_of_threads; n++) {
            pthread_create(&(tHandles[n]),        // Returned thread handle
			    NULL,                  // Thread Attributes
			    handle_request,      // Thread function
			    (void*)&(tdarray[n])); // Data for process_requests
        }

        // Closes the connection
        close(newsockfd);
    }
    close(sockfd);
}

void *handle_request(void *pArg)
{
    struct tdata td = *((struct tdata *)pArg);
    int clientfd = bb_get(bb);

    // read the HTTP request
    n = read(clientfd, buffer, sizeof(buffer) - 1);

    if (n < 0)
    {
        error("Something went wrong when read from the socket");
    }

    char path[256];
    if (sscanf(buffer, "GET %255s", path) <= 0)
    {
        perror("Error with HTTP-method, needs to be GET");
        exit(1);
    }

    file_buf = read_html_file(path);

    if (!file_buf)
    {
        snprintf(msg, sizeof(msg),
                 "HTTP/1.0 404 OK\n"
                 "Content-Type: text/html\n"
                 "Content-Length: 0\n\n");
    }
    else
    {
        snprintf(msg, sizeof(msg),
                 "HTTP/1.0 200 OK\n"
                 "Content-Type: text/html\n"
                 "Content-Length: %ld\n\n%s",
                 strlen(file_buf), file_buf);
    }

    n = write(clientfd, msg, strlen(msg));
    if (n < 0)
    {
        error("Something went wrong when writing from the socket");
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <www-path>\n", argv[0]);
        return EXIT_FAILURE;
    }
    root = argv[1];
    num_of_threads = atoi(argv[2]);
    num_of_bufslots = atoi(argv[3]);
    bb = bb_init(num_of_bufslots);
    initalize_server();
}

char *read_html_file(char *path)
{
    char full_path[256];

    FILE *html_file;
    int len;
    char *html_file_buffer = NULL;
    if (snprintf(full_path, sizeof(full_path), "%s%s", root, path) >= sizeof(full_path))
    {
        printf("Error with path");
        return NULL;
    }
    printf("info: reading html_file %s\n", full_path);

    html_file = fopen(full_path, "r");

    if (!html_file)
    {
        printf("Error with opening file");
        return NULL;
    }
    else
    {
        // seek to the end of the html_file to get the length in bytes
        fseek(html_file, 0, SEEK_END);
        len = ftell(html_file);
        html_file_buffer = malloc(len);

        // seek to the beginning of the file to start reading
        fseek(html_file, 0, SEEK_SET);
        fread(html_file_buffer, 1, len, html_file);

        fclose(html_file);
    }
    return html_file_buffer;
}
