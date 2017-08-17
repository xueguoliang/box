#include "box.h"
#include "box_util.h"
#include "box_event.h"

box b;

void err_cbk(box_event_buffer* ev)
{
    printf("error callback\n");
}
/*recv header, recv data*/
void read_cbk(box_event_buffer* ev)
{
    char buf[5];
    buf[4] = 0;
    ev->read(buf, 4);
    printf("%s\n", buf);

    uint32_t header = *(uint32_t*)buf;
    header = ntohs(header);
    ev->watermark = header;
}

void accept_cbk(box_event_sock* ev)
{
    printf("some one connect\n");
    while(1)
    {
        int newfd = accept(ev->fd, NULL, NULL);
        if(newfd < 0)
            break;

        box_set_nonblock(newfd);
      //  b.add_socket(newfd, read_cbk);
        b.add_buffer(newfd, read_cbk, err_cbk, 4);
    }
}

void timeout2000(box_event_timer*)
{
    printf("timeout 2000\n");
}
void timeout1000(box_event_timer*)
{
    printf("timeout 1000\n");
}
void timeout5000(box_event_timer*)
{
    printf("timeout 5000\n");
}
int main()
{
    b.init(4);

    int server = box_create_server(9988);
    b.add_socket(server, accept_cbk);

    b.run();
    return 0;
}
