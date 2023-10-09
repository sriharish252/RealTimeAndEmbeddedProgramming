//
// Credits to https://github.com/moss-n/multithreading-example-1/blob/master/order_violation_solution.c
//      for Code Inspiration
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


char * HELLO_MESSAGE;
int THREADS_CREATED = 0;
pthread_mutex_t hello_msg_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t msg_created_cond = PTHREAD_COND_INITIALIZER;
int msg_initialized = 0;

void * workerThreadFunc(void * tid){
    long * myID = (long *) tid;
    printf("\nTHIS IS THREAD %ld\n",*myID);
    HELLO_MESSAGE = "HELLO WORLD!";
    pthread_mutex_lock(&hello_msg_lock);
    msg_initialized = 1;
    pthread_cond_signal(&msg_created_cond);
    pthread_mutex_unlock(&hello_msg_lock);
}

void * workerThreadFunc2(void * tid){
    long * myID = (long *) tid;
    printf("\nTHIS IS THREAD %ld\n",*myID);
    pthread_mutex_lock(&hello_msg_lock);
    while(msg_initialized == 0){
        pthread_cond_wait(&msg_created_cond,&hello_msg_lock);
    }
    for(int i = 0; i < 13; i++){
        printf("%c",HELLO_MESSAGE[i]);
    }
    pthread_mutex_unlock(&hello_msg_lock);
}

int main(){

    pthread_t tid0;
    pthread_t tid1;

    pthread_create(&tid0,NULL,workerThreadFunc,(void *)&tid0);
    pthread_create(&tid1,NULL,workerThreadFunc2,(void *)&tid1);

    pthread_exit(NULL);
    return 0;
}