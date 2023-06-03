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

#include <unistd.h>    // for fork()
#include <sys/types.h> // for pid_t
#include <sys/wait.h>  // for wait(), waitpid(), etc.
#include <boost/algorithm/string.hpp> // for split(), trim(), etc.

#include <cstdlib>  // for exit(), EXIT_SUCCESS, and EXIT_FAILURE
#include <iostream>
#include <string>
#include <vector>

using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::string;
using std::getline;
using std::vector;

int main() {
  // ask for user's prompt until EOF or "exit"
  string line;

  while(true) {
    cout << "$ ";
    // if reaching EOF, break
    if(!getline(cin, line)) {
      if(!cin.eof()){
        cerr << "Warning: fatal error while reading input from user" << endl;
      }
      cout << "" << endl;
      break;
    }
    // if user choose to exit the program, break
    if (line == "exit") {
      break;
    }
    
    // TODO: deal with several programs in a single cmd
    boost::algorithm::trim(line);
    vector<string> commands;
    boost::split(commands, line, boost::is_any_of("|"), boost::token_compress_on);

    int process_num = commands.size();
    int in_pipe[2];
    int out_pipe[2];

    for (int i = 0; i < process_num; i++) {
      if ((process_num != 1) && (i != (process_num - 1))) {
        // create pipe
        if (pipe(out_pipe) == -1) {
          cerr << "UNABLE TO CREATE PIPE" << endl;
          return EXIT_FAILURE;
        }
      }

      pid_t pid = fork();

      if (pid == -1) {
        cerr << "UNABLE TO FORK" << endl;
        return EXIT_FAILURE;
      } else if (pid == 0) {
        // child
        if (process_num != 1) {
          if (i == 0) {       
            // FIRST
            close(out_pipe[0]);    // close read end
            dup2(out_pipe[1], STDOUT_FILENO);  // replace stdout with write end of pipe
            close(out_pipe[1]);    // close write end since it has been duplicated
          } else if (i == (process_num - 1)) {      
            // LAST
            close(in_pipe[1]);    // close read end
            dup2(in_pipe[0], STDIN_FILENO);  // replace stdin with read end of pipe
            close(in_pipe[0]);    // close write end since it has been duplicated
          } else {            
            // MIDDLE
            close(out_pipe[0]);    // close read end
            dup2(in_pipe[0], STDIN_FILENO);  // replace stdin with read end of pipe
            close(in_pipe[0]);    // close write end since it has been duplicated

            dup2(out_pipe[1], STDOUT_FILENO);  // replace stdout with write end of pipe
            close(out_pipe[1]);    // close write end since it has been duplicated
          }
        }
      
        string cmd = commands[i];
        boost::algorithm::trim(cmd);
        vector<string> cmd_ele;     // "cmd_ele" store the elements of single cmd (e.g. ls, ./test_files/)
        boost::split(cmd_ele, cmd, boost::is_any_of(" "), boost::token_compress_on);

        int cmd_size = cmd_ele.size();

        char** args = new char*[cmd_size + 1];
        for (int j = 0; j < cmd_size; j++) {
          args[j] = const_cast<char*>(cmd_ele[j].c_str());
        }
        args[cmd_size] = nullptr;   // null terminate args array

        execvp(args[0], args);
        cerr << strerror(errno) << endl;
        // Exec didn't work, so an error must have been encountered
        delete[] args;
        return EXIT_FAILURE;
      } else {
        // parent
        if (commands.size() != 1) {
          if (i == 0) {
            // FIRST
            close(out_pipe[1]);
          } else if (i == (process_num - 1)) {
            // LAST
            close(in_pipe[0]);    // close read end
          } else {
            // MIDDLE
            close(in_pipe[0]);    // close write end
            close(out_pipe[1]);
          }
        }
        // the in_pipe of next program is the out_pipe of current program
        in_pipe[0] = out_pipe[0];
        in_pipe[1] = out_pipe[1];     
      }
    }

    int children_size = commands.size();
    for (int k = children_size; k > 0; k--) {
      int is_finished;
      pid_t pid = wait(&is_finished);
      if (pid == -1) {
        cerr << "Error" << endl;
        exit(EXIT_FAILURE);
      }
    }
  }
  return EXIT_SUCCESS;
}