#ifndef BOX_H
#define BOX_H

class box_event;
class box_event_sock;
class box_event_timer;
class box_event_buffer;
class box_thread;

#include <stdio.h>
#include <pthread.h>
#include <list>
#include <semaphore.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "box_util.h"
#include "box_heap.h"
using namespace std;

/*
 * 事件监控类，监控所有的socket，如果有事件,将事件加入到消息队列 
 * */
class box
{
public:
    box();

    // 业务函数
    // 初始化环境
    void init(int thread_count);
    // 把socket加入到epoll
    void add_socket(int fd, box_read_callback rcbk, box_write_callback wcbk = NULL);
    // 把socket加入到epoll
    void epoll_add(box_event_sock* ev);
    void epoll_add(box_event_buffer* ev);
    // 监听socket
    void run();
    //
    void add_timer(int timeout, box_timer_callback cbk, void* ptr = NULL);
    void add_buffer(int fd, box_buffer_callback cbk,box_buffer_callback err, int watermark);

    // 一些辅助函数
    void lock();
    void unlock();

    void add_event(box_event*);
    box_event* get_event(); // called by thread

    void wait_event();
    uint64_t get_wait_time();

private:
    int epollfd;
    pthread_mutex_t mutex;
    sem_t sem;

    list<box_thread*> threads;
    list<box_event*> evs;

    heap_t* timers;
};

class box_autolock
{
public:
    box_autolock(box* b);
    ~box_autolock();

private:
    box* b;
};

#endif // BOX_H
