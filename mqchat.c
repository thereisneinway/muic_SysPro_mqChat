#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

struct msg {//One instruction of struct for all threads and processes
  long int msg_type;
  char data[BUFSIZ];
};

void *func(void *arg) {
  int index = *((int *)arg);
  int running = 1, msgID;//1 indicate running, 0 indicate to stop
  struct msg a_msg;
  if(index == 1){//Primary receive
    long int rxcv_msg_type = 2;
    msgID = msgget((key_t) 1234,0666 |IPC_CREAT);//key 1
    while (1){
      msgrcv(msgID, (void *) &a_msg, BUFSIZ, rxcv_msg_type, 0);//Read memory area by key
      printf("Secondary: %s",a_msg.data);
      if (strncmp(a_msg.data, "end chat", 8) == 0) exit(EXIT_SUCCESS);
    }
  }else if(index == 2){//Secondary receive
    long int rxcv_msg_type = 1;
    msgID = msgget((key_t) 1234,0666 |IPC_CREAT);//key 2
    while (1){
      msgrcv(msgID, (void *) &a_msg, BUFSIZ, rxcv_msg_type, 0);//Read memory area by key
      printf("Primary: %s",a_msg.data);
      if (strncmp(a_msg.data, "end chat", 8) == 0) exit(EXIT_SUCCESS);
    }
  }else if(index == 3){//Primary send
    msgID = msgget((key_t)1234, 0666 | IPC_CREAT);//key 2
    char buffer[BUFSIZ];
    while (1) {
      msgsnd(msgID, (void *)&a_msg, BUFSIZ, 0) ;//Write memory area by key
      if (strncmp(buffer, "end chat", 8) == 0) exit(EXIT_SUCCESS);
      fgets(buffer, BUFSIZ, stdin);//Input user
      a_msg.msg_type = 1;
      strcpy(a_msg.data, buffer);
    }
  }else if(index == 4){//Secondary send
    msgID = msgget((key_t)1234, 0666 | IPC_CREAT);//key 1
    char buffer[BUFSIZ];
    while (1) {
      msgsnd(msgID, (void *)&a_msg, BUFSIZ, 0);//Write memory area by key
      if (strncmp(buffer, "end chat", 8) == 0) exit(EXIT_SUCCESS);
      fgets(buffer, BUFSIZ, stdin);//Input user
      a_msg.msg_type = 2;
      strcpy(a_msg.data, buffer);
  }
}
}

int main(int argc, char *argv[]){ 
  if(argc < 2){//check if there is user input argv
    fprintf(stderr, "Usage: %s <[1,2]>\n", *argv);
    exit(EXIT_FAILURE);
  }
  pthread_t read,write;
  argv++;
  int index1, index2;
  if(strncmp(*argv, "1", 1) == 0){
    index1 = 1;
    index2 = 3;
  }
  else if(strncmp(*argv, "2", 1) == 0){
    index1 = 2;
    index2 = 4;
  }
  pthread_create(&read, NULL, &func, &index1);
  pthread_create(&write, NULL, &func, &index2);
  pthread_join(read, NULL);
  pthread_join(write, NULL);
  
}