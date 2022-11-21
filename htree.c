#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h> // for EINTR
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
// Print out the usage of the program and exit.

// block size
#define BSIZE 4096
typedef struct args
{
  int threadNo;
  int maxNoThreads;
  uint8_t *mmapLink;
  int length;
} args;

void Usage(char *);
uint32_t jenkins_one_at_a_time_hash(const uint8_t *, uint64_t);
void *findHash(void *args);
args *constructArgs(args *argument, int threadNo, int maxNoThreads, uint8_t *mapping, int length);
uint32_t internalNodeHash(uint32_t hash, uint32_t *lefthash, uint32_t *rightHash);
double GetTime();
uint32_t hashnode(args *thread, int blockperthread);

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

  nblocks = stats.st_size / BSIZE;
  int blocksperthread = nblocks / atoi(argv[2]);

  uint8_t *map = (uint8_t *)mmap(NULL, stats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  printf("no. of blocks = %u \n", nblocks);

  double start = GetTime();

  // calculate hash value of the input file
  int threadNo = 0;
  args *arguments = NULL;
  arguments = constructArgs(arguments, threadNo, atoi(argv[2]), map, blocksperthread);
  printf("Blocks per thread: %d\n", arguments->length);
  hash = *(uint32_t *)findHash((void *)arguments);

  double end = GetTime();
  printf("hash value = %u \n", hash);
  printf("time taken = %f \n", (end - start));
  close(fd);
  return EXIT_SUCCESS;
}

void *findHash(void *threadArgs)
{
  pthread_t left = 0;
  pthread_t right = 0;
  args argu = *((args *)threadArgs);

  // hash your data

  uint32_t *hash = malloc(sizeof(uint32_t));
  uint32_t *lefthash = NULL;
  uint32_t *righthash = NULL;

  int maxNoThreads = ((args *)threadArgs)->maxNoThreads;
  int threadNo = (((args *)threadArgs)->threadNo);
  int bpt = (((args *)threadArgs)->length);

  args *leftArgs = NULL;
  leftArgs = constructArgs(leftArgs, threadNo * 2 + 1, maxNoThreads, argu.mmapLink, bpt);

  args *rightArgs = NULL;
  rightArgs = constructArgs(rightArgs, threadNo * 2 + 2, maxNoThreads, argu.mmapLink, bpt);

  if (leftArgs->threadNo < maxNoThreads)
  {
    int success = pthread_create(&left, NULL, findHash, (void *)leftArgs);
    if (success == 1)
    {
      perror("Failed to create thread");
      exit(1);
    }
  }
  else
  {
    *hash = hashnode(&argu, bpt);
    // child node
    if (argu.threadNo == 0)
    {
      return hash;
    }

    pthread_exit(hash);
  }

  if ((rightArgs->threadNo) < maxNoThreads)
  {
    pthread_create(&right, NULL, findHash, (void *)rightArgs);
  }

  *hash = hashnode(&argu, bpt);
  if (left != 0)
  {

    pthread_join(left, (void **)&lefthash);
  }

  if (right != 0)
  {
    pthread_join(right, (void **)&righthash);
  }
  uint32_t temp = 0;
  if (lefthash != NULL){

    temp = internalNodeHash(*hash, lefthash, righthash);
  }
  *hash = temp;

  if (argu.threadNo == 0)
  {
    return hash;
  }
  pthread_exit(hash);
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

uint32_t hashnode(args *thread, int blockperthread)
{
  uint32_t offset = thread->threadNo;
  offset *= blockperthread * BSIZE;
  const uint8_t *inte = (const uint8_t *)(&(thread->mmapLink[offset]));

  uint64_t len = BSIZE;
  len *= blockperthread;

  return jenkins_one_at_a_time_hash(inte, len);
}

uint32_t internalNodeHash(uint32_t hash, uint32_t *lefthash, uint32_t *rightHash)
{
  char *hash_string;

  if (rightHash == NULL)
  {
    hash_string = malloc(21);
    sprintf(hash_string, "%u%u", hash, *lefthash);
  }
  else
  {
    hash_string = malloc(31);
    sprintf(hash_string, "%u%u%u", hash, *lefthash, *rightHash);
  }
  uint32_t value = jenkins_one_at_a_time_hash((uint8_t *)hash_string, strlen(hash_string));
  // free(hash_string);
  return value;
}

int isLeaf(int nodeNo, int maxNoThreads)
{
  if (nodeNo * 2 + 1 <= maxNoThreads)
  {
    return 1;
  }
  return 0;
}

args *constructArgs(args *argument, int threadNo, int maxNoThreads, uint8_t *mapping, int length)
{
  argument = (args *)malloc(sizeof(args));

  argument->maxNoThreads = maxNoThreads;

  argument->threadNo = threadNo;
  argument->mmapLink = mapping;
  argument->length = length;
  return argument;
}

double GetTime()
{
  struct timeval t;
  int rc = gettimeofday(&t, NULL);
  assert(rc == 0);
  return (double)t.tv_sec + (double)t.tv_usec / 1e6;
}
