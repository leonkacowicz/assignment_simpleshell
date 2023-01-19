# What is this?

This is a very simple shell implementation, done as one of my university assignments back in 2007.
As I've decided to keep the source code as was submitted (except for minor adjustments), 
most identifiers are in portuguese.

# How it works?

Upon starting the shell, it shows a message "Trabalho de SC1. Shell Simples", followed by a prompt.
At each prompt the shell will print "[lsh]$ " and wait for the user to enter a command line of up to 255 characters.  

I implemented all the features requested:
1. Basic command line `cmd arg1 arg2 ... argn`:
   
   An executable is invoked (first token) and the remaining 
   tokens are passed to the executable as arguments 

2. Command line with input/output redirection to files:

   `cmd arg1 arg2 ... argn [ < infile ] [ > outfile ]`
   The shell will redirect the standard input to `infile` (if present) and
   the standard output to `outfile` (if present)

3. Command line with pipe:
   
   `cmd arg1...argn [<infile] [>outfile] | cmd arg1...argn [<infile][>outfile] | ...`
   The shell will execute each command in parallel and redirect the standard output of each executable
   to the standard input of the following.

4. Command line with `&`
   
   `cmdwithpipe1 & cmdwithpipe2 & ...`
   Execute all command lines in parallel and leave the processes running in the background.



