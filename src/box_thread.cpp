#include "box_thread.h"
#include "box.h"
#include "box_event.h"

box_thread::box_thread(box* b):b(b)
{
    pthread_create(&tid, NULL, thread_func, this);
}

void box_thread::run()
{
    while(1)
    {
        b->wait_event();
        box_event* ev = b->get_event();
        if(ev)
        {
            if(ev->type == box_event_type_sock)
            {
                box_event_sock* sockev = (box_event_sock*)ev;
                if((sockev->cbk_events & EPOLLIN) && sockev->ref > 0)
                {
                    sockev->rcbk(sockev);
                }

                if((sockev->cbk_events & EPOLLOUT) && sockev->ref > 0)
                {
                    sockev->wcbk(sockev);
                }

                if(sockev->ref != 0)
                    b->epoll_add(sockev);
                else
                {
                    box_debug("delete socket event\n");
                    delete sockev;
                }
            }
            else if(ev->type == box_event_type_timer)
            {
                box_event_timer* timerev = (box_event_timer*)ev;
                timerev->cbk(timerev);
                delete timerev;
            }
            else if(ev->type == box_event_type_buffer)
            {
                box_event_buffer* bufev = (box_event_buffer*)ev;
                if(bufev->write_buf.size() > 0) // write
                {
                    int ret = write(bufev->fd,
                                    bufev->write_buf.data(),
                                    bufev->write_buf.size());
                    if(ret > 0)
                    {
                        bufev->write_buf.erase(bufev->write_buf.begin(),
                                               bufev->write_buf.begin() + ret);
                    }
                    else
                    {
                        if(errno == EAGAIN)
                        {

                        }
                        else
                        {
                            bufev->ret = ret;
                            bufev->err_no = errno;
                            close(bufev->fd);
                            bufev->fd = -1;
                            bufev->err(bufev);
                            continue;
                        }
                    }
                }

                // try read

                char buf[1024];
                int ret = read(bufev->fd, buf, bufev->watermark - bufev->read_buf.size());
                if(ret > 0)
                {
                    copy(buf, buf+ret, back_inserter(bufev->read_buf));
                    if(bufev->read_buf.size() == bufev->watermark)
                    {
                        bufev->cbk(bufev);
                        bufev->read_buf.clear();
                    }
                }
                else if(ret == 0)
                {
                    // peer close socket
                    close(bufev->fd);
                    bufev->fd = -1;
                    bufev->ret = ret;
                    bufev->err_no = errno;
                    bufev->err(bufev);
                }
                else if(ret < 0)
                {
                    if(errno == EAGAIN)
                    {}
                    else
                    {
                        // error
                        bufev->ret = ret;
                        bufev->err_no = errno;
                        close(bufev->fd);
                        bufev->fd = -1;
                        bufev->err(bufev);
                    }
                }

                if(bufev->fd > 0)
                {
                    b->epoll_add(bufev);
                }
                else
                {
                    delete bufev;
                }
            }
        }
    }
}

void *box_thread::thread_func(void *ptr)
{
    box_thread* This = (box_thread*)ptr;
    This->run();
    return NULL;
}
