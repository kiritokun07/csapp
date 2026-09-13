#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

typedef struct cache_node {
    char uri[MAXLINE];
    char *data;
    int size;
    struct cache_node *prev;
    struct cache_node *next;
} cache_node_t;

static cache_node_t *cache_head;
static cache_node_t *cache_tail;
static int cache_bytes;
static pthread_rwlock_t cache_lock;

void cache_init(void);
int cache_lookup(const char *uri, char *buf, int *size);
void cache_insert(const char *uri, const char *data, int size);
static void cache_move_to_head(cache_node_t *node);
static void cache_evict_lru(void);

void *thread(void *vargp);
void doit(int connfd);
void parse_uri(const char *uri, char *hostname, char *port, char *path);
void build_request(char *request, const char *hostname, const char *port,
                   const char *path, rio_t *client_rio);
int should_forward_header(const char *header);

int main(int argc, char **argv)
{
    int listenfd;
    int *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    /* write() to a closed socket would otherwise kill the process */
    Signal(SIGPIPE, SIG_IGN);
    cache_init();

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        /* heap copy so Accept cannot overwrite the fd before the worker reads it */
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Pthread_create(&tid, NULL, thread, connfdp);
    }
    return 0;
}

void *thread(void *vargp)
{
    int connfd = *((int *)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);
    doit(connfd);
    Close(connfd);
    return NULL;
}

void doit(int connfd)
{
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char hostname[MAXLINE], port[MAXLINE], path[MAXLINE];
    char request[MAXLINE];
    char object[MAX_OBJECT_SIZE];
    int object_size;
    int serverfd;
    ssize_t n;
    rio_t client_rio, server_rio;

    rio_readinitb(&client_rio, connfd);
    if (rio_readlineb(&client_rio, buf, MAXLINE) <= 0)
        return;

    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET"))
        return;

    if (cache_lookup(uri, object, &object_size)) {
        rio_writen(connfd, object, object_size);
        return;
    }

    parse_uri(uri, hostname, port, path);
    build_request(request, hostname, port, path, &client_rio);

    serverfd = open_clientfd(hostname, port);
    if (serverfd < 0)
        return;

    if (rio_writen(serverfd, request, strlen(request)) != (ssize_t)strlen(request)) {
        Close(serverfd);
        return;
    }

    rio_readinitb(&server_rio, serverfd);
    object_size = 0;
    while ((n = rio_readnb(&server_rio, buf, MAXLINE)) > 0) {
        if (rio_writen(connfd, buf, n) != n)
            break;
        if (object_size + (int)n <= MAX_OBJECT_SIZE) {
            memcpy(object + object_size, buf, n);
            object_size += (int)n;
        } else {
            object_size = MAX_OBJECT_SIZE + 1;
        }
    }
    Close(serverfd);

    if (object_size > 0 && object_size <= MAX_OBJECT_SIZE)
        cache_insert(uri, object, object_size);
}

void parse_uri(const char *uri, char *hostname, char *port, char *path)
{
    char buf[MAXLINE];
    char *host, *slash, *colon;

    strcpy(port, "80");
    strcpy(buf, uri);

    host = strstr(buf, "://");
    host = host ? host + 3 : buf;

    slash = strchr(host, '/');
    if (slash) {
        strcpy(path, slash);
        *slash = '\0';
    } else {
        strcpy(path, "/");
    }

    colon = strchr(host, ':');
    if (colon) {
        *colon = '\0';
        strcpy(port, colon + 1);
    }
    strcpy(hostname, host);
}

void build_request(char *request, const char *hostname, const char *port,
                   const char *path, rio_t *client_rio)
{
    char buf[MAXLINE], extra[MAXLINE];
    int has_host = 0;

    extra[0] = '\0';
    sprintf(request, "GET %s HTTP/1.0\r\n", path);

    while (rio_readlineb(client_rio, buf, MAXLINE) > 0) {
        if (!strcmp(buf, "\r\n"))
            break;
        if (!strncasecmp(buf, "Host:", 5)) {
            has_host = 1;
            strcat(request, buf);
        } else if (should_forward_header(buf)) {
            strcat(extra, buf);
        }
    }

    if (!has_host) {
        if (!strcmp(port, "80"))
            sprintf(buf, "Host: %s\r\n", hostname);
        else
            sprintf(buf, "Host: %s:%s\r\n", hostname, port);
        strcat(request, buf);
    }

    strcat(request, user_agent_hdr);
    strcat(request, "Connection: close\r\n");
    strcat(request, "Proxy-Connection: close\r\n");
    strcat(request, extra);
    strcat(request, "\r\n");
}

int should_forward_header(const char *header)
{
    return strncasecmp(header, "User-Agent:", 11) &&
           strncasecmp(header, "Connection:", 11) &&
           strncasecmp(header, "Proxy-Connection:", 17);
}

void cache_init(void)
{
    cache_head = NULL;
    cache_tail = NULL;
    cache_bytes = 0;
    pthread_rwlock_init(&cache_lock, NULL);
}

int cache_lookup(const char *uri, char *buf, int *size)
{
    cache_node_t *node;

    pthread_rwlock_wrlock(&cache_lock);
    for (node = cache_head; node; node = node->next) {
        if (!strcmp(node->uri, uri)) {
            memcpy(buf, node->data, node->size);
            *size = node->size;
            cache_move_to_head(node);
            pthread_rwlock_unlock(&cache_lock);
            return 1;
        }
    }
    pthread_rwlock_unlock(&cache_lock);
    return 0;
}

void cache_insert(const char *uri, const char *data, int size)
{
    cache_node_t *node;

    if (size > MAX_OBJECT_SIZE)
        return;

    pthread_rwlock_wrlock(&cache_lock);
    while (cache_bytes + size > MAX_CACHE_SIZE && cache_tail)
        cache_evict_lru();

    node = Malloc(sizeof(cache_node_t));
    strncpy(node->uri, uri, MAXLINE - 1);
    node->uri[MAXLINE - 1] = '\0';
    node->data = Malloc(size);
    memcpy(node->data, data, size);
    node->size = size;
    node->prev = NULL;
    node->next = cache_head;
    if (cache_head)
        cache_head->prev = node;
    else
        cache_tail = node;
    cache_head = node;
    cache_bytes += size;
    pthread_rwlock_unlock(&cache_lock);
}

static void cache_move_to_head(cache_node_t *node)
{
    if (node == cache_head)
        return;
    if (node->prev)
        node->prev->next = node->next;
    if (node->next)
        node->next->prev = node->prev;
    if (node == cache_tail)
        cache_tail = node->prev;
    node->prev = NULL;
    node->next = cache_head;
    cache_head->prev = node;
    cache_head = node;
}

static void cache_evict_lru(void)
{
    cache_node_t *victim = cache_tail;

    if (!victim)
        return;
    cache_tail = victim->prev;
    if (cache_tail)
        cache_tail->next = NULL;
    else
        cache_head = NULL;
    cache_bytes -= victim->size;
    Free(victim->data);
    Free(victim);
}
