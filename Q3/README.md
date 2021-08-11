To compile the files:
run the make command.


Explanation:
I considered students (musicians and singers) as threads.
Electric stages as a resource type with 'e' instances, Acoustic stage as a resource type with 'a' instances.

The above resources are implemented as functions electric_stage() and acoustic_stage() such that maximum number of threads executing(utilizing the resource) will be 'e' for electric stage and 'a' for acoustic stage. (This is for musicians, singers performing solo).

For musicians of type 'b' or 'v', they wait for one stage, electric and acoustic respectively. So, they wait for the appropriate semaphore and if the time exceeds 't' then they leave.

If e=0 or a=0, I am not waiting for the semaphore and directly printing that there are no electric stages or acoustic stages respectively.

For musicians of the other types (except 'b' and 'v'), they can perform on any stage. So, I create 2 threads -
One thread waits for an electric stage and the other thread for acoustic stage. Whichever locks the mutex first(using a global array 'entered' to check which accessed first), the musician gets that stage. If both the threads fail to lock(time of waiting exceeds 't') then they leave.
The threads are synchronised by using mutex locks(one mutex lock per musician).

In the above cases, I push the student's id(index assigned manually) into a list if they get a stage(resource). The access of this list is such that one thread can access this list at a time(that is for pushing deleting, etc). This is done by using a mutex lock which is common to all the threads.

For singers, I create 3 threads - One waiting for electric stage, one waiting for acoustic stage and the other for joining other musician's performance.
Synchronization of the three threads is done by using mutex lock.
Out of the three whichever locks the mutex lock first(using the global array 'entered'), the singer will get a stage or join other's performance.
I am using another global array 'withSinger' which indicates if a singer has joined a musician(-1 if no singer). Based on this value, I extend the duration of performance by 2 seconds.

Bonus - 
For getting stage numbers, I am keeping all the available electric stages in one list and all the available acoustic stages in other list. If a musician gets the stage then I give him the stage whose number is present at the top(first) of the list and remove it from the list.
Only 1 thread can access the list at a time. (Imposed the restriction by use of the mutex lock).

For singers, if they perform solo then I directly call the function recieveShirt() to collect T-Shirt. If they join some performance then I create a thread and call the function.
