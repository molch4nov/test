#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "shrmem.h"

#define NEG -1
#define PRIME 0
#define NORM 1

void print(caddr_t memptr, sem_t *semptr, const char *empty_string, int stat, int n) {
  while (true) {
	if ((sem_wait(semptr)) == 0) {
	  if (strcmp(memptr, empty_string) != 0) {
		if (sem_post(semptr) != 0) {
		  perror("SEM_POST");
		  exit(EXIT_FAILURE);
		}
		continue;
	  }
	  switch (stat) {
		case NEG:
		  sprintf(memptr, "Negative number %d\n", n);
		  break;
		case PRIME:
		  sprintf(memptr, "Prime number %d\n", n);
		  break;
		case NORM:
		  sprintf(memptr, "%d\n", n);
		  break;
		default:
		  sprintf(memptr, "Programmist debil\n");
	  }
	  if ((sem_post(semptr)) != 0) {
		perror("SEM_POST");
		exit(EXIT_FAILURE);
	  }
	  break;
	} else {
	  perror("SEM_WAIT");
	  exit(EXIT_FAILURE);
	}
  }
}

int main() {
  int n;
  char c;
  int map_fd = shm_open(BackingFile, O_RDWR, AccessPerms);
  if (map_fd < 0) {
	perror("SHM_OPEN");
	exit(EXIT_FAILURE);
  }
  caddr_t memptr = mmap(
	  NULL,
	  map_size,
	  PROT_READ | PROT_WRITE,
	  MAP_SHARED,
	  map_fd,
	  0);
  if (memptr == MAP_FAILED) {
	perror("MMAP");
	exit(EXIT_FAILURE);
  }
  sem_t *semptr = sem_open(SemaphoreName, O_CREAT, AccessPerms, 2);
  if (semptr == SEM_FAILED) {
	perror("SEM_OPEN");
	exit(EXIT_FAILURE);
  }
  char *empty_string = (char *)malloc(sizeof(char) * map_size);
  while (scanf("%d%c", &n, &c) != EOF) {
	if (n < 0) {
	  print(memptr, semptr, empty_string, NEG, n);
	}
	bool prost = true;
	for (int i = 2; i * i <= n; ++i) {
	  if (n % i == 0) {
		prost = false;
		print(memptr, semptr, empty_string, NORM, n);
		break;
	  }
	}
	if (prost) {
	  print(memptr, semptr, empty_string, PRIME, n);
	  break;
	}
  }
  usleep(00500000);
  memptr[0] = EOF;
  free(empty_string);
  close(map_fd);
  sem_close(semptr);
  return EXIT_SUCCESS;
}   