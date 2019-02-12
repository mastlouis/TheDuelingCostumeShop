//Matthew St. Louis (mastlouis), Fay Whittall (fxwhittall)
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
//#include <sys/time.h>

#define MAX_SIDE_CHANGE (30)

typedef struct ShopData_{
  //Parameters for the program
  int numPirates;
  int numNinjas;
  int numTeams;
  double avgCostPirate, avgCostNinja, avgArrPirate, avgArrNinja;

  //Semaphores
  sem_t *doorLock;
  sem_t *teams;

  //Information about the people in and waiting for the shop

  int ninjasInShop;
  int piratesInShop;
  int numTeamsAvailable;
  int numPiratesWaiting;
  int numNinjasWaiting;

  //Infomation for controlling when ninjas or pirates can enter

  int blockPirates; //Whether or not the pirates should be allowed to enter
  int blockNinjas; //Whether or not the ninjas should be allowed to enter
  int ninjaMaxWait; //The maximum time for which pirates may control the shop
  int pirateMaxWait; //The maximum time for which ninjas may control the shop
  int ninjaWaitMultiplier; //The scale factor for ninjaMaxWait (in case of backup)
  int pirateWaitMultiplier; //The scale factor for pirateMaxWait (in case of backup)
} ShopData;

typedef struct AdventurerData_{
  ShopData* theShop;
  int isArr;
  int minutesWaiting;
  pthread_t* theThread;
  int numVisits;
  int timeInShop;
  int totalTimeInShop;
  int totalTimeWaiting;
} AdventurerData;

double getRandNormNum(double avg);

