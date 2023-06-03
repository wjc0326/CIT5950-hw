#include <iostream>
#include <iomanip>
#include <limits>
#include <ios>
#include <unistd.h>
#include <pthread.h>
#include "./DoubleQueue.h"

using std::cout;
using std::endl;
using std::fixed;
using std::cin;
using std::setprecision;
using std::numeric_limits;
using std::streamsize;

// globals
DoubleQueue* dq;
pthread_mutex_t lock;
bool is_EOF = false;

// reads the global array nums
// and prints out statistics on it
// if there are more than 5 elements in the
// array, then the array is truncated down to
// 5 elements
// The array should never exceed 6 elements
void* print_nums_thread(void* arg) {
  double nums[6];
  int len = 0;

  while(true) {

    // read from the double queue and store it into the local array
    double read_in_double;
    while (!(dq->remove(&read_in_double))) {
      pthread_mutex_lock(&lock);
      if (is_EOF) {
        pthread_mutex_unlock(&lock);
        pthread_exit(nullptr);
      }
      pthread_mutex_unlock(&lock);
    }
    nums[len] = read_in_double;
    len++;

    if (len > 5) {
      for (int i = 0; i < 5; i++) {
        nums[i] = nums[i+1];
      }
      len--;
    }

    double min = nums[0];
    double max = nums[0];
    double sum = nums[0];
    for (int i = 1; i < len; i++) {
      sum += nums[i];
      if (min > nums[i]) {
        min = nums[i];
      }
      if (max < nums[i]) {
        max = nums[i];
      }
    }
    double avg = sum / len;

    cout << setprecision(2) << std::fixed;
    cout << "Max: " << max << endl;
    cout << "Min: " << min << endl;
    cout << "Average: " << avg << endl;
    cout << "Last five: ";

    for ( int i = 0; i < len; i++) {
      cout << nums[i] << " ";
    }
    cout << endl;
  }

  pthread_mutex_unlock(&lock);
  pthread_exit(nullptr);
}


// Reads a number from the console
// and adds it to the global array called "nums".
//
// returns false when the EOF (ctrl + d)
// is read. Meaning that the overall
// program should now terminate..
// returns true if the reading was successful
void* read_nums_thread(void* arg) {
  while (true) {
    double value;
    cin >> value;
    while (!cin) {
      // if an input that was passed in to cause reading to fail
      // then check for EOF, otherwise ignore a charcter of input
      // and try again
      if (cin.eof()) {
        pthread_mutex_lock(&lock);
        is_EOF = true;
        pthread_mutex_unlock(&lock);
        pthread_exit(nullptr);
      }

      cin.clear(); // clear error flags

      // ignore error unput that would cause the error.
      cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

      // try reading value again
      cin >> value;
    }

    // add the input double to double queue
    dq->add(value);
    sleep(1);
  }
  pthread_exit(nullptr);
}

int main() {
  dq = new DoubleQueue();
  pthread_t thd_read, thd_print;

  pthread_mutex_init(&lock, nullptr);

  pthread_create(&thd_read, nullptr, read_nums_thread, nullptr);
  pthread_create(&thd_print, nullptr, print_nums_thread, nullptr);

  pthread_join(thd_read,nullptr);
  pthread_join(thd_print,nullptr);

  pthread_mutex_destroy(&lock);

  delete dq;
  return EXIT_SUCCESS;
}