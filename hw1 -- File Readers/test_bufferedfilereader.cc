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

#include "./BufferedFileReader.h"
#include "./BufferChecker.h"

#include <fstream>
#include <string>

using std::ifstream;
using std::string;

namespace hw1 {

class Test_BufferedFileReader : public ::testing::Test {
 protected:
  // Code here will be called before all tests execute (ie, before
  // any TEST_F).
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

  static string kHelloContents;
  static string kByeContents;
  static string kLongContents;
  static string kGreatContents;

  // Code here will be called after each test executes (ie, after
  // each TEST_F)
  virtual void TearDown() {
    // Nothing as of now
  }

};  // class Test_BufferedFileReader

// statics
string Test_BufferedFileReader::kHelloContents = "";
string Test_BufferedFileReader::kByeContents = "";
string Test_BufferedFileReader::kLongContents = "";
string Test_BufferedFileReader::kGreatContents = "";

// Helper Functions
static bool verify_token(const string& actual, const string& expected_contents, const string& delims, off_t* offset);
static bool verify_tokens(const string* actual, int len, const string& expected_contents, const string& delims, off_t* offset);

TEST_F(Test_BufferedFileReader, Basic) {
  HW1Environment::OpenTestCase();
  BufferedFileReader* bf = new BufferedFileReader(kHelloFileName);
  char c = bf->get_char();
  ASSERT_EQ('H', c);

  // Check that the buffer has 'h' in it
  BufferChecker bc(*bf);
  string expected("H");
  ASSERT_FALSE(bc.check_token_errors(expected, 0));

  HW1Environment::AddPoints(5);

  // Delete SF to make sure destructor works, then award points
  // if it doesn't crash
  delete bf;
  HW1Environment::AddPoints(5);
}

TEST_F(Test_BufferedFileReader, open_close) {
  HW1Environment::OpenTestCase();
  // close when already closed
  BufferedFileReader* bf = new BufferedFileReader(kHelloFileName);
  ASSERT_TRUE(bf->good());
  bf->close_file();
  ASSERT_FALSE(bf->good());
  bf->close_file();
  ASSERT_FALSE(bf->good());


  // open when already opened
  bf->open_file(kByeFileName);
  ASSERT_TRUE(bf->good());
  bf->open_file(kByeFileName);
  ASSERT_TRUE(bf->good());
  bf->open_file(kByeFileName);
  ASSERT_TRUE(bf->good());

  HW1Environment::AddPoints(5);

  // open and close the same file over and over again
  for (size_t i = 0; i < 10 ; i++) {
    bf->open_file(kByeFileName);
    ASSERT_TRUE(bf->good());
    bf->close_file();
    ASSERT_FALSE(bf->good());
    bf->close_file();
    ASSERT_FALSE(bf->good());
    bf->open_file(kByeFileName);
    ASSERT_TRUE(bf->good());
  }

  // destructor on an already closed file
  delete bf;

  HW1Environment::AddPoints(5);
}

TEST_F(Test_BufferedFileReader, get_char) {
  HW1Environment::OpenTestCase();

  // Hello test case
  BufferedFileReader bf(kHelloFileName);
  BufferChecker bc(bf);
  string contents;
  char c;
  for (size_t i = 0; i < kHelloContents.length(); i++) {
    ASSERT_EQ(i, bf.tell());
    c = bf.get_char();
    contents += c;
    ASSERT_TRUE(bf.good());
    ASSERT_FALSE(bc.check_char_errors(c, i));
  }
  ASSERT_EQ(kHelloContents, contents);
  c = bf.get_char();
  ASSERT_EQ(EOF, c);
  ASSERT_FALSE(bf.good());

  HW1Environment::AddPoints(5);

  // Goodbye test case
  bf.close_file();
  bf.open_file(kByeFileName);
  contents.clear();
  for (size_t i = 0; i < kByeContents.length(); i++) {
    ASSERT_EQ(i, bf.tell());
    c = bf.get_char();
    contents += c;
    ASSERT_TRUE(bf.good());
    ASSERT_FALSE(bc.check_char_errors(c, i));
  }
  ASSERT_EQ(kByeContents, contents);
  c = bf.get_char();
  ASSERT_EQ(EOF, c);
  ASSERT_FALSE(bf.good());

  HW1Environment::AddPoints(5);

  // Long file test case
  contents.clear();
  bf.close_file();
  bf.open_file(kLongFileName);
  contents.reserve(kLongContents.length());
  for (size_t i = 0; i < kLongContents.length(); i++) {
    ASSERT_EQ(i, bf.tell());
    c = bf.get_char();
    contents += c;
    ASSERT_TRUE(bf.good());
    ASSERT_FALSE(bc.check_char_errors(c, i));
  }
  ASSERT_EQ(kLongContents, contents);
  c = bf.get_char();
  ASSERT_EQ(EOF, c);
  ASSERT_FALSE(bf.good());

  HW1Environment::AddPoints(5);

  // "Great" file test case
  contents.clear();
  bf.close_file();
  bf.open_file(kGreatFileName);
  contents.reserve(kGreatContents.length());
  for (size_t i = 0; i < kGreatContents.length(); i++) {
    ASSERT_EQ(i, bf.tell());
    c = bf.get_char();
    contents += c;
    ASSERT_TRUE(bf.good());
    ASSERT_FALSE(bc.check_char_errors(c, i));
  }
  ASSERT_EQ(kGreatContents, contents);
  c = bf.get_char();
  ASSERT_EQ(EOF, c);
  ASSERT_FALSE(bf.good());

  HW1Environment::AddPoints(5);
}

TEST_F(Test_BufferedFileReader, get_token) {
  HW1Environment::OpenTestCase();
  string token;
  string delims = "\t ";
  off_t offset = 0;

  BufferedFileReader bf(kHelloFileName, delims);
  BufferChecker bc(bf);

  while (bf.good()) {
    token = bf.get_token();
    ASSERT_FALSE(bc.check_token_errors(token, offset));
    ASSERT_TRUE(verify_token(token, kHelloContents, delims, &offset));
    ASSERT_EQ(offset, bf.tell());
  }

  ASSERT_EQ(kHelloContents.length(), offset);

  HW1Environment::AddPoints(10);

  offset = 0;
  bf.open_file(kLongFileName);
  while (bf.good()) {
    token = bf.get_token();
    ASSERT_FALSE(bc.check_token_errors(token, offset));
    ASSERT_TRUE(verify_token(token, kLongContents, delims, &offset));
    ASSERT_EQ(offset, bf.tell());
  }

  ASSERT_EQ(kLongContents.length(), offset);

  HW1Environment::AddPoints(15);
}

TEST_F(Test_BufferedFileReader, get_line) {
  HW1Environment::OpenTestCase();
  const string *tokens;
  string delims = ",\t ";
  int len = 0;
  off_t offset = 0;

  BufferedFileReader bf(kByeFileName, delims);
  BufferChecker bc(bf);

  while (bf.good()) {
    tokens = bf.get_line(&len);
    ASSERT_TRUE(verify_tokens(tokens, len, kByeContents, delims, &offset));
    if (len > 0) {
      ASSERT_FALSE(bc.check_token_errors(tokens[len-1], offset - (tokens[len-1].length()+1)));
    }
    ASSERT_EQ(offset, bf.tell());
    delete[] tokens;
  }

  ASSERT_EQ(kByeContents.length(), offset);

  HW1Environment::AddPoints(10);

  offset = 0;
  bf.open_file(kGreatFileName);

  while (bf.good()) {
    tokens = bf.get_line(&len);

    ASSERT_TRUE(verify_tokens(tokens, len, kGreatContents, delims, &offset));
    if (len > 0) {
      ASSERT_FALSE(bc.check_token_errors(tokens[len-1], offset - (tokens[len-1].length()+1)));
    }
    ASSERT_EQ(offset, bf.tell());
    delete[] tokens;
  }

  ASSERT_EQ(kGreatContents.length(), offset);

  HW1Environment::AddPoints(15);
}

TEST_F(Test_BufferedFileReader, Complex) {
  HW1Environment::OpenTestCase();

  const string *tokens;
  string token;
  string delims = ",\n ";
  int len = 0;
  off_t offset = 0;
  bool read_line = false;

  BufferedFileReader bf(kLongFileName, delims);
  BufferChecker bc(bf);

  for (int i = 0; i < 3; i++) {
    while (bf.good()) {
      if (read_line) {
        tokens = bf.get_line(&len);
        ASSERT_TRUE(verify_tokens(tokens, len, kLongContents, delims, &offset));
        if (len > 0) {
          ASSERT_FALSE(bc.check_token_errors(tokens[len-1], offset - (tokens[len-1].length()+1)));
        }
        ASSERT_EQ(offset, bf.tell());
        delete[] tokens;
      } else {
        token = bf.get_token();
        ASSERT_FALSE(bc.check_token_errors(token, offset));
        ASSERT_TRUE(verify_token(token, kLongContents, delims, &offset));
        ASSERT_EQ(offset, bf.tell());
      }
      read_line = !read_line;
    }
    ASSERT_EQ(kLongContents.length(), offset);
    offset = 0;
    bf.rewind();
  }

  HW1Environment::AddPoints(25);
}

static bool verify_token(const string& actual, const string& expected_contents, const string& delims, off_t *offset) {
  off_t off = *offset;
  string expected = expected_contents.substr(off, actual.length());
  if (actual != expected) {
    *offset = off;
    return false;
  }

  off += actual.length(); 
  if (off >= static_cast<off_t>(expected_contents.length())) {
    // eof reached
    *offset = off;
    return true;
  }

  if (delims.find(expected_contents[off]) == string::npos) {
    *offset = off + 1;
    return false;
  }

  off++; 

  *offset = off;
  return true;
}

// Ensures that the last token has a new line character after it
static bool verify_tokens(const string* actual, int len, const string& expected_contents, const string& delims, off_t *offset) {
  if (len <= 0) {
    if (static_cast<size_t>(*offset) < expected_contents.length() && expected_contents[*offset] == '\n') {
      *offset += 1; // for \n since this is a blank line
    }
    return true;
  }
  for (int i = 0; i < len-1; i++) {
    if (!verify_token(actual[i], expected_contents, delims, offset)) {
      return false;
    }
  }
  if (!verify_token(actual[len-1], expected_contents, string("\n"), offset)) {
    return false;
  }
  
  return true;
}

}  // namespace hw1
