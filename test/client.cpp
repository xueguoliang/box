#include <inttypes.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "cJSON.h"
#include <pthread.h>
#include <string.h>
int doRead(int fd, char* buf, int size)
{
    int readlen = 0;
    while(readlen != size)
    {
        int ret = read(fd, buf + readlen, size-readlen);
        if(ret > 0)
        {
            readlen += ret;
        }
        else if(ret < 0 && errno == EINTR)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    return readlen;
}

void* thread_func(void* ptr)
{
    int fd = (intptr_t)ptr;
    while(1)
    {
        uint32_t header;
        int ret = doRead(fd, (char*)&header, 4);
        header = ntohl(header);

        char* buf = (char*)alloca(header+1);
        buf[header] = 0;
        ret = doRead(fd, buf, header);

        cJSON* root = cJSON_Parse(buf);
        cJSON* ip = cJSON_GetObjectItem(root, "ip");
        cJSON* msg= cJSON_GetObjectItem(root, "msg");
        printf("%s:%s\n", ip->valuestring, msg->valuestring);
        cJSON_Delete(root);
    }
}
int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9988);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(fd, (struct sockaddr*)&addr,sizeof(addr));

    pthread_t tid;
    pthread_create(&tid, NULL, thread_func, (void*)(intptr_t)fd);

    while(1)
    {
        char buf[4096];
        fgets(buf , sizeof(buf), stdin);
        buf[strlen(buf)-1] = 0; // remove \n
        if(buf[0] == 0)
            continue;

        char* ip = strtok(buf, ":");
        char* msg = strtok(buf, "\0");

        cJSON* root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "ip", ip);
        cJSON_AddStringToObject(root, "msg", msg);
        char* p = cJSON_Print(root);
        cJSON_Delete(root);

        uint32_t header = strlen(p);
        header = htonl(header);
        write(fd, (char*)&header, 4);
        write(fd, p, strlen(p));
        free(p);
    }

    return 0;
}
