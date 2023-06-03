#include <pthread.h>
#include "./DoubleQueue.h"

// TODO
DoubleQueue::DoubleQueue() {
    this->queue_length = 0;        // Initializes the queue to be empty
    this->head = nullptr;
    this->tail = nullptr;
    pthread_mutex_init(&lock, nullptr);     // Ready to handle concurrent operations
    pthread_cond_init(&cond, nullptr);
}

DoubleQueue::~DoubleQueue() {
    // Cleans up any remaining elements in the queue
    if (this->queue_length != 0) {
        QueueNode* currNode = this->head;
        while (currNode != nullptr) {
            QueueNode* nextNode = currNode->next;
            currNode->next = nullptr;
            currNode->value = 0.0;
            delete currNode;
            currNode = nextNode;
        }
    }
    // Return to the initial state
    this->queue_length = 0;
    this->head = nullptr;
    this->tail = nullptr;
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
}

void DoubleQueue::add(double val) {
    pthread_mutex_lock(&lock);

    QueueNode* newNode = new QueueNode;
    newNode->next = nullptr;
    newNode->value = val;

    if (this->queue_length == 0) {         // check whether the queue is empty
        this->head = newNode;
        this->tail = newNode;
        pthread_cond_signal(&cond);      // wake up the cond_wait in wait_remove()
    } else {
        (this->tail)->next = newNode;    // adds node to the tail
        this->tail = newNode;
    }
    this->queue_length += 1;

    pthread_mutex_unlock(&lock);

}

bool DoubleQueue::remove(double* ret) {
    pthread_mutex_lock(&lock);

    if (this->queue_length == 0) {
        pthread_mutex_unlock(&lock);
        return false;
    }
    *ret = (this->head)->value;      // store double value in return parameter
    QueueNode* nextNode = (this->head)->next;
    delete this->head;
    this->head = nextNode;
    this->queue_length -= 1;

    pthread_mutex_unlock(&lock);
    return true;
}

double DoubleQueue::wait_remove() {
    pthread_mutex_lock(&lock);
    double ele;

    while (this->queue_length == 0) {
        pthread_cond_wait(&cond, &lock);
    }

    ele = (this->head)->value;     
    QueueNode* nextNode = (this->head)->next;
    delete this->head;
    this->head = nextNode;
    this->queue_length -= 1;

    pthread_mutex_unlock(&lock);
    return ele;
}

int DoubleQueue::length() {
    return this->queue_length;
}