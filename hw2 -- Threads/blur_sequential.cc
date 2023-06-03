#include <iostream>
#include <string>
#include <cstdlib>
#include "qdbmp.h"

using std::cerr;
using std::endl;

int main(int argc, char** argv) {
  // TODO: write your sequential version of the blur program in this file. 
  // You can mimic negative.cc to get started.

  // Check input commands
  if (argc != 4) {
    cerr << "Usage: " << argv[0] << " <input file> <output file> <block_size>" << endl;
    return EXIT_FAILURE;
  }

  // Check the box size
  if (argv[3] <= 0) {
    cerr << "Error: The box size cannot be 0 or negative" << endl;
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

  // Loop through each pixel and turn into blur
  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      int num_block = 0;
      int total_r = 0;
      int total_g = 0;
      int total_b = 0;
      size_t box_size = std::stoi(argv[3]);

      for (int row = std::max((int)(y-box_size), static_cast<int>(0)); 
           row <= std::min((int)(y+box_size), static_cast<int>(height-1)); ++row) {
        for (int col = std::max((int)(x-box_size), static_cast<int>(0)); 
             col <= std::min((int)(x+box_size), static_cast<int>(width-1)); ++col) {
          // if (row < 0 || row > height || col < 0 || col > width) {
          //   continue;
          // }
          num_block += 1;
          // Read the current pixel RGB color
          RGB cur_color = image.get_pixel(col, row);
          total_r += (int)cur_color.r;
          total_g += (int)cur_color.g;
          total_b += (int)cur_color.b;
        }
      }

      // Calculate the blur RGB color
      RGB blur_color((int)total_r/num_block, (int)total_g/num_block, (int)total_b/num_block);

      // Set the blur color
      blur.set_pixel(x, y, blur_color);
    }
  }

  // Output the blur image to disk
  blur.write_file(argv[2]);

  if (image.check_error() != BMP_OK) {
    perror("ERROR: Failed to open BMP file.");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
