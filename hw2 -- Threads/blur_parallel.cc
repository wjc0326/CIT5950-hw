#include <iostream>
#include <string>
#include <cstdlib>
#include <pthread.h>
#include "qdbmp.h"

using std::cerr;
using std::endl;

// struct to pass arguments to thread_main
struct thd_arg {
  size_t left_bound;
  size_t right_bound;
  size_t upper_bound;
  size_t lower_bound;
  size_t box_size; 
  BitMap* image;
  BitMap* blur;
};

// blur the original image in an atomic fashion
void* thread_main(void* arg) {
  // cast arguments back into struct
  struct thd_arg* a = reinterpret_cast<struct thd_arg*>(arg);

  // Loop through each pixel and turn into blur
  for (size_t y = a->lower_bound; y < a->upper_bound; ++y) {
    for (size_t x = a->left_bound; x < a->right_bound; ++x) {
      int num_block = 0;
      int total_r = 0;
      int total_g = 0;
      int total_b = 0;
      const unsigned int height = (*a->image).height();
      const unsigned int width = (*a->image).width();
    
      for (int row = std::max((int)(y-(a->box_size)), static_cast<int>(0)); 
           row <= std::min((int)(y+(a->box_size)), static_cast<int>(height-1)); ++row) {
        for (int col = std::max((int)(x-(a->box_size)), static_cast<int>(0)); 
             col <= std::min((int)(x+(a->box_size)), static_cast<int>(width-1)); ++col) {
          num_block += 1;
          // Read the current pixel RGB color
          RGB cur_color = (*a->image).get_pixel(col, row);
          total_r += (int)cur_color.r;
          total_g += (int)cur_color.g;
          total_b += (int)cur_color.b;
        }
      }

      // Calculate the blur RGB color
      RGB blur_color((int)total_r/num_block, (int)total_g/num_block, (int)total_b/num_block);

      // Set the blur color
      (*a->blur).set_pixel(x, y, blur_color);
    }
  }
  // delete dynamically-allocated struct
  delete a;
  return nullptr;  // return type is a pointer
}


int main(int argc, char** argv) {
  // TODO: write your parallel version of the blur program in this file. 
  // You should write your sequential version first

  // Check input commands
  if (argc != 5) {
    cerr << "Usage: " << argv[0] << " <input file> <output file> <block_size> <thread_count>" << endl;
    return EXIT_FAILURE;
  }

  // Check the box size
  if (argv[3] <= 0) {
    cerr << "Error: The box size cannot be 0 or negative" << endl;
    return EXIT_FAILURE;
  }

  // Check the number of threads
  if (argv[4] <= 0) {
    cerr << "Error: The number of threads cannot be 0 or negative" << endl;
    return EXIT_FAILURE;
  }  

  // Construct a BitMap object using the input file specified
  BitMap image(argv[1]);

  // Check the command above succeed
  if (image.check_error() != BMP_OK) {
    perror("ERROR: Failed to open BMP file.");
    return EXIT_FAILURE;
  }

  // Create a new BitMap for output the blur image
  const unsigned int height = image.height();
  const unsigned int width = image.width();
  BitMap blur(width, height);

  // Check the command above succeed
  if (image.check_error() != BMP_OK) {
    perror("ERROR: Failed to open BMP file.");
    return EXIT_FAILURE;
  }
  
  int box_size = (int)std::stoi(argv[3]);
  int num_threads = (int)std::stoi(argv[4]);
  pthread_t* thds = new pthread_t[num_threads];  // array of thread ids 
  int col_num = (int)width/num_threads;

  // create threads to run thread_main() (exclude the last thread)
  for (int i = 0; i < (num_threads - 1); i++) {
    struct thd_arg* args = new struct thd_arg;
    args->left_bound = i * col_num;
    args->right_bound = (i + 1) * col_num;      // in thread: x < a->right_bound
    args->lower_bound = 0;
    args->upper_bound = height;
    args->box_size = box_size;
    args->image = &image;
    args->blur = &blur;

    if (pthread_create(&thds[i], nullptr, &thread_main, args) != 0) {
      cerr << "pthread_create failed" << endl;
    }
  }

  // the last thread
  struct thd_arg* args = new struct thd_arg;
  args->left_bound = (num_threads - 1) * col_num;
  args->right_bound = width;      // the last thread should cover the remaining cols
  args->lower_bound = 0;
  args->upper_bound = height;
  args->box_size = box_size;
  args->image = &image;
  args->blur = &blur;

  if (pthread_create(&thds[num_threads - 1], nullptr, &thread_main, args) != 0) {
    cerr << "pthread_create failed" << endl;
  }

  // wait for all child threads to finish
  // (children may terminate out of order, but cleans up in order)
  for (int i = 0; i < num_threads; i++) {
    if (pthread_join(thds[i], nullptr) != 0) {
      cerr << "pthread_join failed" << endl;
    }
  }

  delete[] thds;

  // Output the blur image to disk
  blur.write_file(argv[2]);

  if (image.check_error() != BMP_OK) {
    perror("ERROR: Failed to open BMP file.");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}