#ifndef BOX_EVENT_H
#define BOX_EVENT_H

#include "box_util.h"
#define box_event_type_sock 1
#define box_event_type_timer 2

// 封装一个event，event可能是socket或者是定时器
class box_event
{
public:
    box_event(int type);

    int type;
};

class box_event_sock: public box_event
{
public:
    box_event_sock(int fd);

    void set_callback(box_read_callback r, box_write_callback w);
    void close();

    void add_ref();
    void del_ref();

public:
    int fd;
    box_read_callback rcbk;
    box_write_callback wcbk;
    int cbk_events;
    int events;
    int ref;  //ref count
};

class box_event_timer: public box_event
{
public:
    box_event_timer(int timeout, box_timer_callback cbk, void* ptr);

public:
    uint64_t expire;  // shi ke
    int timeout;
    box_timer_callback cbk;
    void* ptr;
};

#endif // BOX_EVENT_H
