#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<time.h>
#include<assert.h>
int n, m, o;
pthread_mutex_t vaccZones[3001], pharmComps[3001], stdCount;
int batchLeft[3001],vaccBatch[3001], vaccLeftZone[3001], vaccPhase[3001], slotsLeft[3001], vaccLeftComp[3001], stdRound[3001], batchUse[3001];
int batchRecieved[3001];
int slotsAllocated[3001];
int studentsLeft, notAllocated;
float success[3001];
void VaccPhase(int std_num, int zone, int* vaccinated)
{
    printf("\nStudent %d assigned a slot on the Vaccination Zone %d and waiting to be vaccinated\n", std_num, zone);
    while(1)
    {
        pthread_mutex_lock(&vaccZones[zone]);
        if(vaccPhase[zone]==1)
        {
            pthread_mutex_unlock(&vaccZones[zone]);
            break;
        }
        pthread_mutex_unlock(&vaccZones[zone]);
    }
    printf("\nStudent %d on Vaccination Zone %d has been vaccinated which has success probability %f\n", std_num, zone, success[batchUse[zone]]);
    sleep(1);
    float prob = (float)rand()/(float)RAND_MAX;
    if(prob<=success[batchUse[zone]])
    {
        *vaccinated=1;
        printf("\nStudent %d has tested positive for antibodies\n", std_num);
    }
    else
    {
        stdRound[std_num]++;
        printf("\nStudent %d has tested negative for antibodies\n", std_num);
    }
    pthread_mutex_lock(&pharmComps[batchUse[zone]]);
    vaccLeftComp[batchUse[zone]]--;
    if(vaccLeftComp[batchUse[zone]]==0)
        printf("\nAll the vaccines prepared by Pharmaceutical Company %d are emptied. Resuming production now\n", batchUse[zone]);
    pthread_mutex_unlock(&pharmComps[batchUse[zone]]);
    pthread_mutex_lock(&vaccZones[zone]);
    vaccLeftZone[zone]--;
    slotsAllocated[zone]--;
    if(vaccLeftZone[zone]==0 || slotsAllocated[zone]==0)
    {
        vaccPhase[zone]=0;
        if(vaccLeftZone[zone]==0)
        {
            printf("\nVaccination Zone %d has run out of vaccines\n", zone);
            batchRecieved[zone]=0;
        }
    }
    pthread_mutex_unlock(&vaccZones[zone]);
}
void* Student(void* inp)
{
    int std_num = *(int*)inp;
    stdRound[std_num] = 1;
    int timeL = 1, timeU = 10;
    float a = timeL + (timeU-timeL+1) * (float)rand()/(float)RAND_MAX;
    int arrival = (int)a;
    sleep(arrival);
    pthread_mutex_lock(&stdCount);
    notAllocated++;
    pthread_mutex_unlock(&stdCount);
    int vaccinated = 0, allocated = 0, zone=0;
    while(!vaccinated && stdRound[std_num]<=3)
    {
        if(!allocated)
        printf("\nStudent %d has arrived for his round %d of Vaccination\n", std_num, stdRound[std_num]);
        printf("\nStudent %d is waiting to be allocated a slot on a Vaccination Zone\n", std_num);
        while(!allocated)
        {
            for(int i=1;i<=m;i++)
            {
                pthread_mutex_lock(&vaccZones[i]);
                if(slotsLeft[i]>0 && !vaccPhase[i])
                {
                    slotsLeft[i]--;
                    slotsAllocated[i]++;
                    pthread_mutex_unlock(&vaccZones[i]);
                    zone = i;
                    pthread_mutex_lock(&stdCount);
                    notAllocated--;
                    pthread_mutex_unlock(&stdCount);
                    allocated = 1;
                    break;
                }
                pthread_mutex_unlock(&vaccZones[i]);
            }
        }
        VaccPhase(std_num, zone, &vaccinated);
        allocated = 0;
        pthread_mutex_lock(&stdCount);
        notAllocated++;
        pthread_mutex_unlock(&stdCount);
    }
    pthread_mutex_lock(&stdCount);
    studentsLeft--;
    notAllocated--;
    pthread_mutex_unlock(&stdCount);
}
void* VaccZone(void* inp)
{
    int zone_num = *(int*)inp;
    pthread_mutex_lock(&vaccZones[zone_num]);
    vaccPhase[zone_num] = 0, slotsLeft[zone_num] = 0, vaccLeftZone[zone_num] = 0;
    pthread_mutex_unlock(&vaccZones[zone_num]);
    int check = 0;
    while(1)
    {
        pthread_mutex_lock(&stdCount);
        if(studentsLeft<=0)
        {
            pthread_mutex_unlock(&stdCount);
            return NULL;
        }
        pthread_mutex_unlock(&stdCount);
        pthread_mutex_lock(&vaccZones[zone_num]);
        if(vaccPhase[zone_num])
        {
            pthread_mutex_unlock(&vaccZones[zone_num]);
            continue;
        }
        pthread_mutex_unlock(&vaccZones[zone_num]);
        pthread_mutex_lock(&vaccZones[zone_num]);
        if(vaccPhase[zone_num]==1)
        {
            pthread_mutex_unlock(&vaccZones[zone_num]);
            continue;
        }
        pthread_mutex_unlock(&vaccZones[zone_num]);
        while(1)
        {
            pthread_mutex_lock(&vaccZones[zone_num]);
            if(batchRecieved[zone_num])
            {
                pthread_mutex_unlock(&vaccZones[zone_num]);
                break;
            }
            pthread_mutex_unlock(&vaccZones[zone_num]);
            pthread_mutex_lock(&stdCount);
            if(studentsLeft<=0)
            {
                pthread_mutex_unlock(&stdCount);
                return NULL;
            }
            pthread_mutex_unlock(&stdCount);
            for(int i=1;i<=n;i++)
            {
                pthread_mutex_lock(&pharmComps[i]);
                if(batchLeft[i]>0)
                {
                    batchLeft[i]--;
                    pthread_mutex_lock(&vaccZones[zone_num]);
                    vaccLeftZone[zone_num]=vaccBatch[i];
                    batchUse[zone_num] = i;
                    slotsAllocated[zone_num] = 0;
                    batchRecieved[zone_num] = 1;
                    pthread_mutex_unlock(&vaccZones[zone_num]);
                    pthread_mutex_unlock(&pharmComps[i]);
                    if(check==0)
                    {
                        printf("\nPharmaceutical Company %d is delivering a vaccine batch to Vaccination Zone %d which has success probability %f \n", i, zone_num, success[i]);
                        check = 1;
                    }
                    else
                        printf("\nPharmaceutical Company %d has delivered vaccines to Vaccination zone %d, resuming vaccinations now\n", i, zone_num);
                    break;
                }
                pthread_mutex_unlock(&pharmComps[i]);
            }
        }
        int maxi=8;
        pthread_mutex_lock(&stdCount);
        if(maxi>=notAllocated)
            maxi = notAllocated;
        pthread_mutex_unlock(&stdCount);
        pthread_mutex_lock(&vaccZones[zone_num]);
        if(maxi>=vaccLeftZone[zone_num])
            maxi = vaccLeftZone[zone_num];
        pthread_mutex_unlock(&vaccZones[zone_num]);
        if(maxi==0)
            continue;
        float s = 1 + (maxi) * (float)rand()/(float)RAND_MAX;
        int total = (int)s;
        if(total<=0)
            continue;
        pthread_mutex_lock(&vaccZones[zone_num]);
        if(studentsLeft>0)
            printf("\nVaccination Zone %d is ready to vaccinate with %d slots\n", zone_num, total);
        slotsLeft[zone_num] = total;
        pthread_mutex_unlock(&vaccZones[zone_num]);
        while(1)
        {
            pthread_mutex_lock(&stdCount);
            if(studentsLeft<=0)
            {
                pthread_mutex_unlock(&stdCount);
                return NULL;
            }
            pthread_mutex_unlock(&stdCount);
            pthread_mutex_lock(&vaccZones[zone_num]);
            if(slotsLeft[zone_num]<=0)
            {
                pthread_mutex_unlock(&vaccZones[zone_num]);
                break;
            }
            if(slotsAllocated[zone_num]<=0)
            {
                pthread_mutex_unlock(&vaccZones[zone_num]);
                continue;
            }
            pthread_mutex_unlock(&vaccZones[zone_num]);
            pthread_mutex_lock(&stdCount);
            if(notAllocated<=0)
            {
                pthread_mutex_unlock(&stdCount);
                break;
            }
            pthread_mutex_unlock(&stdCount);
        }
        pthread_mutex_lock(&vaccZones[zone_num]);
        if(studentsLeft>0)
        {
            vaccPhase[zone_num] = 1;
            printf("\nVaccination Zone %d entering Vaccination Phase\n", zone_num);
        }
        pthread_mutex_unlock(&vaccZones[zone_num]);
    }
}
void* Company(void* inp)
{
    int comp_num = *(int*)inp;
    batchLeft[comp_num] = 0;
    vaccLeftComp[comp_num] = 0;
    int vaccineL = 10, vaccineU = 20;
    int timeL = 2, timeU = 5;
    int batchL = 1, batchU = 5;
    while(1)
    {
        pthread_mutex_lock(&stdCount);
        if(studentsLeft<=0)
        {
            pthread_mutex_unlock(&stdCount);
            return NULL;
        }
        pthread_mutex_unlock(&stdCount);
        pthread_mutex_lock(&pharmComps[comp_num]);
        if(batchLeft[comp_num]!=0 || vaccLeftComp[comp_num]!=0)
        {
            pthread_mutex_unlock(&pharmComps[comp_num]);
            continue;
        }
        pthread_mutex_unlock(&pharmComps[comp_num]);
        float b = batchL + (batchU - batchL + 1) * (float)rand()/RAND_MAX;
        float t = timeL + (timeU - timeL) * (float)rand()/RAND_MAX;
        printf("\nPharmaceutical Company %d is preparing %d batches of vaccines which have success probability %f\n", comp_num, (int)b, success[comp_num]);
        sleep((int)t);
        printf("\nPharmaceutical Company %d has prepared %d batches of vaccines which have success probability %f. Waiting for all the vaccines to be used to resume production\n", comp_num, (int)b, success[comp_num]);
        float num = vaccineL + (vaccineU - vaccineL + 1) * (float)rand()/RAND_MAX;
        vaccBatch[comp_num] = (int)num;
        pthread_mutex_lock(&pharmComps[comp_num]);
        batchLeft[comp_num] = (int)b;
        vaccLeftComp[comp_num] = (int)b * vaccBatch[comp_num];
        pthread_mutex_unlock(&pharmComps[comp_num]);
    }
}
int main()
{
    srand(time(0));
    scanf("%d%d%d", &n, &m, &o);
    assert(n>=0 && m>=0 && o>=0);
    for(int i=1;i<=n;i++)
    {
        scanf("%f", &success[i]);
        assert(success[i]-0>=0.000001 && success[i]-1<=0.000001);
    }
    if(o==0)
    {
        printf("\nSimulation Finished\n");
        return 0;
    }
    if(n==0)
    {
        printf("\nSimulation Finished as there are no pharmacutical companies to manufacture vaccines\n");
        return 0;
    }
    if(m==0)
    {
        printf("\nSimulation Finished as there are no vaccination zones where students gets vaccinated\n");
        return 0;
    }
    studentsLeft = o; notAllocated = 0;
    pthread_t companies[3001], zones[3001], stds[3001];
    pthread_mutex_init(&stdCount, NULL);
    int comp_id[3001], zone_id[3001], std_id[3001];
    for(int i=1;i<=n;i++)
    {
        comp_id[i] = i;
        pthread_mutex_init(&pharmComps[i], NULL);
        pthread_create(&companies[i], NULL, Company, (void*)&comp_id[i]);
    }
    for(int i=1;i<=m;i++)
    {
        zone_id[i]=i;
        pthread_mutex_init(&vaccZones[i], NULL);
        pthread_create(&zones[i], NULL, VaccZone, (void*)&zone_id[i]);
    }
    for(int i=1;i<=o;i++)
    {
        std_id[i]=i;
        pthread_create(&stds[i], NULL, Student, (void*)&std_id[i]);
    }
    for(int i=1;i<=n;i++)
    {
        pthread_join(companies[i], NULL);
        pthread_mutex_destroy(&pharmComps[i]);
    }
    for(int i=1;i<=m;i++)
    {
        pthread_join(zones[i], NULL);
        pthread_mutex_destroy(&vaccZones[i]);
    }
    for(int i=1;i<=o;i++)
        pthread_join(stds[i], NULL);
    pthread_mutex_destroy(&stdCount);
    printf("\nSimulation Finished\n");
    return 0;
}