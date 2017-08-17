#ifndef BOX_EVENT_H
#define BOX_EVENT_H

#include "box_util.h"
#include <vector>
using namespace std;
#define box_event_type_sock 1
#define box_event_type_timer 2
#define box_event_type_buffer 3

// 封装一个event，event可能是socket或者是定时器
class box_event
{
public:
    box_event(int type);
    int type;
    int ref;

    void add_ref();
    void del_ref();

    void* ptr;
};

class box_event_buffer: public box_event
{
public:
    box_event_buffer(int fd, box_buffer_callback, box_buffer_callback, int watermark);

    void read(char* buf, size_t size);
    void write(char* buf, size_t size);

    box_buffer_callback cbk;
    box_buffer_callback err;

    void close();

    vector<char> read_buf;
    vector<char> write_buf;

    int ret;
    int err_no;

    int fd;
    int watermark;
};

class box_event_sock : public box_event
{
public:
    box_event_sock(int fd);

    void set_callback(box_read_callback r, box_write_callback w);
    void close();



public:
    int fd;
    box_read_callback rcbk;
    box_write_callback wcbk;
    int cbk_events;
    int events;

};

class box_event_timer: public box_event
{
public:
    box_event_timer(int timeout, box_timer_callback cbk, void* ptr);

public:
    uint64_t expire;  // shi ke
    int timeout;
    box_timer_callback cbk;

};



#endif // BOX_EVENT_H
