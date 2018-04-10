#ifndef __memory_pool__
#define __memory_pool__

#include <new>
#include <cstdlib>
#include <cstdio>
#include <iostream>


#include <sys/time.h>
#include <vector>

using namespace std;

namespace yangshu_common {
//一级配置器
template<int inst>
class __malloc_alloc_template {
private:
static void *oom_malloc(size_t);
static void *oom_realloc(void*, size_t);
static void (* __malloc_alloc_oom_handler) ();

public:
static void *allocate(size_t n) {
    void *result = malloc(n);
    if (0 == result) result = oom_malloc(n);
    return result;
}
static void deallocate(void *p, size_t) {
    free(p);
}
static void *reallocate(void *p, size_t, size_t new_sz) {
    void *result = realloc(p, new_sz);
    if (0 == result) result = oom_realloc(p, new_sz);
    return result;
}
static void (*set_malloc_handler(void (*f)()))() {
    void (* old)() = __malloc_alloc_oom_handler;
    __malloc_alloc_oom_handler = f;
    return(old);
}
};

template<int inst>
void (*__malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;

template<int inst>
void * __malloc_alloc_template<inst>::oom_malloc(size_t n) {
    void (*my_malloc_handler)();
    void *result;
    while(1) {
        my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == my_malloc_handler) {
            cerr << "out of memory" << endl; exit(1);
        }
        (*my_malloc_handler)();
        result = malloc(n);
        if (result) return result;
    }
}

template<int inst>
void * __malloc_alloc_template<inst>::oom_realloc(void *p, size_t n) {
    void (*my_malloc_handler)();
    void *result;
    while(1) {
        my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == my_malloc_handler) {
            cerr << "out of memory" << endl; exit(1);
        }
        (*my_malloc_handler)();
        result = realloc(p, n);
        if(result) return result;
    }
}

typedef __malloc_alloc_template<0> malloc_alloc;

//二级配置器
enum {__ALIGN = 8};
enum {__MAX_BYTES = 128};
enum {__NFREELISTS = __MAX_BYTES/__ALIGN};

template<bool threads, int inst>
class __default_alloc_template {
private:
union obj {
    union obj * free_list_link;
    char client_data[1];
};
private:
static size_t ROUND_UP(size_t bytes) {
    //int tmp = ((bytes)+(__ALIGN-1))%__ALIGN;
    //return (((bytes)+(__ALIGN-1))) - tmp;
    return (((bytes)+(__ALIGN-1)) & ~(__ALIGN-1)); //2的x次方可用
}
static obj * volatile free_list[__NFREELISTS];
static size_t FREELIST_INDEX(size_t bytes) {
    return (bytes+__ALIGN-1)/__ALIGN - 1;
}
static void *refill(size_t n);
static char *chunk_alloc(size_t size, int nobjs);
static char *start_free;
static char *end_free;
static size_t heap_size;
public:
static void *allocate(size_t n) {
    if (n > (size_t)__MAX_BYTES) {
        return (malloc_alloc::allocate(n));
    }
    obj * volatile * my_free_list = free_list + FREELIST_INDEX(n);
    obj * result = *my_free_list;
    if (0 == result) {
        void *r = refill(ROUND_UP(n));
        return r;
    }
    *my_free_list = result->free_list_link;
    return result;
}
static void deallocate(void *p, size_t n) {
    if (n > (size_t)__MAX_BYTES) {
        malloc_alloc::deallocate(p, n);
        return;
    }
    obj * volatile * my_free_list = free_list + FREELIST_INDEX(n);
    obj * old = (obj *)p;
    old->free_list_link = *my_free_list;
    *my_free_list = old;
}
static void *reallocate(void *p, size_t old_size, size_t new_size) {

}
};


template<bool threads, int inst>
char *__default_alloc_template<threads, inst>::start_free = 0;

template<bool threads, int inst>
char *__default_alloc_template<threads, inst>::end_free = 0;

template<bool threads, int inst>
size_t
__default_alloc_template<threads, inst>::heap_size = 0;

template<bool threads, int inst>
typename __default_alloc_template<threads, inst>::obj * volatile
__default_alloc_template<threads, inst>::free_list[__NFREELISTS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};

