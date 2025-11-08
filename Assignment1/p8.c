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


int SIZE = sizeof(float) * 2; //size of 2 floats
char *my_shm = "/myshm";
char *msg = "hello IPC";
char *addr;
int fd;

int main(){

    int arr[] = {1,1,1,1,1,1,1,1,1};
    int n = sizeof(arr)/sizeof(arr[0]);
    int mid = n/2;

    //create the shared memory
    fd = shm_open(my_shm, O_CREAT | O_RDWR, 0666); 
    if(fd == -1){
        perror("failed at: shm_open");
        exit(1);
    }

    //configure size of shared memory
    if(ftruncate(fd, SIZE) == -1){
        perror("failed at: ftruncate");
        exit(1);
    }


    //memory map to the shared memory
    addr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(addr == MAP_FAILED){
        perror("failed at: mmap");
        exit(1);
    }

    


    pid_t pid1 = fork(); //add check for fork()
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
    if(pid2 == 0){
        float p2 = 78.96;
        float sum2 = 0;
        for(int i = mid ; i < n ; i++){
            sum2 += arr[i];
        }


        sprintf(addr + 32, "%f", sum2); //write to shared memory
        exit(0);
    }


    wait(NULL);
    wait(NULL);

    printf("Size of array: %d\n", n);

    //result is saved as char. convert to float
    float sum1, sum2;
    sscanf(addr, "%f", &sum1); 
    sscanf(addr + 32, "%f", &sum2);

    printf("reading 1: %f\n", sum1); 
    printf("reading 2: %f\n", sum2); 

    float total = sum1 + sum2;
    printf("The total sum is: %f\n", total);


    

    

    munmap(addr, SIZE);
    shm_unlink(my_shm); // Remove shared memory object
    close(fd);

    return 0;

}