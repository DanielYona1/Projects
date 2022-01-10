#include "threadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

threadpool *create_threadpool(int num_threads_in_pool)
{
    if (num_threads_in_pool > MAXT_IN_POOL)
    {
        fprintf(stderr, "illeagal number of threads");
    }
    threadpool *t = (threadpool *)malloc(sizeof(threadpool));
    if (t == NULL)
    {
        perror("allocate didnt suscess");
    }
    pthread_t *thread = (pthread_t *)calloc(num_threads_in_pool , sizeof(pthread_t));
    if (thread == NULL)
    {
        perror("allocate didnt suscess");
        free(t);
    }
    t->num_threads = num_threads_in_pool;
    t->qsize = 0;
    t->threads = thread;
    t->qhead = NULL;
    t->qtail = NULL;
    t->dont_accept = 0;
    t->shutdown = 0;
    t->qlock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    int ret = pthread_cond_init(&t->q_empty, NULL);
    if (ret == -1)
    {
        perror("initialize didnt sucess");
    }
    int ret2 = pthread_cond_init(&t->q_not_empty, NULL);
    if (ret2 == -1)
    {
        perror("initialize didnt sucess");
    }
    for (int i = 0; i < num_threads_in_pool; i++)
    {
        pthread_create(t->threads + i, NULL, do_work, t);
    }
    return t;
}

void dispatch(threadpool *from_me, dispatch_fn dispatch_to_here, void *arg)
{
    work_t *element = (work_t *)calloc(1 , sizeof(work_t));
    pthread_mutex_lock(&from_me->qlock);
    if (from_me->dont_accept == 1)
    {
        fprintf(stderr, "destroy has begun");
        return;
    }
    element->routine = dispatch_to_here;
    element->next = NULL;
    element->arg = arg;
    if (from_me->qsize == 0)
    {
        from_me->qhead = element;
        from_me->qtail = element;
    }
    else
    {
        from_me->qtail->next = element;
        from_me->qtail = element;
    }
    from_me->qsize++;
    pthread_mutex_unlock(&from_me->qlock);
    pthread_cond_signal(&from_me->q_not_empty);
}

void *do_work(void *p)
{
    while (1)
    {
        threadpool *t = (threadpool *)p;
        pthread_mutex_lock(&t->qlock);
        if (t->shutdown == 1)
        {
            pthread_mutex_unlock(&t->qlock);
            pthread_exit(NULL);
        }
        if (t->qsize == 0)
        {
            pthread_cond_wait(&t->q_not_empty, &t->qlock);
        }
        if (t->shutdown == 1)
        {
            pthread_mutex_unlock(&t->qlock);
            pthread_exit(NULL);
        }
        work_t *element = NULL;
        element = t->qhead;
        t->qhead = t->qhead->next;
        t->qsize--;
        if (t->qsize == 0 && t->dont_accept == 1)
        {
            pthread_cond_signal(&t->q_empty);
        }

        pthread_mutex_unlock(&t->qlock);
        if (element != NULL)
        {
            element->routine(element->arg);
            free(element);
        }
    }
}

void destroy_threadpool(threadpool *destroyme)
{
    pthread_mutex_lock(&destroyme->qlock);
    destroyme->dont_accept = 1;
    if (destroyme->qsize > 0)
    {
        pthread_cond_wait(&destroyme->q_empty, &destroyme->qlock);
    }

    destroyme->shutdown = 1;
    pthread_mutex_unlock(&destroyme->qlock);
    pthread_cond_broadcast(&destroyme->q_not_empty);
    pthread_cond_broadcast(&destroyme->q_empty);
    for (int i = 0; i < destroyme->num_threads; i++)
    {
        pthread_join(destroyme->threads[i], NULL);
    }
    pthread_mutex_destroy(&(destroyme->qlock));
	pthread_cond_destroy(&(destroyme->q_empty));
	pthread_cond_destroy(&(destroyme->q_not_empty));
    free(destroyme->threads);
    free(destroyme);
}

int f(void *t)
{
    int index = *(int *)t;
    for (int i = 0; i < 3; i++)
    {
        printf("%d\n", index);
        sleep(1);
    }
    return 0;
}
