1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

It creates a new child process, leaving the parent process intact; if we just called execvp it would replace the current process and it would not continue

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

it returns an error code; I handle this by printing the error and exiting

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

It uses the PATH variabale and searches each directory in order they appear in PATH, selecting the first one

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

