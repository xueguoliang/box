#ifndef BOX_THREAD_H
#define BOX_THREAD_H

class box_event_sock;
class box_event_buffer;
class box_event_timer;
#include <pthread.h>
class box;

// 封装线程
class box_thread
{
public:
    box_thread(box* b);

    void run();

    void handle_event(box_event_sock* sockev);
    void handle_event(box_event_timer* timerev);
    void handle_event(box_event_buffer* bufev);

    void handle_event_buffer_write(box_event_buffer* bufev);
    void handle_event_buffer_read(box_event_buffer* bufev);

private:
    static void* thread_func(void* ptr);
    box* b;
    pthread_t tid;
};

#endif // BOX_THREAD_H
