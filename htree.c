#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h> // for EINTR
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>

// Print out the usage of the program and exit.
void Usage(char *);
uint32_t jenkins_one_at_a_time_hash(const uint8_t *, uint64_t);
uint32_t findHash(void *args);
void slice(const char *str, char *result, size_t start, size_t end);
void constructArgs(struct args *argument, int threadNo, int max, uint8_t *mapping);
// block size
#define BSIZE 4096
struct args
{
  int threadNo;
  int max;
  uint8_t *mmapLink;
};

int main(int argc, char **argv)
{
  int32_t fd;
  uint32_t nblocks;

  // input checking
  if (argc != 3)
    Usage(argv[0]);

  // open input file
  fd = open(argv[1], O_RDWR);
  if (fd == -1)
  {
    perror("open failed");
    exit(EXIT_FAILURE);
  }
  uint32_t hash = 0;
  // use fstat to get file size
  struct stat stats;
  fstat(fd, &stats);
  // calculate nblocks

  uint8_t *map = (uint8_t *)mmap(NULL, stats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  printf(" no. of blocks = %u \n", nblocks);

  // double start = GetTime();

  // calculate hash value of the input file
  int threadNo = 0;
  struct args *argumets;
  constructArgs(argumets, threadNo, 6, map);

  findHash((void *)argumets);

  // double end = GetTime();
  printf("hash value = %u \n", hash);
  // printf("time taken = %f \n", (end - start));
  close(fd);
  return EXIT_SUCCESS;
}

uint32_t findHash(void *args)
{
  pthread_t left;
  pthread_t right;
  struct args argu = *((struct args *)args);
  struct args *leftArgs = (struct args *)malloc(sizeof(args));

  int max = ((struct args *)args)->max;
  int threadNo = (((struct args *)args)->threadNo);
  leftArgs->max = max;
  int leftThreadNo = threadNo * 2 + 1;
  leftArgs->threadNo = leftThreadNo;

  struct args *rightArgs = (struct args *)malloc(sizeof(args));
  rightArgs->max = max;
  int rightThreadNo = threadNo * 2 + 2;
  rightArgs->threadNo = rightThreadNo;

  if (leftArgs->threadNo < max)
  {
    int success = pthread_create(&left, NULL, &findHash, (void *)leftArgs);
    printf("Got to 1\n");
  }

  if ((rightArgs->threadNo) < max)
  {
    int success = pthread_create(&right, NULL, &findHash, (void *)rightArgs);
    printf("Got to 2\n");
  }

  int val;
  pthread_exit(&val);
  return val;
}

uint32_t
jenkins_one_at_a_time_hash(const uint8_t *key, uint64_t length)
{
  uint64_t i = 0;
  uint32_t hash = 0;

  while (i != length)
  {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

void Usage(char *s)
{
  fprintf(stderr, "Usage: %s filename num_threads \n", s);
  exit(EXIT_FAILURE);
}

void slice(const char *str, char *result, size_t start, size_t end)
{
  strncpy(result, str + start, end - start);
}

uint32_t hashnode(char *str, uint64_t length)
{
  return jenkins_one_at_a_time_hash(str, length);
}

u_int32_t internalNodeHash(uint32_t *str, uint32_t *lefthash, uint32_t *rightHash, uint64_t length)
{
  uint32_t hash = jenkins_one_at_a_time_hash(str, length);
  char *hash_string = malloc(31);
  sprintf(hash_string, "%u%u%u", hash, lefthash, rightHash);

  uint32_t value = jenkins_one_at_a_time_hash(hash_string, length);
  free(hash_string);
  return value;
}

int isLeaf(int nodeNo, int max)
{
  if (nodeNo * 2 + 1 <= max)
  {
    return 1;
  }
  return 0;
}

void constructArgs(struct args *argument, int threadNo, int max, uint8_t *mapping)
{
  argument = (struct args *)malloc(sizeof(struct args));
  argument->max = max;
  int rightThreadNo = threadNo * 2 + 2;
  argument->threadNo = rightThreadNo;
  argument->mmapLink = mapping;
}
