#include "box.h"
#include "box_event.h"
#include "box_thread.h"

box::box()
{

}

void box::init(int thread_count)
{
    this->epollfd = epoll_create(1024);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&this->mutex, &attr);

    sem_init(&sem, 0, 0);

    for(int i=0; i<thread_count; ++i)
    {
        this->threads.push_back(new box_thread(this));
    }
}

void box::add_socket(int fd, box_read_callback rcbk, box_write_callback wcbk)
{
    box_event_sock* ev = new box_event_sock(fd);
    ev->set_callback(rcbk, wcbk);
    epoll_add(ev);
}

void box::run()
{
    struct epoll_event evs[8];
    while(1)
    {
        int ret = epoll_wait(this->epollfd, evs, 8, 5000);
        if(ret > 0)
        {
            for(int i=0;i <ret; ++i)
            {
                box_event_sock*ev = (box_event_sock*)evs[i].data.ptr;
                ev->cbk_events = evs[i].events;
                add_event(ev);
            }
        }
        else if(ret < 0 && errno != EINTR)
        {
            box_err("error\n");
            exit(1);
        }
    }
}

void box::lock()
{
    pthread_mutex_lock(&mutex);
}

void box::unlock()
{
    pthread_mutex_unlock(&mutex);
}

void box::add_event(box_event *ev)
{
    lock();
    evs.push_back(ev);
    unlock();

    sem_post(&sem);
}

box_event *box::get_event()
{
    box_event* ev = NULL;
    lock();
    if(evs.size() != 0)
    {
        ev = *evs.begin();
        evs.pop_front();
    }
    unlock();

    return ev;
}

void box::wait_event()
{
    sem_wait(&sem);
}

void box::epoll_add(box_event_sock *ev)
{
    struct epoll_event epollev;
    epollev.data.ptr = ev;
    epollev.events = ev->events | EPOLLONESHOT;
    if(epoll_ctl(epollfd, EPOLL_CTL_MOD, ev->fd, &epollev) != 0)
    {
        epoll_ctl(epollfd, EPOLL_CTL_ADD, ev->fd, &epollev);
    }
}

box_autolock::box_autolock(box *b):b(b)
{
    b->lock();
}

box_autolock::~box_autolock()
{
    b->unlock();
}
