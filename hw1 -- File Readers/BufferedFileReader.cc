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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>
#include "BufferedFileReader.h"

BufferedFileReader::BufferedFileReader(const string& fname, const string& delims) {
    this->fd_ = open(fname.c_str(), O_RDONLY);
    if (this->fd_ == -1) {
        perror("open failed");
        this->good_ = false;                  // no file open
        exit(EXIT_FAILURE);
    }
    lseek(this->fd_, 0, SEEK_SET);      // read from the start of the file
    this->good_ = true;

    this->delims_ = delims;
    this->curr_length_ = 0;
    this->curr_index_ = 0;
    this->fill_num_ = 0;
    fill_buffer();
}

BufferedFileReader::~BufferedFileReader() {
    close(this->fd_);
    this->fd_ = -1;                     // indicates no file open
    this->good_ = false;                // no file open
    this->curr_index_ = 0;
    this->curr_length_ = 0;
}

void BufferedFileReader::open_file(const string& fname) {
    //if the object is already managing a file, that file is closed.
    if (this->fd_ != -1) {
        close_file();
    }
    this->fd_ = open(fname.c_str(), O_RDONLY);
    if (this->fd_ == -1) {
        perror("open failed");
        this->good_ = false;                  // no file open
        exit(EXIT_FAILURE);
    }
    lseek(this->fd_, 0, SEEK_SET);      // read from the start of the file
    this->good_ = true;
    fill_buffer();
}

void BufferedFileReader::close_file() {
    close(this->fd_);
    this->fd_ = -1;                     // indicates no file open
    this->good_ = false;                // no file open
    this->fill_num_ = 0;
    this->curr_index_ = 0;
    this->curr_length_ = 0;
}

char BufferedFileReader::get_char() {
    char char_;

    // If there is no file open currently, then EOF is returned.
    if (this->fd_ == -1) {
        this->good_ = false;
        return EOF;
    }

    if (curr_index_ != curr_length_) {      // hasn't arrived at the end of buffer
        char_ = buffer_[curr_index_];
        curr_index_++;
    } else {
        if (curr_length_ != BUF_SIZE) {
            this->good_ = false;        
            return EOF;
        }
        fill_buffer();
        char_ = buffer_[curr_index_];
        curr_index_++;
    }

    return char_;
}

string BufferedFileReader::get_token() {
    string token = "";
    while (true) {
        char c = get_char();
        if (is_delim(c) || c == -1) {
            break;
        }
        token += c;
    }
    return token;
}

string* BufferedFileReader::get_line(int* len) {
    string* line = new string[50];
    *len = 0;
    
    string token = "";
    while (true) {
        char c = get_char();
        if (c == '\n' || c == -1) {
            line[*len] = token;
            token = "";
            *len += 1;
            break;
        } else if (is_delim(c)) {       // add token to the heap
            line[*len] = token;
            token = "";
            *len += 1;
            continue;
        }
        token += c;
    }

    string* token_line = new string[*len + 1];
    for (int i = 0; i < *len; i++) {
        token_line[i] = line[i];
    }

    delete[] line;
    return token_line;
}

int BufferedFileReader::tell() {
    return (fill_num_ - 1) * BUF_SIZE + curr_index_;
}

void BufferedFileReader::rewind() {
    // If there is no file open currently, then return directly
    if(fd_ == -1){ 
        this->good_=false;
        exit(EXIT_FAILURE);
      }   
    lseek(fd_, 0, SEEK_SET);   //reset to index 0 of file 
    this->good_ = true;
    fill_num_ = 0;
    fill_buffer();
}

bool BufferedFileReader::good() {
    return this->good_;
}

void BufferedFileReader::fill_buffer() {
    curr_length_ = 0;
    int result;

    // If there is no file open currently, then return directly
    if (this->fd_ == -1) {
        this->good_ = false;
        exit(EXIT_FAILURE);
    }

    char* ptr = buffer_;
    while (ptr < buffer_ + BUF_SIZE) {
        result = read(this->fd_, ptr, buffer_ + BUF_SIZE - ptr);
        if (result == -1) {
            if (errno != EINTR) {
                perror("read failed");
                this->good_ = false;
                exit(EXIT_FAILURE);
            }
            continue;       // EINTR happened, so do nothing and try again
        } else if (result == 0) {
            this->good_ = false;        // If at the end of the file, good_ is false.
            break;
        }
        this->good_ = true;
        ptr += result;
        curr_length_ += result;
    }
    curr_index_ = 0;

    if (curr_length_ != BUF_SIZE) {
        buffer_[curr_length_] = -1;           // mark for end of buffer read
    }
    if (curr_length_ == 0) {        // nothing read into the buffer, at the end of the file
        this->good_ = false;
        return;
    }

    this->good_ = true;
    fill_num_++;
    return;
}

bool BufferedFileReader::is_delim(char c) {
    std::size_t found = delims_.find(c);        // found is the position of the first character of the first match
    if (found != std::string::npos) {             // there is a match
        return true;
    } else {
        return false;
    }
}