template<bool threads, int inst>
void *__default_alloc_template<threads, inst>::refill(size_t n) {
    //初始化
    int nobjs = 20;
    char * chunk = chunk_alloc(n, nobjs);
    if (1 == nobjs) {
        return chunk;
    }
    obj * volatile * my_free_list = free_list + FREELIST_INDEX(n);
    obj * result = (obj *)chunk;
    obj * next_obj = (obj *)(chunk + n);
    *my_free_list = next_obj;
    for (int i = 1;; ++i) {
        obj *current_obj = next_obj;
        next_obj = (obj*)((char*)next_obj+n);
        if (nobjs-1 == i) {
            current_obj->free_list_link = 0;
            break;
        } else {
            current_obj->free_list_link = next_obj;
        }
    }
    return result;
}

template<bool threads, int inst>
char *__default_alloc_template<threads, inst>::chunk_alloc(size_t size, int nobjs) {
    char *result;
    size_t total_bytes = size*nobjs;
    size_t bytes_left = end_free - start_free;

    if (bytes_left >= total_bytes) {
        result = start_free;
        start_free = start_free + total_bytes;
        return result;
    } else if (bytes_left >= size) {
        result = start_free;
        nobjs = bytes_left/size;
        start_free = start_free + nobjs*size;
        return result;
    } else {
        if (bytes_left > 0) {
            obj * volatile * my_free_list = free_list + FREELIST_INDEX(bytes_left);
            ((obj*)start_free)->free_list_link = *my_free_list;
            *my_free_list = (obj*)start_free;
        }
        size_t need_get = (total_bytes<<1);
        start_free = (char*)malloc(need_get);
        if (0 == start_free) {
            //obj * volatile * my_free_list,*p;
            obj * volatile * my_free_list;
            obj *p;
            for (int i = size; i <= __MAX_BYTES ; i+=__ALIGN) {
                my_free_list = free_list + FREELIST_INDEX(i);
                p = *my_free_list;
                if (0 != p) {
                    *my_free_list = p->free_list_link;
                    start_free = (char*)p;
                    end_free = start_free+i;
                    return chunk_alloc(size, nobjs);
                }
            }
            end_free = 0;
            start_free = (char *)malloc_alloc::allocate(need_get);
        }
        end_free = start_free + need_get;
        return chunk_alloc(size, nobjs);
    }
}
}

#endif

int main() {
    //一级配置器
    void *p = yangshu_common::malloc_alloc::allocate(5);
    yangshu_common::malloc_alloc::deallocate(p, 1);

    //二级配置器
    p = yangshu_common::__default_alloc_template<false, 0>::allocate(5);
    yangshu_common::__default_alloc_template<false, 0>::deallocate(p, 5);
    struct timeval tv;
    long long start, end;
    {
        gettimeofday(&tv,NULL);
        start = tv.tv_sec*1000 + tv.tv_usec/1000;
        vector<int*> vec;
        vec.reserve(10000);
        for (int i = 0; i < 10000; ++i) {
            for (int j = 0; j < 10000; ++j) {
                vec.push_back(new int);
            }
            for (int j = 0; j < 10000; ++j) {
                delete vec[j];
            }
            vec.clear();
        }
        gettimeofday(&tv,NULL);
        end = tv.tv_sec*1000 + tv.tv_usec/1000;
        printf("timestamp : %lld\n", end - start);
    }
    {
        gettimeofday(&tv,NULL);
        start = tv.tv_sec*1000 + tv.tv_usec/1000;
        vector<int*> vec;
        vec.reserve(10000);
        for (int i = 0; i < 10000; ++i) {
            for (int j = 0; j < 10000; ++j) {
                vec.push_back((int*)yangshu_common::__default_alloc_template<false, 0>::allocate(sizeof(int)));
            }
            for (int j = 0; j < 10000; ++j) {
                yangshu_common::__default_alloc_template<false, 0>::deallocate(vec[i], sizeof(int));
            }
            vec.clear();
        }
        gettimeofday(&tv,NULL);
        end = tv.tv_sec*1000 + tv.tv_usec/1000;
        printf("timestamp : %lld\n", end - start);
    }
    return 0;
}
