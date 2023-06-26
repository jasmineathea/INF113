/*
ASSIGNMENT 2: ice cream stands
by Jasmine Næss
*/
#include <pthread.h> // pthread
#include <stdio.h>   // prinftf
#include <stdlib.h>  // random
#include <unistd.h>  // sleep
#include <semaphore.h>  // semaphore

void *worker_actions(void *employee_id);
void *customer_actions(void *personal_id);

/* functions for implementing a FIFO-queue system */
void enqueue(int id);
int dequeue();

#define NUM_CUSTOMERS 5
#define NUM_WORKERS 2
#define MAX_CAPACITY 3 // maximum size of the queue

/* defining semaphores */
sem_t serving;
sem_t workers;

/* mutex */
pthread_mutex_t get_queue;

/* other parameters */
int worker0;
int worker1;
int customer_counter; // keep track of queue-size
int queue[MAX_CAPACITY], front = 0, rear = 0; // queue represented as a list. saving the front and rear values :)

int main(int argc, char **argv) {
    /* initialize semaphores */
    sem_init(&serving, 0, 0);
    sem_init(&workers, 0, 0);

    int personal_ids[NUM_CUSTOMERS], employee_ids[NUM_WORKERS];

    /* customer and worker threads */
    pthread_t customers[NUM_CUSTOMERS], workers[NUM_WORKERS];

    /* create treads */
    for (int i = 0; i < NUM_WORKERS; i++) {
        employee_ids[i] = i;
        pthread_create(&workers[i], NULL, worker_actions, (void *)&employee_ids[i]);
    }

    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        personal_ids[i] = i;
        pthread_create(&customers[i], NULL, customer_actions, (void *)&personal_ids[i]);
    }

    /* join threads */
    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(workers[i], NULL);
    }

    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customers[i], NULL);
    }

    return 0;
}

void *worker_actions(void *employee_id) {
    /* get the id of this employee */
    int id = *(int *)employee_id;
    int time = 1 + rand() % 10; // random number between 1 and 10

    while (1) {
        if (customer_counter == 0) {
            printf("\tNo customers. Worker %d sleeps 😴\n", id);
            worker0 = 1; // worker sleeps when value = 1
            worker1 = 1;
        }

        sem_wait(&workers); // wait for workers to call sem_post (critical section)

        int person_id = dequeue(); // remove customer from queue

        if (worker0 == 1) {
            printf("Person %d wakes up worker %d 📣\n", person_id, id);
            worker0 --;
        }

        else if (worker1 == 1) {
            printf("Person %d wakes up worker %d 📣\n", person_id, id);
            worker1 --;
        }
    
        printf("\tWorker %d is making ice cream for person %d. Taking %d seconds...\n", id, person_id, time);
        sleep(time);
        
        sem_post(&serving); // release lock (worker is not serving anyone else)
    }
}

void *customer_actions(void *personal_id) {
    /* get the id of this customer */
    int id = *(int *)personal_id;
    int time = 1 + rand() % 10; // random number between 1 and 10

    while (1) {
        printf("Person %d is relaxing on the beach for %d seconds 🌴\n", id, time);
        sleep(time);

        if (customer_counter < MAX_CAPACITY) {
		    printf("Person %d gets in queue. %d others waiting...\n", id, customer_counter);
            enqueue(id); // add person to queue
            sleep(time);

            sem_post(&workers); // release lock; worker actions begin (sem_wait on line 89)

            sem_wait(&serving); // lock: employee is serving (critical section)
            printf("\t\tPerson %d received ice cream 🍦\n", id);
            sleep(time);
        }
    }
}

/* function for entering the queue */
void enqueue(int id) {
    pthread_mutex_lock(&get_queue); // critical seciton
    queue[rear] = id;
    rear = (rear +1) % MAX_CAPACITY;
    customer_counter += 1;
    pthread_mutex_unlock(&get_queue);
}

/* function for removing an element from queue */
int dequeue() {
    pthread_mutex_lock(&get_queue); // critical section
    int result = queue[front];
    front = (front +1) % MAX_CAPACITY;
    customer_counter -= 1;
    pthread_mutex_unlock(&get_queue);
    return result;
}