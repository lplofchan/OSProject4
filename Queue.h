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
        T pop();
        T front();
        T back();
        bool empty();
        unsigned int size();
        pthread_mutex_t lock;
        
    private:
       queue<T> Q;
};

// constructor
template<typename T>
Queue<T>::Queue() {
    lock = PTHREAD_MUTEX_INITIALIZER;
}

template<typename T>
void Queue<T>::push(T element) {
    Q.push(element);
}

template<typename T>
T Queue<T>::pop() {
    T elem = Q.front();
    Q.pop();
    return elem;
}

template<typename T>
T Queue<T>::front() {
    T elem = Q.front();
    return elem;
}

template<typename T>
T Queue<T>::back() {
    T elem = Q.back();
    return elem; 
}

template<typename T>
unsigned int Queue<T>::size() {
    int mysize = Q.size();
    return mysize; 
}

template<typename T> 
bool Queue<T>::empty() {
    bool is_empty = Q.empty();
    return is_empty;
}
#endif