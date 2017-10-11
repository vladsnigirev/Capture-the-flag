#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <wait.h>
#include <setjmp.h>
void mysig(int sig);

int main(void)
{
    int sid;
    uint addrlen;
    struct sockaddr_in addr;
    char buf[4096];
    int rop;
    int ns;
    char* addr_shm;
    char str[1024];
    struct sembuf L[1]= {0,-1,0};
    struct sigaction sa_new;// Объявляем структуру данных типа sigaction для обработки прерывания
    int i;
    printf("Hello!Try to find me out!\n");
    memset(str,0,1024);
    memset(buf,0,4096);
    if((sid=socket(AF_INET,SOCK_DGRAM,0))<0){
        printf("socket error\n");
        exit(1);
    }
    bzero((void*)&addr,sizeof(addr));
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");//локальный IP адрес
    addr.sin_family=AF_INET;// заполняем поле семейства адресов
    addr.sin_port=htons(7777);//заполняем поле порта
    if(bind(sid,(struct sockaddr*)&addr,sizeof(addr))!=0){//првязка гнезда к адресу
        printf("bind error\n");
        exit(2);
    }
    addrlen=sizeof(struct sockaddr_in);// размер sockaddr_in, нужно для recvfrom;
    //Ожидаем любого сообщения на 7777 порт по локальному адресу.
    if(recvfrom(sid,buf,sizeof(buf),0,(struct sockaddr*)&addr,&addrlen)<0){
        printf("can't rcv\n");
        exit(3);
    }
    //ФЛАГ1
    printf("you_g0t_it\nCongrants! Your next challenge is SHM.\n");
    shutdown(sid,0);
    close(sid);
    //создание роп
    memset(str,0,1024);
    if((rop=shmget(777,1024,IPC_CREAT|0660))==-1){
        printf("shmget error\n");
        exit(4);
    }
    if((ns=semget(777,1,IPC_CREAT|0660))==-1){
        shmctl(rop,IPC_RMID,0);
        printf("semget error\n");
        exit(5);
    }
    addr_shm=(char*) shmat(rop,0,0);
    memset(addr_shm,0,1024);
    if(addr_shm==(char*) -1){
        shmctl(rop,IPC_RMID,0);
        semctl(ns,0,IPC_RMID,0);
        printf("shmat error\n");
        exit(6);
    }
    //Посылаем ФЛАГ2 в разделяемую память
    strcpy(str,"you_close_to_finish!\nClue - User's signal #2\n\0");
    strcpy(addr_shm,str);
    if(semctl(ns,0,SETVAL,0)<0){
        shmctl(rop,IPC_RMID,0);
        semctl(ns,0,IPC_RMID,0);
        shmdt(addr_shm);
        printf("semctl error");
        exit(7);
    }
    //Заблокировали процесс.
    if(semop(ns,L,1)<0){
        shmctl(rop,IPC_RMID,0);
        semctl(ns,0,IPC_RMID,0);
        shmdt(addr_shm);
        printf("semop error");
        exit(8);
    }
    if(shmdt(addr_shm)<0){
        shmctl(rop,IPC_RMID,0);
        semctl(ns,0,IPC_RMID,0);
        printf("addr deliting error\n");
        exit(9);
    }
    if(shmctl(rop,IPC_RMID,0)<0){
        printf("shm deliting error\n");
        exit(10);
    }
    sa_new.sa_handler=mysig;// указываем, какой именно фн. будем обрабатывать прерывание.
    sigprocmask (0,0,&sa_new.sa_mask);// сохраняем записываем сигнальную маску нашего процесса в структуру.
    sa_new.sa_flags=0;// флаги не нужны=> равны 0.
    sigaction (SIGUSR2,&sa_new,0);//указываем, что будем обрабатывать сигнал прерывания с клавиатуры самостоятельно
    printf("You have only 15 minutes for this challenge or you lose!\n");
    for(i=0; i<180;i++)
        sleep(10);
    return 0;
}
void mysig(int sig){
    printf("thats_@ll\n");
    _exit(sig);
}
