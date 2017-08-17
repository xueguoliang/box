#ifndef BOX_THREAD_H
#define BOX_THREAD_H

#include <pthread.h>
class box;

// 封装线程
class box_thread
{
public:
    box_thread(box* b);

    void run();

private:
    static void* thread_func(void* ptr);
    box* b;
    pthread_t tid;
};

#endif // BOX_THREAD_H
