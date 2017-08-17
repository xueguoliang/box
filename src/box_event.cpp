#include "box_event.h"
#include <stdio.h>
#include <sys/epoll.h>
#include <string.h>
box_event::box_event(int type):type(type)
{
    ref = 1;
}

box_event_sock::box_event_sock(int fd) :
    box_event(box_event_type_sock), fd(fd)
{
    rcbk = NULL;
    wcbk = NULL;
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

void box_event::add_ref()
{
    ref++;
}

void box_event::del_ref()
{
    ref--;
}

void box_event_sock::close()
{
    del_ref();

    ::close(fd);
    fd = -1;

}

box_event_timer::box_event_timer(int timeout, box_timer_callback cbk, void *ptr):
    box_event(box_event_type_timer),
    timeout(timeout),
    cbk(cbk)
{
    this->expire = box_now() + timeout;
}

box_event_buffer::box_event_buffer(int fd,
                                   box_buffer_callback cbk,
                                   box_buffer_callback err,
                                   int watermark):
    box_event(box_event_type_buffer),
    fd(fd),
    cbk(cbk),
    err(err),
    watermark(watermark)
{

}

void box_event_buffer::read(char *buf, size_t size)
{
    memcpy(buf, this->read_buf.data(), size);
    this->read_buf.erase(this->read_buf.begin(), this->read_buf.begin()+size);
}

void box_event_buffer::write(char *buf, size_t size)
{
    copy(buf, buf+size, back_inserter(this->write_buf));
}

void box_event_buffer::close()
{
    del_ref();
    ::close(fd);
    fd = -1;
}
