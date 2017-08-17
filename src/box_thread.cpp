#include "box_thread.h"
#include "box.h"
#include "box_event.h"

box_thread::box_thread(box* b):b(b)
{
    pthread_create(&tid, NULL, thread_func, this);
}

void box_thread::run()
{
    while(1)
    {
        b->wait_event();
        box_event* ev = b->get_event();
        if(ev)
        {
            if(ev->type == box_event_type_sock)
            {
                box_event_sock* sockev = (box_event_sock*)ev;
                if((sockev->cbk_events & EPOLLIN) && sockev->ref > 0)
                {
                    sockev->rcbk(sockev);
                }

                if((sockev->cbk_events & EPOLLOUT) && sockev->ref > 0)
                {
                    sockev->wcbk(sockev);
                }

                if(sockev->ref != 0)
                    b->epoll_add(sockev);
                else
                {
                    box_debug("delete socket event\n");
                    delete sockev;
                }
            }
        }
    }
}

void *box_thread::thread_func(void *ptr)
{
    box_thread* This = (box_thread*)ptr;
    This->run();
    return NULL;
}
