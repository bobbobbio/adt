#include <stdtyp/subprocess.h>
#include <stdtyp/file.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

create_error_body(subprocess_error);


struct error
subprocess_run(const struct string *command, struct string *output)
{
   create(string_vec, arg_vec);
   string_tokenize(command, &arg_vec);
   create_cstr_array(argv, &arg_vec);
   // Creates 2 file descriptors that are linked together writes to one go to
   // the other:
   // write_pipe ---> read_pipe
   int fds[2];
   if (pipe(fds) != 0)
      return error_make(subprocess_error, "Failed to make pipe");
   create_file_fd(write_pipe, fds[1]);
   create_file_fd(read_pipe, fds[0]);

   pid_t pid = fork();
   switch (pid) {
      case 0: // child
      {
         // set up the stdout file descriptor to point to our pipe's write end
         if (dup2(file_fd(&write_pipe), STDOUT_FILENO) == -1) {
            panic("Failed to link stdout to pipe");
         }
         if (dup2(file_fd(&write_pipe), STDERR_FILENO) == -1) {
            panic("Failed to link stderr to pipe");
         }
         // these are now not needed on the child, and the cleanup won't work
         file_close(&write_pipe);
         file_close(&read_pipe);
         if (execvp(argv[0], (char * const *)argv))
            panic("Failed to exec binary");
      }
      case -1: // error
      {
         return error_make(subprocess_error, "Failed to fork a child process");
      }
      default: // parent
      {
         // the write pipe has to close before reading
         file_close(&write_pipe);
         int status;
         if (waitpid(pid, &status, 0) == -1) {
            return error_make(subprocess_error, "Failed to wait for child");
         }

         if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS)
            return error_make(subprocess_error,
               "Subprocess had non-zero exit status");

         epass(stream_read((struct stream *)&read_pipe, output));
      }
   }

   return no_error;
}
