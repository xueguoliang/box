#include "box.h"
#include "box_event.h"
#include "box_thread.h"

int box_timer_cmp(void* ptr1, void* ptr2)
{
    box_event_timer* timer1 = (box_event_timer*)ptr1;
    box_event_timer* timer2 = (box_event_timer*)ptr2;
    return timer1->expire - timer2->expire;
}

box::box()
{

}

void box::init(int thread_count)
{
    this->epollfd = epoll_create(1024);

    this->timers = heap_create(box_timer_cmp);

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
        lock();
        uint64_t time_wait = get_wait_time();
        unlock();
        int ret = epoll_wait(this->epollfd, evs, 8, time_wait);
        if(ret > 0)
        {
            for(int i=0;i <ret; ++i)
            {
                box_event* ev = (box_event*)evs[i].data.ptr;
                if(ev->type == box_event_type_sock)
                {
                    box_event_sock*ev = (box_event_sock*)evs[i].data.ptr;
                    ev->cbk_events = evs[i].events;
                }
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

void box::add_timer(int timeout, box_timer_callback cbk, void *ptr)
{
    box_event_timer* ev = new box_event_timer(timeout, cbk, ptr);
    lock();
    heap_add(timers, ev);
    unlock();
}

box_event_buffer* box::add_buffer(int fd, box_buffer_callback cbk, box_buffer_callback err,int watermark)
{
    box_event_buffer* ev = new box_event_buffer(fd, cbk,err, watermark);
    epoll_add(ev);
    return ev;
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

uint64_t box::get_wait_time()
{
    uint64_t now = box_now();
    while(timers->data_use > 0)
    {
        box_event_timer* timer =  (box_event_timer*)this->timers->data[0];
        if(timer->expire < now)
        {
         //   timer->cbk(timer);
            add_event(timer);
            heap_del(timers, 0);
        }
        else
        {
            return timer->expire - now;
        }
    }

    return 5000;
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

void box::epoll_add(box_event_buffer *ev)
{
    struct epoll_event epollev;
    epollev.data.ptr = ev;
    epollev.events = EPOLLIN|EPOLLONESHOT;
    if(ev->write_buf.size() > 0)
        epollev.events |= EPOLLOUT;

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
