/* Operating Systems Project 4
Leah Plofchan and Brynna Conway */
 
#ifndef _QUEUE_
#define _QUEUE_

#include <iostream>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <queue>

using namespace std;

template<typename T>
class Queue {
    public:
        Queue();        // constructor
        void push(T element);
        void pop();

    private:
       pthread_mutex_t lock;
       queue<T> Q;

}

// constructor
template<typename T>
Queue::Queue() {
    lock = PTHREAD_MUTEX_INITIALIZER;
}

template<typename T>
void Queue::push(T element) {
    pthread_mutex_lock(&lock);
    Q.push(element);
    pthread_mutex_unlock(&lock);

}

template<typename T>
void Queue::pop() {
    pthread_mutex_lock(&lock);
    Q.pop();
    pthread_mutex_unlock(&lock);
}

#endif _QUEUE_