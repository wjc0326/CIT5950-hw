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

#include "./SimpleFileReader.h"

SimpleFileReader::SimpleFileReader(const string& fname) {
    this->fd_ = open(fname.c_str(), O_RDONLY);
    if (this->fd_ == -1) {
        perror("open failed");
        good_ = false;                  // no file open
        exit(EXIT_FAILURE);
    }
    lseek(this->fd_, 0, SEEK_SET);      // read from the start of the file
    this->good_ = true;
}

SimpleFileReader::~SimpleFileReader() {
    close(this->fd_);
    this->fd_ = -1;                     // indicates no file open
    this->good_ = false;                // no file open
}

void SimpleFileReader::open_file(const string& fname) {
    //if the object is already managing a file, that file is closed.
    if (this->fd_ != -1) {
        close(this->fd_);
        this->fd_ = -1;                     // indicates no file open
        this->good_ = false;                // no file open
    }
    this->fd_ = open(fname.c_str(), O_RDONLY);
    if (this->fd_ == -1) {
        perror("open failed");
        good_ = false;                  // no file open
        exit(EXIT_FAILURE);
    }
    lseek(this->fd_, 0, SEEK_SET);      // read from the start of the file
    this->good_ = true;
}

void SimpleFileReader::close_file() {
    // If there is not a file currently open, then nothing should happen.
    if (this->fd_ == -1) {
        return;
    }
    close(this->fd_);
    this->fd_ = -1;                     // indicates no file open
    this->good_ = false;                // no file open
}

char SimpleFileReader::get_char() {
    char char_;
    int result;

    // If there is no file open currently, then EOF is returned.
    if (this->fd_ == -1) {
        this->good_ = false;
        return EOF;
    }
    result = read(this->fd_, &char_, 1);
    if (result == -1) {
        if (errno != EINTR) {
            perror("read failed");
            this->good_ = false;
            exit(EXIT_FAILURE);
        }
    } else if (result == 0) {
        this->good_ = false;        // If at the end of the file, then EOF is returned.
        return EOF;
    }

    this->good_ = true;
    return char_;
}

string SimpleFileReader::get_chars(size_t n) {  
    char* buf = new char[n+1];
    int result;

    if (this->fd_ == -1) {
        this->good_ = false;
        delete[] buf;
        exit(EXIT_FAILURE);
    }

    char* ptr = buf;
    while (ptr < buf + n) {
        result = read(this->fd_, ptr, buf + n - ptr);
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
    }

    int bytes_left = buf + n - ptr;
    buf[n - bytes_left] = '\0';
    string res = buf;
    delete[] buf;
    return res;
}

int SimpleFileReader::tell() {
    return lseek(this->fd_, 0, SEEK_CUR);
}

void SimpleFileReader::rewind() {
    lseek(this->fd_, 0, SEEK_SET);      // read from the start of the file
    this->good_ = true;
}

bool SimpleFileReader::good() {
    return this->good_;
}