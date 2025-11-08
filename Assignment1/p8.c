#define _GNU_SOURCE
#include <err.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h> 
#include <time.h>


int SIZE = 32 * 2; //size of 2 floats
char *my_shm = "/myshm";

char *addr;
int fd;


int main(int argc, char *argv[]){



    if(argv[1] == NULL){
        printf("Please enter a size for the array\n");
        exit(1);
    }
    int N; 
    sscanf(argv[1], "%d", &N);
    int mid = N/2;

    float arr[N];
    srand(time(0));

    for (int i = 0; i < N; i++){
        arr[i] = (float)rand() / RAND_MAX; 
    }



    //create the shared memory
    fd = shm_open(my_shm, O_CREAT | O_RDWR, 0666); 
    if(fd == -1){
        printf("failed at: shm_open\n");
        exit(1);
    }

    //configure size of shared memory
    if(ftruncate(fd, SIZE) == -1){
        printf("failed at: ftruncate\n");
        exit(1);
    }


    //memory map to the shared memory
    addr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(addr == MAP_FAILED){
        printf("failed at: mmap\n");
        exit(1);
    }


    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pid_t pid1 = fork(); //add check for fork()
    if(pid1 == -1){
        printf("Error forking first child");
        exit(1);
    }

    if(pid1 == 0){
        float p1 = 34.42;
        float sum1 = 0;

        for(int i = 0; i < mid; i++){
            sum1 += arr[i];
        }

        sprintf(addr, "%f", sum1); //write to shared memory
        exit(0);
    }

    pid_t pid2 = fork();   
    if(pid2 == -1){
        printf("Error forking first child");
        exit(1);
    }
    if(pid2 == 0){
        float p2 = 78.96;
        float sum2 = 0;
        for(int i = mid ; i < N ; i++){
            sum2 += arr[i];
        }


        sprintf(addr + 32, "%f", sum2); //write to shared memory
        exit(0);
    }


    wait(NULL);
    wait(NULL);

    //result is saved as char. convert to float
    float sum1, sum2;
    sscanf(addr, "%f", &sum1); 
    sscanf(addr + 32, "%f", &sum2);

    printf("reading 1: %f\n", sum1); 
    printf("reading 2: %f\n", sum2); 

    float total = sum1 + sum2;
    printf("The total sum is: %f\n", total);


    clock_gettime(CLOCK_MONOTONIC, &end);

    double t_elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Time taken: %.9f seconds\n", t_elapsed);


    munmap(addr, SIZE);
    shm_unlink(my_shm); // Remove shared memory object
    close(fd);

    return 0;

}