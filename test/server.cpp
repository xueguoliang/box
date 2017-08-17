#include "box.h"
#include "box_util.h"
#include "box_event.h"
#include "cJSON.h"
#include <string>
#include <map>
using namespace std;

typedef struct user_t
{
    string ip;
    box_event_buffer* bufev;
} user_t;

map<string, user_t*> users;

box b;

void err_cbk(box_event_buffer* bufev)
{
    bufev->close();
}

void packet_ready(box_event_buffer* bufev)
{
    user_t* user = (user_t*)bufev->ptr;

    char* buf = (char*)alloca(bufev->watermark+1);
    buf[bufev->watermark] = 0;
    bufev->read(buf, bufev->watermark);

    /*
     *  {
     *      ip: ip
     *      msg: msg
     *  }
    */

    cJSON* root = cJSON_Parse(buf);
    string ip = cJSON_GetObjectItem(root, "ip")->valuestring;
    string msg = cJSON_GetObjectItem(root, "msg")->valuestring;
    cJSON_Delete(root);

    user_t* dst;
    {
        box_autolock lock(&b);
        if(users.find(ip) == users.end())
        {
            return;
        }
        dst = users[ip];
        dst->bufev->add_ref();
    }

    root = cJSON_CreateObject();
    /*
        {
            ip: ip, // from
            msg: msg
         }
    */
    cJSON_AddStringToObject(root, "ip", user->ip.c_str());
    cJSON_AddStringToObject(root, "msg", msg.c_str());
    char* json = cJSON_Print(root);
    cJSON_Delete(root);

    uint32_t header = strlen(json);
    header = htons(header);
    dst->bufev->write((char*)&header, 4);
    dst->bufev->write(json, strlen(json));
    free(json);

    box_autolock lock(&b);
    dst->bufev->del_ref();
    if(dst->bufev->ref == 0)
    {
        delete dst->bufev;
        users.erase(users.find(dst->ip));
        delete dst;
    }
}

void header_ready(box_event_buffer* bufev)
{
    uint32_t header;
    bufev->read((char*)&header, 4);

    header = ntohl(header);

    bufev->cbk = packet_ready;
    bufev->watermark = header;
}


void accept_cbk(box_event_sock* sock)
{
    while(1)
    {
        struct sockaddr_in addr;
        socklen_t len =sizeof(addr);
        int newfd = accept(sock->fd, (struct sockaddr*)&addr, &len);
        if(newfd < 0)
            break;

        box_event_buffer* ev =
                b.add_buffer(newfd, header_ready, err_cbk, 4);

        user_t* user = new user_t;
        user->bufev = ev;
        user->ip = box_getip(&addr.sin_addr);

        b.lock();
        users[user->ip] = user;
        b.unlock();

        ev->ptr = user;
    }

    if(errno == EAGAIN)
        return;

    exit(1);
}
int main()
{
    b.init(4);

    int server = box_create_server(9988);
    b.add_socket(server, accept_cbk);

    b.run();
    return 0;
}
