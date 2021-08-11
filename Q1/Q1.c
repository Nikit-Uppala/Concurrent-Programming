#define _POSIX_C_SOURCE 199309L
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<wait.h>
#include<limits.h>
#include<fcntl.h>
#include<time.h>
#include<pthread.h>
#include<inttypes.h>
#include<math.h>

typedef struct arguments
{
    int* arr;
    int s;
    int e;
}arg;
void selectionSort(int* arr,int s, int e)
{
    int smallest,index;
    for(int i=s;i<=e;i++)
    {
        smallest = arr[i],index=i;
        for(int j=i+1;j<=e;j++)
            if(arr[j]<smallest)
                smallest=arr[j],index=j;
        arr[index] = arr[i];
        arr[i] = smallest;
    }
    return;
}
void merge(int* arr,int s,int m, int e)
{
    int n1 = m-s+1,n2 = e-m;
    int left[n1], right[n2];
    for(int i=0;i<n1;i++)
        left[i]=arr[s+i];
    for(int i=0;i<n2;i++)
        right[i] = arr[m+1+i];
    int c1=0, c2=0, ctr=s;
    while(c1<n1 && c2<n2)
    {
        if(left[c1]<=right[c2])
            arr[ctr] = left[c1++];
        else
            arr[ctr] = right[c2++];
        ctr++;
    }
    while(c1<n1)
        arr[ctr++] = left[c1++];
    while(c2<n2)
        arr[ctr++] = right[c2++];
}
void mergeSort(int* arr, int s, int e)
{
    if(s<e)
    {
        if(e-s+1<=5)
        {
            selectionSort(arr,s,e);
            return;
        }
        int m = (e+s)/2;
        mergeSort(arr, s, m);
        mergeSort(arr, m+1, e);
        merge(arr, s, m, e);
    }
}
void* threaded_mergeSort(void* args)
{
    arg* arguments = (arg*) args;
    int s = arguments->s;
    int e = arguments->e;
    if(s<e)
    {
        if(e-s+1<=5)
        {
            selectionSort(arguments->arr,s,e);
            return NULL;
        }
        int m = (s+e)/2;
        pthread_t leftT, rightT;
        arg* left = (arg*)malloc(sizeof(arg));
        left->s = s;
        left->e = m;
        left->arr = arguments->arr;
        pthread_create(&leftT, NULL, threaded_mergeSort, left);
        arg* right = (arg*)malloc(sizeof(arg));
        right->s = m+1;
        right->e = e;
        right->arr = arguments->arr;
        pthread_create(&rightT, NULL, threaded_mergeSort, right);
        pthread_join(leftT, NULL);
        pthread_join(rightT, NULL);
        merge(arguments->arr,s,m,e);
        free(left), free(right);
    }
}
void process_mergeSort(int* arr, int s, int e)
{
    if(s<e)
    {
        if(e-s+1<=5)
        {
            selectionSort(arr,s,e);
            return;
        }
        int m = (s+e)/2;
        int child1 = fork();
        if(child1==-1)
        {
            perror("fork");
            _exit(1);
        }
        if(child1==0)
        {
            process_mergeSort(arr,s,m);
            _exit(1);
        }
        else
        {
            int child2=fork();
            if(child2==-1)
            {
                perror("fork");
                exit(1);
            }
            if(child2==0)
            {
                process_mergeSort(arr,m+1,e);
                _exit(1);
            }
            else
            {
                waitpid(child1,NULL,WUNTRACED);
                waitpid(child2,NULL,WUNTRACED);
                merge(arr,s,m,e);
            }
        }
    }
}
void runNormal(int* arr, int n, struct timespec t)
{
    long double start, seconds;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    start = t.tv_nsec/(1e9) + t.tv_sec;
    mergeSort(arr, 0, n-1);
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    seconds = t.tv_sec + t.tv_nsec/(1e9) - start;
    printf("Normal=%Lf\n",seconds);
    printf("Sorted Array - ");
    for(int i=0;i<n;i++) printf("%d ",arr[i]);
    printf("\n");
}
void runThreaded(int *arr, int n, struct timespec t)
{
    long double start, seconds;
    arg* temp = (arg*)malloc(sizeof(arg));
    temp->s = 0, temp->e = n-1, temp->arr = arr;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    start = t.tv_nsec/(1e9) + t.tv_sec;
    threaded_mergeSort((void*)temp);
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    seconds = t.tv_nsec/(1e9) + t.tv_sec - start;
    printf("Threaded=%Lf\n",seconds);
    printf("Sorted Array - ");
    for(int i=0;i<n;i++) printf("%d ",arr[i]);
    printf("\n");
}
void runConcurrent(int* arr,int n, struct timespec t)
{
    long double start, seconds;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    start = t.tv_nsec/(1e9) + t.tv_sec;
    process_mergeSort(arr,0,n-1);
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    seconds = t.tv_nsec/(1e9) + t.tv_sec - start;
    printf("Concurrent=%Lf\n",seconds);
    printf("Sorted Array - ");
    for(int i=0;i<n;i++) printf("%d ",arr[i]);
    printf("\n");
}
int main()
{
    struct timespec t;
    int n;
    scanf("%d",&n);
    key_t key = IPC_PRIVATE;
    int shm_id = shmget(key, (n+1)*sizeof(int), IPC_CREAT | 0666);
    int* arr=(int*)shmat(shm_id, NULL, 0);
    for(int i=0;i<n;i++) scanf("%d",arr+i);
    int* threadedArr = calloc(n,sizeof(int));
    for(int i=0;i<n;i++) threadedArr[i] = arr[i];
    int* normalArr = calloc(n, sizeof(int));
    for(int i=0;i<n;i++) normalArr[i] = arr[i];
    runNormal(normalArr, n, t);
    runConcurrent(arr, n, t);
    runThreaded(threadedArr, n, t);
    free(threadedArr), free(normalArr);
    return 0;
}