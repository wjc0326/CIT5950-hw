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

#include "gtest/gtest.h"
#include "./test_suite.h"

#include "./SimpleFileReader.h"

#include <fstream>
#include <string>

using std::ifstream;
using std::string;

namespace hw1 {

class Test_SimpleFileReader : public ::testing::Test {
 protected:
  // Code here will be called before each test case executes
  virtual void SetUp() {
    // Re-read the file content for each file
    ifstream hello_ifs(kHelloFileName);
    kHelloContents.assign((std::istreambuf_iterator<char>(hello_ifs)),
                          (std::istreambuf_iterator<char>()));
    ifstream bye_ifs(kByeFileName);
    kByeContents.assign((std::istreambuf_iterator<char>(bye_ifs)),
                          (std::istreambuf_iterator<char>()));
    ifstream long_ifs(kLongFileName);
    kLongContents.assign((std::istreambuf_iterator<char>(long_ifs)),
                          (std::istreambuf_iterator<char>()));
    ifstream great_ifs(kGreatFileName);
    kGreatContents.assign((std::istreambuf_iterator<char>(great_ifs)),
                          (std::istreambuf_iterator<char>()));

  }

  // These values contain the filename and file contents that we will
  // be using to test the simple file_reader
  static constexpr const char* kHelloFileName = "./test_files/Hello.txt";
  static constexpr const char* kByeFileName = "./test_files/Bye.txt";
  static constexpr const char* kLongFileName = "./test_files/war_and_peace.txt";
  static constexpr const char* kGreatFileName = "./test_files/mutual_aid.txt";

  static string kHelloContents, kByeContents, kLongContents, kGreatContents;

