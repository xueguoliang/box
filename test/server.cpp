#include "box.h"
#include "box_util.h"
#include "box_event.h"

box b;

void read_cbk(box_event_sock* ev)
{
    char buf[1024];
    if(read(ev->fd, buf, sizeof(buf)) <= 0)
    {
        ev->close(); // release self
    }
    printf("buf=%s\n", buf);
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
        b.add_socket(newfd, read_cbk);
    }
}

int main()
{

    b.init(4);

    int server = box_create_server(9988);

    b.add_socket(server, accept_cbk);

    b.run();
    return 0;
}
