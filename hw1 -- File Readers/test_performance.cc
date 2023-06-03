/*
 * Copyright ©2023 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Pennsylvania
 * CIT 5950 for use solely during Spring Semester 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <time.h>  // POSIX
#include <cmath>

#include "gtest/gtest.h"
#include "./test_suite.h"

#include "./BufferedFileReader.h"
#include "./SimpleFileReader.h"


namespace hw1 {

class Test_Performance : public ::testing::Test {
 protected:
  // Code here will be called before each test case
  virtual void SetUp() {
    // Nothing
  }

  // These values contain the filename and file contents that we will
  // be using to test the simple file_reader
  static constexpr const char* kHelloFileName = "./test_files/Hello.txt";
  static constexpr const char* kByeFileName = "./test_files/Bye.txt";
  static constexpr const char* kLongFileName = "./test_files/war_and_peace.txt";
  static constexpr const char* kGreatFileName = "./test_files/mutual_aid.txt";

  // Code here will be called after each test executes (ie, after
  // each TEST_F)
  virtual void TearDown() {
    // Nothing as of now
  }

};  // class Test_BufferedFileReader

static uint64_t get_ms() {
  struct timespec spec;
  time_t seconds;
  uint64_t milli;

  clock_gettime(CLOCK_REALTIME, &spec);

  seconds  = spec.tv_sec;
  milli = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
  milli += seconds * 1000;
  return milli;
}


TEST_F(Test_Performance, Basic) {
  HW1Environment::OpenTestCase();
  BufferedFileReader bf(kLongFileName);
  SimpleFileReader sf(kLongFileName);
  char c;

  uint64_t start_time = get_ms();

  do {
    c = sf.get_char();
  } while (c != EOF);

  uint64_t end_time = get_ms();

  uint64_t simple_time = end_time - start_time;

  std::cout << "Time (ms) for SimpleFileReader to read \"War and Peace\": " << simple_time << std::endl;

  start_time = time(nullptr);

  do {
    c = bf.get_char();
  } while (c != EOF);
  end_time = time(nullptr);

  uint64_t buffered_time = end_time - start_time;

  std::cout << "Time (ms) for BufferedFileReader to read \"War and Peace\": " << buffered_time << std::endl;

  ASSERT_TRUE(buffered_time * 3 < simple_time);
  
  HW1Environment::AddPoints(5);
}


}  // namespace hw1