  // Code here will be called after each test executes (ie, after
  // each TEST_F)
  virtual void TearDown() {
    // Nothing as of now
  }

};  // class Test_SimpleFileReader

// statics
string Test_SimpleFileReader::kHelloContents;
string Test_SimpleFileReader::kByeContents;
string Test_SimpleFileReader::kLongContents;
string Test_SimpleFileReader::kGreatContents;

TEST_F(Test_SimpleFileReader, Basic) {
  HW1Environment::OpenTestCase();
  SimpleFileReader* sf = new SimpleFileReader(kHelloFileName);
  char c = sf->get_char();
  ASSERT_EQ('H', c);
  HW1Environment::AddPoints(5);

  // Delete SF to make sure destructor works, then award points
  // if it doesn't crash
  delete sf;
  HW1Environment::AddPoints(5);
}

TEST_F(Test_SimpleFileReader, open_close) {
  HW1Environment::OpenTestCase();
  // TODO
  // close when already closed
  SimpleFileReader* sf = new SimpleFileReader(kHelloFileName);
  ASSERT_EQ(true, sf->good());
  sf->close_file();
  ASSERT_EQ(false, sf->good());
  sf->close_file();
  ASSERT_EQ(false, sf->good());


  // open when already opened
  sf->open_file(kByeFileName);
  ASSERT_EQ(true, sf->good());
  sf->open_file(kByeFileName);
  ASSERT_EQ(true, sf->good());
  sf->open_file(kByeFileName);
  ASSERT_EQ(true, sf->good());

  HW1Environment::AddPoints(5);

  // open and close the same file over and over again
  for (size_t i = 0; i < 10 ; i++) {
    sf->open_file(kByeFileName);
    ASSERT_EQ(true, sf->good());
    sf->close_file();
    ASSERT_EQ(false, sf->good());
    sf->close_file();
    ASSERT_EQ(false, sf->good());
    sf->open_file(kByeFileName);
    ASSERT_EQ(true, sf->good());
  }

  // destructor on an already closed file
  delete sf;

  HW1Environment::AddPoints(5);
}

TEST_F(Test_SimpleFileReader, get_char) {
  HW1Environment::OpenTestCase();

  // Hello test case
  SimpleFileReader sf(kHelloFileName);
  string contents;
  char c;
  for (size_t i = 0; i < kHelloContents.length(); i++) {
    ASSERT_EQ(i, sf.tell());
    c = sf.get_char();
    contents += c;
    ASSERT_EQ(true, sf.good());
  }
  ASSERT_EQ(kHelloContents, contents);
  c = sf.get_char();
  ASSERT_EQ(EOF, c);
  ASSERT_EQ(false, sf.good());

  HW1Environment::AddPoints(5);

  // Goodbye test case
  sf.close_file();
  sf.open_file(kByeFileName);
  contents.clear();
  for (size_t i = 0; i < kByeContents.length(); i++) {
    ASSERT_EQ(i, sf.tell());
    c = sf.get_char();
    contents += c;
    ASSERT_EQ(true, sf.good());
  }
  ASSERT_EQ(kByeContents, contents);
  c = sf.get_char();
  ASSERT_EQ(EOF, c);
  ASSERT_EQ(false, sf.good());

  HW1Environment::AddPoints(5);

  // Long file test case
  contents.clear();
  sf.close_file();
  sf.open_file(kLongFileName);
  contents.reserve(kLongContents.length());
  for (size_t i = 0; i < kLongContents.length(); i++) {
    ASSERT_EQ(i, sf.tell());
    c = sf.get_char();
    contents += c;
    ASSERT_EQ(true, sf.good());
  }
  ASSERT_EQ(kLongContents, contents);
  c = sf.get_char();
  ASSERT_EQ(EOF, c);
  ASSERT_EQ(false, sf.good());

  HW1Environment::AddPoints(5);

  // "Great" file test case
  contents.clear();
  sf.close_file();
  sf.open_file(kGreatFileName);
  contents.reserve(kGreatContents.length());
  for (size_t i = 0; i < kGreatContents.length(); i++) {
    ASSERT_EQ(i, sf.tell());
    c = sf.get_char();
    contents += c;
    ASSERT_EQ(true, sf.good());
  }
  ASSERT_EQ(kGreatContents, contents);
  c = sf.get_char();
  ASSERT_EQ(EOF, c);
  ASSERT_EQ(false, sf.good());

  HW1Environment::AddPoints(5);
}

TEST_F(Test_SimpleFileReader, get_chars) {
  HW1Environment::OpenTestCase();

  // Hello test case
  SimpleFileReader sf(kHelloFileName);
  string contents;
  string next;
  size_t n = 0;

  for (size_t i = 0; i < kHelloContents.length(); i += n, n++) {
    next = sf.get_chars(n);
    contents += next;
    if ( i + n <= kHelloContents.length() ) {
      ASSERT_TRUE(sf.good());
    } else {
      ASSERT_FALSE(sf.good());
    }
  }
  ASSERT_EQ(kHelloContents, contents);
  next = sf.get_chars(1);
  ASSERT_EQ(string(""), next);
  ASSERT_EQ(false, sf.good());

  HW1Environment::AddPoints(5);

  // Bye test case
  contents.clear();
  sf.close_file();
  sf.open_file(kByeFileName);
  for (size_t i = 0; i < kByeContents.length(); i += n, n++) {
    next = sf.get_chars(n);
    contents += next;
    if ( i + n <= kByeContents.length() ) {
      ASSERT_TRUE(sf.good());
    } else {
      ASSERT_FALSE(sf.good());
    }
  }
  ASSERT_EQ(kByeContents, contents);
  next = sf.get_chars(1);
  ASSERT_EQ(string(""), next);
  ASSERT_EQ(false, sf.good());

  HW1Environment::AddPoints(5);
  
  // Long test case
  contents.clear();
  sf.close_file();
  sf.open_file(kLongFileName);
  for (size_t i = 0; i < kLongContents.length(); i += n, n++) {
    next = sf.get_chars(n);
    contents += next;
    if ( i + n <= kLongContents.length() ) {
      ASSERT_TRUE(sf.good());
    } else {
      ASSERT_FALSE(sf.good());
    }
  }

  ASSERT_EQ(kLongContents, contents);
  
  
  next = sf.get_chars(1);
  ASSERT_EQ(string(""), next);
  ASSERT_EQ(false, sf.good());

  HW1Environment::AddPoints(5);

  // Great test case
  contents.clear();
  sf.close_file();
  sf.open_file(kGreatFileName);
  for (size_t i = 0; i < kGreatContents.length(); i += n, n++) {
    next = sf.get_chars(n);
    contents += next;
    if ( i + n <= kGreatContents.length() ) {
      ASSERT_TRUE(sf.good());
    } else {
      ASSERT_FALSE(sf.good());
    }
  }
  ASSERT_EQ(kGreatContents, contents);
  next = sf.get_chars(1);
  ASSERT_EQ(string(""), next);
  ASSERT_EQ(false, sf.good());

  HW1Environment::AddPoints(5);
}


TEST_F(Test_SimpleFileReader, Complex) {
  HW1Environment::OpenTestCase();

  // Hello test case
  SimpleFileReader sf(kHelloFileName);
  string contents;
  string next;
  char c;
  size_t n = 0;

  for (size_t i = 0; i < 3; i++) {
    for (size_t j = 0; j < kHelloContents.length(); n++) {
      if (j % 2) {
        c = sf.get_char();
        contents += c;
        j++;
      } else {
        next = sf.get_chars(n);
        contents += next;
        j+=n;
      }
      if ( j <= kHelloContents.length() ) {
        ASSERT_TRUE(sf.good());
      } else {
        ASSERT_FALSE(sf.good());
      }
    }
    ASSERT_EQ(kHelloContents, contents);
    c = sf.get_char();
    ASSERT_EQ(EOF, c);
    ASSERT_EQ(false, sf.good());
    sf.rewind();
    contents.clear();
    HW1Environment::AddPoints(5);
  }

  // Great test case
  
  // clear previous contents of "contents"
  contents.clear();

  // open and close repeatedly
  sf.close_file();
  sf.close_file();
  sf.close_file();
  sf.open_file(kGreatFileName);
  sf.close_file();
  sf.open_file(kGreatFileName);
  sf.close_file();
  sf.open_file(kGreatFileName);

  n = 0;
  for (size_t i = 0; i < kGreatContents.length(); n++) {
    if (!(i % 2)) {
      c = sf.get_char();
      contents += c;
      i++;
    } else {
      next = sf.get_chars(n);
      contents += next;
      i+=n;
    }
    if ( i <= kGreatContents.length() ) {
      ASSERT_TRUE(sf.good());
    } else {
      ASSERT_FALSE(sf.good());
    }
  }
  ASSERT_EQ(kGreatContents, contents);
  next = sf.get_chars(1);
  ASSERT_EQ(string(""), next);
  ASSERT_EQ(false, sf.good());

  HW1Environment::AddPoints(15);
}

}  // namespace hw1

