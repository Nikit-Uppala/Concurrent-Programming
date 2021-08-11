#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>
#include<pthread.h>
#include<unistd.h>
#include<time.h>
#include<assert.h>
#include "stack.h"
sem_t elec, acou, cood, singers;
pthread_mutex_t locks[3001], mutex, stageE, stageA;
int entered[3001], t, k, a, e, c, t1, t2;
int withSinger[3001], stageNums[3001];
stack* performing = NULL;
stack* electricStage = NULL;
stack* acousticStage = NULL;
typedef struct student
{
    pthread_t tid;
    int id;
    char name[101];
    char type;
    int arrival;
    int duration;
    int stage_num;
}student;
student stds[3001];
void* recieveShirt(void* inp)
{
    student std = *(student*)inp;
    if(c==0)
    {
        printf("\nThere are no coordinators\n");
        return NULL;
    }
    sem_wait(&cood);
    printf("\n\033[1;36m%s collecting t-shirt\n", std.name);
    sleep(2);
    sem_post(&cood);
}
void* electric_stage(student std)
{
    pthread_mutex_lock(&stageE);
    stageNums[std.id] = top(electricStage);
    electricStage = pop(electricStage);
    pthread_mutex_unlock(&stageE);
    if(std.type=='s')
        printf("\n\033[01;33m%s performing solo at electric stage %d for %d sec\n", std.name,stageNums[std.id], std.duration);
    else
    {
        sem_post(&singers);
        printf("\n\033[01;33m%s performing %c at electric stage %d for %d sec\n", std.name, std.type,stageNums[std.id], std.duration);
        pthread_mutex_lock(&mutex);
        performing = push(performing, std.id);
        pthread_mutex_unlock(&mutex);
    }
    sleep(std.duration);
    pthread_t tid;
    if(std.type!='s')
    {
        if(withSinger[std.id]!=-1)
        {
            sleep(2);
            printf("\n\033[0m%s and %s performance at electric stage %d ended\n", std.name, stds[withSinger[std.id]].name, stageNums[std.id]);
            pthread_create(&tid, NULL, recieveShirt, &stds[withSinger[std.id]]);
        }
        else
        {
            printf("\n\033[0;33m%s performance at electric stage %d finished\n", std.name, stageNums[std.id]);
            pthread_mutex_lock(&mutex);
            performing = deleteID(performing, std.id);
            pthread_mutex_unlock(&mutex);
            sem_wait(&singers);
        }
    }
    else
        printf("\n\033[0;34m%s performance at electric stage %d finished\n", std.name, stageNums[std.id]);
    sem_post(&elec);
    pthread_mutex_lock(&stageE);
    electricStage = push(electricStage, stageNums[std.id]);
    pthread_mutex_unlock(&stageE);
    recieveShirt(&std);
    if(withSinger[std.id]!=-1)
        pthread_join(tid, NULL);
}
void* acoustic_stage(student std)
{
    pthread_mutex_lock(&stageA);
    stageNums[std.id] = top(acousticStage);
    acousticStage = pop(acousticStage);
    pthread_mutex_unlock(&stageA);
    if(std.type=='s')
        printf("\n\033[1;34m%s performing solo at acoustic stage %d for %d sec\n", std.name, stageNums[std.id], std.duration);
    else
    {
        sem_post(&singers);
        printf("\n\033[1;34m%s performing %c at acoustic stage %d for %d sec\n", std.name, std.type, stageNums[std.id], std.duration);
        pthread_mutex_lock(&mutex);
        performing = push(performing, std.id);
        pthread_mutex_unlock(&mutex);
    }
    sleep(std.duration);
    pthread_t tid;
    if(std.type!='s')
    {
        if(withSinger[std.id]!=-1)
        {
            sleep(2);
            printf("\n\033[0m%s and %s performance at acoustic stage %d ended\n", std.name, stds[withSinger[std.id]].name, stageNums[std.id]);
            pthread_create(&tid, NULL, recieveShirt, &stds[withSinger[std.id]]);
        }
        else
        {
            printf("\n\033[0;34m%s performance at acoustic stage %d finished\n", std.name, stageNums[std.id]);
            if(std.type!='s')
            {
                pthread_mutex_lock(&mutex);
                performing = deleteID(performing, std.id);
                pthread_mutex_unlock(&mutex);
            }
            sem_wait(&singers);
        }
    }
    else
        printf("\n\033[0;34m%s performance at acoustic stage %d finished\n", std.name, stageNums[std.id]);
    sem_post(&acou);
    pthread_mutex_lock(&stageA);
    acousticStage = push(acousticStage, stageNums[std.id]);
    pthread_mutex_unlock(&stageA);
    recieveShirt(&std);
    if(withSinger[std.id]!=-1)
        pthread_join(tid, NULL);
}
int checkEnterStatus(student std)
{
    pthread_mutex_lock(&locks[std.id]);
    if(entered[std.id])
    {
        pthread_mutex_unlock(&locks[std.id]);
        return 0;
    }
    else
    {
        entered[std.id] = 1;
        pthread_mutex_unlock(&locks[std.id]);
        return 1;
    }
}
void* waitElectric(void* inp)
{
    if(e==0)
    {
        printf("\nThere are no electric stages\n");
        return NULL;
    }
    struct timespec limit;
    student std = *(student*)inp;
    clock_gettime(CLOCK_REALTIME, &limit);
    limit.tv_sec+=t;
    int check = sem_timedwait(&elec, &limit);
    if(check==0)
    {
        if(checkEnterStatus(std))
            electric_stage(std);
        else
            sem_post(&elec);
    }
}
void* waitAcoustic(void* inp)
{
    if(a==0)
    {
        printf("\nThere are no acoustic stages\n");
        return NULL;
    }
    struct timespec limit;
    student std = *(student*)inp;
    clock_gettime(CLOCK_REALTIME, &limit);
    limit.tv_sec+=t;
    int check = sem_timedwait(&acou, &limit);
    if(check==0)
    {
        if(checkEnterStatus(std))
            acoustic_stage(std);
        else
            sem_post(&acou);
    }
}
void* joinOthers(void* inp)
{
    struct timespec limit;
    student std = *(student*)inp;
    clock_gettime(CLOCK_REALTIME, &limit);
    limit.tv_sec+=t;
    int check = sem_timedwait(&singers, &limit);
    if(check == 0)
    {
        if(checkEnterStatus(std))
        {
            pthread_mutex_lock(&mutex);
            int std_id = top(performing);
            if(std_id==-1)
                return NULL;
            performing = pop(performing);
            pthread_mutex_unlock(&mutex);
            withSinger[std_id] = std.id;
            printf("\n\033[0m%s joined %s's performance, performance extended by 2 sec\n", std.name, stds[std_id].name);
        }
        else
            sem_post(&singers);
    }
}
void* arrival(void* inp)
{
    student std = *((student*)inp);
    sleep(std.arrival);
    printf("\n\033[1;32m%s %c arrived\n", std.name, std.type);
    if(std.type=='b')
        waitElectric((void*)&std);
    else if(std.type=='v')
        waitAcoustic((void*)&std);
    else if(std.type=='p' || std.type=='g' || std.type=='s')
    {
        pthread_t tid1, tid2,tid3;
        student std1 = std, std2 = std, std3=std;
        pthread_create(&tid1, NULL, waitElectric, (void*)&std1);
        pthread_create(&tid2, NULL, waitAcoustic, (void*)&std2);
        if(std.type=='s')
            pthread_create(&tid3, NULL, joinOthers, (void*)&std3);
        pthread_join(tid1, NULL);
        pthread_join(tid2, NULL);
        if(std.type=='s')
            pthread_join(tid3, NULL);
    }
    else
        printf("\n%c not a valid instrument\n", std.type);
    if(!entered[std.id])
        printf("\n\033[1;31m%s %c left because of impatience\n", std.name, std.type);
}
int main()
{
    srand(time(0));
    scanf("%d%d%d%d%d%d%d",&k, &a, &e, &c, &t1, &t2, &t);
    assert(k>=0 && a>=0 && e>=0 && c>=0 && t1>=0 && t1<=t2 && t>=0);
    sem_init(&acou, 0, a);
    sem_init(&elec, 0, e);
    sem_init(&cood, 0, c);
    sem_init(&singers, 0, 0);
    pthread_mutex_init(&stageE, NULL);
    pthread_mutex_init(&stageA, NULL);
    pthread_mutex_init(&mutex, NULL);
    for(int i=e;i>0;i--)
        electricStage = push(electricStage, i);
    for(int i=a;i>0;i--)
        acousticStage = push(acousticStage, i);
    for(int i=0;i<k;i++)
    {
        entered[i]=0;
        withSinger[i]=-1;
        stageNums[i]=-1;
        pthread_mutex_init(&locks[i], NULL);
        scanf("%s %c%d", stds[i].name, &stds[i].type, &stds[i].arrival);
        assert(stds[i].arrival>=0);
        float d = t1 + (t2-t1)*(float)rand()/(float)RAND_MAX;
        stds[i].duration = (int)d;
        stds[i].id = i;
        pthread_create(&stds[i].tid, NULL, arrival, (void*)&stds[i]);
    }
    for(int i=0;i<k;i++)
        pthread_join(stds[i].tid, NULL);
    printf("\n\033[0mFinished\n");
    sem_destroy(&elec), sem_destroy(&acou), sem_destroy(&cood), sem_destroy(&singers);
    for(int i=0;i<k;i++)
        pthread_mutex_destroy(&locks[i]);
    pthread_mutex_destroy(&stageE);
    pthread_mutex_destroy(&stageA);
    pthread_mutex_destroy(&mutex);
    return 0;
}