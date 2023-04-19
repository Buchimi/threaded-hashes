# Multi-threaded File Hashing Program
This is a C/C++ program that calculates the hash of a file using Jenkin's one-at-a-time hash function in a multi-threaded manner, resulting in significant performance improvements compared to single-threaded execution. The program allows users to specify the number of threads to be used for hashing and outputs the computed hash value.

## Getting Started
To compile and run the program, you will need to have a C/C++ compiler installed on your system. The program has been tested on Windows and Linux environments, but should work on other platforms as well.

Clone this repository to your local machine using 
```
git clone https://github.com/Buchimi/threaded-hashes
``` 
or download the zip file and extract it.
Navigate to the src directory and open a terminal/command prompt in that directory.
Compile using
```
gcc -o main -c htree.c
```
Run the command make to compile the program.
To run the program, type 
```
./main <filename> <num_threads>
``` 
where <filename> is the path to the file to be hashed and <num_threads> is the number of threads to be used for hashing.

## Program Output
The program will output the hash value computed for the input file, as well as the time taken for the computation. The time measurement includes both the time taken for the hashing process and any thread synchronization overhead.

