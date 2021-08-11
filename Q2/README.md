To compile the file - 
gcc Q2.c -lpthread

Explanation -
All the companies, vaccination zones and students are created as threads(one thread per entity).

Pharmacutical companies prepare batches of vaccine (if simulation just began or all its batches previously manufactured are consumed) in a time interval of 'w' seconds (random from 2-5 seconds. Can change the limits)
The companies will wait until all their vaccines are consumed before continuing the manufacture. This is done by busy waiting where I store the vaccines left manufactured by the company in an array. Only one thread at a time can access the element of the array. Done by using mutex lock(one per company).

Vaccination zone will be busy waiting until they recieve a batch of vaccine from any one of the companies. In this busy waiting, they iterate over the number of batches available from each company stored in the array 'batchesLeft'. Only one thread can access one element of the array at a time(that is 2 threads can't access the same element at a time). This is done by using mutex lock(one per company).
On recieving a batch, the vaccination zones wait for the students to be allocated. If there are no waiting students or the slots alloted are full, then the zone enters vaccination phase.
When all the alloted students are vaccinated or all the vaccines in the batch are over, the zone comes out of vaccination phase and waits for vaccines or students to be allocated(depending on above conditions).

Students arrive randomly in the interval (1 - 10 seconds. Can change the limits).
Students wait for a slot in any of the vaccination zones. So, until they get a slot, it iterates over the slotsLeft array. Only 1 student thread can access one element of the array at a time. Done by using mutex locks(one per vaccination zone).

Student tests positive when a randomly generated number (between 0 and 1) is less than probability of success of the vaccine used.
If students tests positive, the thread exits otherwise it enters for next round(max 3 rounds).

Simulation stops when all students test positive or test negative after 3 rounds or companies are 0 or vaccination zones are 0.