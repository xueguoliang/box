
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


typedef struct heap_t
{
    // int* data;
    void** data; // [box_event_timer*, box_event_timer*]
    int data_cap;  // 申请的空间
    int data_use;  // 使用的空间
    int (*cmp)(void* ptr1, void* ptr2);
} heap_t;


static inline heap_t* heap_create(int (*cmp)(void* ptr1, void* ptr2))
{
    heap_t* heap = (heap_t*)malloc(sizeof(heap_t));
    heap->data_cap = 16;
    heap->data_use = 0;
    heap->data = (void**)malloc(sizeof(void*) * heap->data_cap);
    heap->cmp = cmp;
    return heap;
}

static inline void heap_swap(heap_t* heap, int index1, int index2)
{
    void* tmp = heap->data[index1];
    heap->data[index1] = heap->data[index2];
    heap->data[index2] = tmp;
}

static inline void heap_add(heap_t* heap, void* value)
{
    if(heap->data_use == heap->data_cap)
    {
        heap->data_cap += 16;
        // 拓展空间
        heap->data = (void**)realloc(heap->data, sizeof(void*) * heap->data_cap);
    }

    int index = heap->data_use++;
    int parent_index;
    heap->data[index] = value;

    while(1)
    {
        if(index == 0)
            break;

        parent_index = (index - 1) / 2;

      //  if(heap->data[index] < heap->data[parent_index])
        if(heap->cmp(heap->data[index], heap->data[parent_index]) < 0)
        {
            heap_swap(heap, index, parent_index);
            index = parent_index;
        }
        else
        {
            break;
        }
    }
}

static inline void heap_del(heap_t* heap, int index)
{
    if(index >= heap->data_use)
        return;

    // 和最后一个节点交换
    int last_index = heap->data_use-1;
    heap_swap(heap, index, last_index);
    heap->data_use --;

    // 针对index做两个方向的比较
    // 向下比较
    int p = index;
    while(1)
    {
        // 两个儿子的坐标确定
        int cl = p * 2 + 1;
        int cr = p * 2 + 2;

        void* d = heap->data[p];
        if(cr >= heap->data_use) // 如果右边没有儿子
        {
            if(cl < heap->data_use) // 如果左边有儿子
            {
                void* l = heap->data[cl];
               // if(l < d)
                if(heap->cmp(l, d) < 0)
                {
                    heap_swap(heap, p, cl);
                }    
            }

            break;
        }
        else // 左儿子和右儿子都有
        {
            void* r = heap->data[cr];
            void* l = heap->data[cl];
            //if(l < r)
             if(heap->cmp(l, r) < 0)
            {
             //   if(l < d)
                    if(heap->cmp(l, d) < 0)
                {
                    heap_swap(heap, p, cl);
                    p = cl;
                }
                else
                {
                    break;
                }
            }
            else
            {
              //  if(r < d)
                     if(heap->cmp(r, d) < 0)
                {
                    heap_swap(heap, p, cr);
                    p = cr;
                }
                else
                {
                    break;
                }
            }
        }
    }


    int c = index;
    while(1)
    {
        if(c == 0)
        {
            break;
        }
        
        int p = (c - 1) / 2;
        if(heap->cmp(heap->data[p], heap->data[c]) > 0)
        //if(heap->data[p] > heap->data[c])
        {
            heap_swap(heap, p, c);
            c = p;
        }
        else
        {
            break;
        }
    }
}

#if 0
int main()
{
    heap_t* heap = heap_create();
    heap_add(heap, 990);
    heap_add(heap, 8);
    heap_add(heap, 111);
    heap_add(heap, 200);
    heap_add(heap, 7);
    heap_add(heap, 99);
    heap_add(heap, 10001);

    int i;
    for(i=0; i<7; ++i)
        heap_del(heap, 0);

    heap_dump(heap, 7);
}
#endif
