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

// block size
#define BSIZE 4096
typedef struct args
{
  int threadNo;
  int max;
  uint8_t *mmapLink;
  int length;
} args;

void Usage(char *);
uint32_t jenkins_one_at_a_time_hash(const uint8_t *, uint64_t);
uint32_t findHash(void *args);
void slice(const char *str, char *result, size_t start, size_t end);
void constructArgs(args *argument, int threadNo, int max, uint8_t *mapping, int length);
u_int32_t internalNodeHash(uint32_t *str, uint32_t *lefthash, uint32_t *rightHash, uint64_t length);
uint32_t hashnode(char *str, uint64_t length);

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

  int blocksperthread = stats.st_blocks / atoi(argv[2]);

  uint8_t *map = (uint8_t *)mmap(NULL, stats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  printf(" no. of blocks = %u \n", nblocks);

  // double start = GetTime();

  // calculate hash value of the input file
  int threadNo = 0;
  struct args *argumets;
  constructArgs(argumets, threadNo, 6, map, blocksperthread);

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

  // get the slice of string
  char *stringSlice;
  slice(argu.mmapLink, stringSlice, argu.threadNo * BSIZE, (argu.threadNo + 1) * BSIZE);

  // hash your data

  uint32_t hash;
  uint32_t *lefthash = NULL;
  uint32_t *righthash = NULL;

  int max = ((struct args *)args)->max;
  int threadNo = (((struct args *)args)->threadNo);
  int bpt = (((struct args *)args)->length);

  struct args *leftArgs;
  constructArgs(leftArgs, threadNo * 2 + 1, max, argu.mmapLink, bpt);

  struct args *rightArgs;
  constructArgs(rightArgs, threadNo * 2 + 2, max, argu.mmapLink, bpt);

  if (leftArgs->threadNo < max)
  {
    int success = pthread_create(&left, NULL, &findHash, (void *)leftArgs);
    printf("Got to 1\n");
  }
  else
  {
    // child node
    hash = hashnode(stringSlice, bpt);
    pthread_exit(&hash);
  }

  if ((rightArgs->threadNo) < max)
  {
    int success = pthread_create(&right, NULL, &findHash, (void *)rightArgs);
    printf("Got to 2\n");
  }
  pthread_join(left, lefthash);
  pthread_join(right, righthash);

  hash = internalNodeHash(stringSlice, lefthash, righthash, bpt);

  pthread_exit(&hash);
  // return val;
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
  const uint8_t *inte = (const uint8_t *)str;
  return jenkins_one_at_a_time_hash(inte, length);
}

u_int32_t internalNodeHash(uint32_t *str, uint32_t *lefthash, uint32_t *rightHash, uint64_t length)
{
  if (rightHash == NULL)
  {
  }
  else
  {
    const uint8_t *inte = (const uint8_t *)str;
    uint32_t hash = jenkins_one_at_a_time_hash(inte, length);
    char *hash_string = malloc(31);
    sprintf(hash_string, "%u%u%u", hash, lefthash, rightHash);

    uint32_t value = jenkins_one_at_a_time_hash(hash_string, length);
    free(hash_string);
    return value;
  }
}

int isLeaf(int nodeNo, int max)
{
  if (nodeNo * 2 + 1 <= max)
  {
    return 1;
  }
  return 0;
}

void constructArgs(args *argument, int threadNo, int max, uint8_t *mapping, int length)
{
  argument = (struct args *)malloc(sizeof(struct args));

  argument->max = max;
  int rightThreadNo = threadNo * 2 + 2;
  argument->threadNo = rightThreadNo;
  argument->mmapLink = mapping;
  argument->length = length;
}