void costumeDept(AdventurerData *person){
  //Extract the data from the person struct to make code more readable
  ShopData *costumeShop = person->theShop;
  int needsCostume = 1;
  int isEntering = 0;
  while(needsCostume){
    //Aaaadventure time
    if(person->isArr) sleep(getRandNormNum(costumeShop->avgArrPirate));
    else sleep(getRandNormNum(costumeShop->avgArrNinja));

    printf("Walk up to the shop like what up I got a big sword\n");
    while(!isEntering){
      sem_wait(costumeShop->doorLock);
      //If the person is a pirate
      if(person->isArr){
        //If the pirate can enter the shop
        if(!costumeShop->ninjasInShop
            && costumeShop->numTeamsAvailable > 0
            && !costumeShop->blockPirates){
          costumeShop->numTeamsAvailable--;
          costumeShop->piratesInShop++;
          isEntering = 1;
          //If the pirate was counted in pirates waiting
          if(person->minutesWaiting) costumeShop->numPiratesWaiting--;
        }
        //If the pirate cannot enter the shop
        else if(person->minutesWaiting == 0){
           costumeShop->numPiratesWaiting++;
           //If the shop may lose money by not letting in this Pirate soon
          if(person->minutesWaiting 
              >= (costumeShop->pirateMaxWait * costumeShop->pirateWaitMultiplier) - 1
              && !costumeShop->blockNinjas){
            //Block ninjas but let in pirates
            costumeShop->blockNinjas = 1;
            costumeShop->blockPirates = 0;
            printf("A pirate has blocked the ninjas. New multiplier: %d\n", costumeShop->pirateWaitMultiplier + 1);
            //Increase the threshold for pirates to block
            costumeShop->pirateWaitMultiplier++;
          }
        }
      }
      //If the person is a ninja
      else{
        //If ninja can enter the shop
        if(!costumeShop->piratesInShop
            && costumeShop->numTeamsAvailable > 0
            && !costumeShop->blockNinjas){
          costumeShop->numTeamsAvailable--;
          costumeShop->ninjasInShop++;
          isEntering = 1;
          //If the person was counted in ninjas waiting
          if(person->minutesWaiting) costumeShop->numNinjasWaiting--;
        }
        //If the ninja cannot enter the shop
        else if(person->minutesWaiting == 0){
          costumeShop->numNinjasWaiting++;
          //If the shop may lose money by not letting in this Ninja soon
          if(person->minutesWaiting 
              >= (costumeShop->ninjaMaxWait * costumeShop->ninjaWaitMultiplier) - 1
              && !costumeShop->blockPirates){
            //Block pirates but let in ninjas
            costumeShop->blockPirates = 1;
            costumeShop->blockNinjas = 0;
            printf("A ninja has blocked the pirates. New Multiplier: %d\n", costumeShop->ninjaWaitMultiplier + 1);
            //Increase the threshold for ninjas to block again
            costumeShop->ninjaWaitMultiplier++;
          }
        }
      }
      //release control of the doors
      sem_post(costumeShop->doorLock);

      if(!isEntering){
        person->minutesWaiting++;
        sleep(1);
      }
    }

    printf("We be in the shop bois %d %d\n", costumeShop->piratesInShop, costumeShop->ninjasInShop); 
    if(person->isArr) printf("Pirate entered!\n");
    else printf("Ninja Entered!\n");
    
    //if there is an available team, go into the costume shop
    //Wait should never actually put a thread to sleep
    sem_wait(costumeShop->teams);
    if(person->isArr){
      person->timeInShop = getRandNormNum(costumeShop->avgCostPirate);
      sleep(person->timeInShop);
    }
    else{
      person->timeInShop = getRandNormNum(costumeShop->avgCostNinja);
      sleep(person->timeInShop);
    }
    //relinquish a team
    sem_post(costumeShop->teams);

    sem_wait(costumeShop->doorLock);
    printf("Lookin' spiffy, as always ;)\n");
    //leave the costume shop
    costumeShop->numTeamsAvailable++;

    //If the person leaving is a pirate
    if(person->isArr){
      printf("Pirate Left!\n");
      costumeShop->piratesInShop--;
      //If there are no pirates waiting
      if(costumeShop->numPiratesWaiting == 0)
        costumeShop->pirateWaitMultiplier = 1;
    }
    //If the person leaving is a ninja
    else{
      printf("Ninja Left!\n");
      costumeShop->ninjasInShop--;
      //If there are no ninjas waiting
      if(costumeShop->numNinjasWaiting == 0)
        costumeShop->ninjaWaitMultiplier = 1;
    }
    sem_post(costumeShop->doorLock);

    //let there be a 25% chance that the pirate or ninja will return to the costume shop
    if(rand()%4){
      needsCostume = 0;
      printf("I know my lines, b*tch %d %d\n", costumeShop->piratesInShop, costumeShop->ninjasInShop);
    }
    //Path for pirates or ninjas that plan to come back later
    else{
      isEntering = 0;
      person->totalTimeWaiting += person->minutesWaiting;
      person->minutesWaiting = 0;
      person->numVisits++;
      person->totalTimeInShop = person->timeInShop;
      person->timeInShop = 0;
      printf("gotta get those lines right%d %d\n", costumeShop->piratesInShop, costumeShop->ninjasInShop);
    }
  }
  printf("Here comes the reaper\n");
  pthread_exit(NULL);
}

double getRandNormNum(double avg){
  double num = avg*(sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48()) + 1);
  while(num < 0 || num > avg*2){
    num = avg*(sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48()) + 1);
  }
  return num;
}

