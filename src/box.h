#ifndef BOX_H
#define BOX_H

class box_event;
class box_event_sock;
class box_event_timer;
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
using namespace std;



class box
{
public:
    box();

    void init(int thread_count);
    void add_socket(int fd, box_read_callback rcbk, box_write_callback wcbk = NULL);
    void run();

    void lock();
    void unlock();

    void add_event(box_event*);
    box_event* get_event(); // called by thread

    void wait_event();

    void epoll_add(box_event_sock* ev);

private:
    int epollfd;
    pthread_mutex_t mutex;
    sem_t sem;

    list<box_thread*> threads;
    list<box_event*> evs;
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
