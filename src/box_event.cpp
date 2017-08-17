#include "box_event.h"
#include <stdio.h>
#include <sys/epoll.h>
box_event::box_event(int type):type(type)
{

}

box_event_sock::box_event_sock(int fd) :
    box_event(box_event_type_sock), fd(fd)
{
    rcbk = NULL;
    wcbk = NULL;
    ref = 1;
}

void box_event_sock::set_callback(box_read_callback r, box_write_callback w)
{
    this->events = 0;

    if(r)
    {
        this->events |= EPOLLIN;
        rcbk = r;
    }
    if(w)
    {
        this->events |= EPOLLOUT;
        wcbk = w;
    }
}

void box_event_sock::add_ref()
{
    ref++;
}

void box_event_sock::del_ref()
{
    ref--;
}

void box_event_sock::close()
{
    del_ref();
    if(ref == 0)
    {
        ::close(fd);
        fd = -1;
    }
}

box_event_timer::box_event_timer(int timeout, box_timer_callback cbk, void *ptr):
    box_event(box_event_type_timer),
    timeout(timeout),
    cbk(cbk),
    ptr(ptr)
{
    this->expire = box_now() + timeout;
}