int main(int argc, char* argv[]){
  AdventurerData** theAdventurers;
  srand(time(0));
  srand48(1);
  int numTeams, numPirates, numNinjas;
  double avgCostPirate, avgCostNinja, avgArrPirate, avgArrNinja;
  ShopData *costumeShop;
  if(argc != 8){ //incorrect number of arguments
    printf("Incorrect argument entry! Arguments are are follows:\n");
    printf("\t ./part1 <number of teams> <number of pirates> <number of ninjas> <average pirate costuming time> <average ninja costuming time> <average pirate arrival time> <average ninja arrival time>\n");
    return 1;
  }
  numTeams = atoi(argv[1]);
  if(numTeams > 4 || numTeams < 2){
    printf("Error! There can be between 2 and 4 teams.\n");
    return 1;
  }
  numPirates = atoi(argv[2]);
  if(numPirates > 50 || numPirates < 10){
    printf("Error! There can be between 10 and 50 pirates.\n");
    return 1;
  }
  numNinjas = atoi(argv[3]);
  if(numNinjas > 50 || numNinjas < 10){
    printf("Error! There can be between 10 and 50 ninjas.\n");
    return 1;
  }
  avgCostPirate = atof(argv[4]);
  avgCostNinja = atof(argv[5]);
  avgArrPirate = atof(argv[6]);
  avgArrNinja = atof(argv[7]);

  costumeShop = (ShopData*) malloc(sizeof(ShopData));
  theAdventurers = (AdventurerData**) calloc((numPirates + numNinjas), sizeof(AdventurerData*));

  costumeShop->doorLock = (sem_t*) malloc(sizeof(sem_t));
  costumeShop->teams = (sem_t*) malloc(sizeof(sem_t));

  costumeShop->numPirates = numPirates;
  costumeShop->numNinjas = numNinjas;
  costumeShop->numTeams = numTeams;
  costumeShop->numTeamsAvailable = numTeams;
  costumeShop->numPiratesWaiting = 0;
  costumeShop->numNinjasWaiting = 0;
  costumeShop->ninjasInShop = 0;
  costumeShop->piratesInShop = 0;
  costumeShop->avgArrNinja = avgArrNinja;
  costumeShop->avgArrPirate = avgArrPirate;
  costumeShop->avgCostNinja = avgCostNinja;
  costumeShop->avgCostPirate = avgCostPirate;

  //Variables for fairness
  costumeShop->blockPirates = 0;
  costumeShop->blockNinjas = 0;
  costumeShop->ninjaWaitMultiplier = 1;
  costumeShop->pirateWaitMultiplier = 1;
  /*
  * Default maximum time for each side is 30 minutes.
  * I don't know if there's an advantage to that, but
  * I assume that there's some math that works out
  * nicely.
  */
  costumeShop->ninjaMaxWait = MAX_SIDE_CHANGE;
  costumeShop->pirateMaxWait = MAX_SIDE_CHANGE;
  /*
  * If there's enough time to switch before anyone
  * starts getting free costuming, do that.
  */
  if((MAX_SIDE_CHANGE - avgCostNinja) > 0)
  costumeShop->pirateMaxWait = MAX_SIDE_CHANGE - avgCostNinja;
  if((MAX_SIDE_CHANGE - avgCostPirate) > 0)
  costumeShop->ninjaMaxWait = MAX_SIDE_CHANGE - avgCostPirate;

  sem_init(costumeShop->teams, 0, numTeams);
  sem_init(costumeShop->doorLock, 0, 1);

  for(int i = 0; i < numPirates + numNinjas; i++){
    AdventurerData* threadData = (AdventurerData*) malloc(sizeof(AdventurerData));
    theAdventurers[i] = threadData;
    //Give each thread a copy of the pointer to the costume shop
    threadData->theShop = costumeShop;

    //Tell the thread whether or not it's a pirate
    if(i < numPirates) threadData->isArr = 1;
    else threadData->isArr = 0;

    //Initialize data values
    threadData->minutesWaiting = 0;
    threadData->numVisits = 0;
    threadData->totalTimeInShop = 0;
    threadData->totalTimeWaiting = 0;
    threadData->timeInShop = 0;
    threadData->theThread = (pthread_t*) malloc(sizeof(pthread_t));
    pthread_create(threadData->theThread, NULL, (void*) &costumeDept, threadData);
  }

  //Join the threads
  for(int i = 0; i < numPirates + numNinjas; i++){
    pthread_join(*(theAdventurers[i]->theThread), NULL);
  }
  //Free the threads here if that's something we should do

  for(int i = 0; i < (numNinjas + numPirates); i++){
    free(theAdventurers[i]);
  }
  free(theAdventurers);
  free(costumeShop);

  return 0;
}
