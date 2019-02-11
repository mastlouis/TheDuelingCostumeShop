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
  int blockPirates;
  int blockNinjas;
  int maxPiratesTime;
  int maxNinjasTime;
} ShopData;

typedef struct AdventurerData_{
  ShopData* theShop;
  int isArr;
  int minutesWaiting;
  pthread_t* theThread;
} AdventurerData;

double getRandNormNum(double avg);

void costumeDept(AdventurerData *person){
	//Extract the data from the person struct to make code more readable
	ShopData *costumeShop = person->theShop;
  int needsCostume = 1;
  int isEntering = 0;
  while(needsCostume){
    //Aaaadventure time
    if(person->isArr){
      sleep(getRandNormNum(costumeShop->avgArrPirate));
    }
    else sleep(getRandNormNum(costumeShop->avgArrNinja));

    printf("Walk up to the shop like what up I got a big sword\n");
    while(!isEntering){
      sem_wait(costumeShop->doorLock);
      if(person->isArr){
        if(!costumeShop->ninjasInShop
        			&& costumeShop->numTeamsAvailable > 0
        			&& !costumeShop->blockPirates){
          costumeShop->numTeamsAvailable--;
          costumeShop->piratesInShop++;
          isEntering = 1;
        }
        else if(person->minutesWaiting == 0){
          costumeShop->numPiratesWaiting++;
        }
      }
      else{
        if(!costumeShop->piratesInShop
        			&& costumeShop->numTeamsAvailable > 0
        			&& !costumeShop->blockNinjas){
          costumeShop->numTeamsAvailable--;
          costumeShop->ninjasInShop++;
          isEntering = 1;
        }
        else if(person->minutesWaiting == 0){
          costumeShop->numNinjasWaiting++;
        }
      }
      sem_post(costumeShop->doorLock);
      person->minutesWaiting++;
      if(!isEntering)
        sleep(1);
    }

    //if there is an available team, go into the costume shop
    //Wait should never actually put a thread to sleep
    printf("We be in the shop bois\n");
    sem_wait(costumeShop->teams);
    if(person->isArr){
      sleep(getRandNormNum(costumeShop->avgCostPirate));
    }
    else sleep(getRandNormNum(costumeShop->avgCostNinja));
    //relinquish a team
    sem_post(costumeShop->teams);

    //leave the costume shop
    sem_wait(costumeShop->doorLock);
    costumeShop->numTeamsAvailable++;
    if(person->isArr){
      costumeShop->piratesInShop--;
      if(person->minutesWaiting)
        costumeShop->numPiratesWaiting--;
    }
    else{
      costumeShop->ninjasInShop--;
      if(person->minutesWaiting)
        costumeShop->numNinjasWaiting--;
    }
    sem_wait(costumeShop->doorLock);

    //let there be a 25% chance that the pirate or ninja will return to the costume shop
    if(!rand()%4){
      needsCostume = 0;
    }
  }
  printf("Here comes the reaper\n");
  pthread_exit(NULL);
}

double getRandNormNum(double avg){
  double num = avg*(sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48()) + 1);
  while(num < 0){
    num = avg*(sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48()) + 1);
  }
  return num;
}

int main(int argc, char* argv[]){
	AdventurerData** theAdventurers;
  srand(time(0));
  srand48(time(0));
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
  costumeShop->ninjasInShop = 0;
  costumeShop->piratesInShop = 0;
  costumeShop->avgArrNinja = avgArrNinja;
  costumeShop->avgArrPirate = avgArrPirate;
  costumeShop->avgCostNinja = avgCostNinja;
  costumeShop->avgCostPirate = avgCostPirate;

  //Variables for fairness
  costumeShop->blockPirates = 0;
  costumeShop->blockNinjas = 0;
  /*
  * Default maximum time for each side is 30 minutes.
  * I don't know if there's an advantage to that, but
  * I assume that there's some math that works out
  * nicely.
  */
  costumeShop->maxPiratesTime = MAX_SIDE_CHANGE;
  costumeShop->maxNinjasTime = MAX_SIDE_CHANGE;
  /*
  * If there's enough time to switch before anyone
  * starts getting free costuming, do that.
  */
  if((MAX_SIDE_CHANGE - avgCostNinja) > 0)
 		costumeShop->maxNinjasTime = MAX_SIDE_CHANGE - avgCostNinja;
 	if((MAX_SIDE_CHANGE - avgCostPirate) > 0)
 		costumeShop->maxPiratesTime = MAX_SIDE_CHANGE - avgCostPirate;

  sem_init(costumeShop->teams, 0, numTeams);
  sem_init(costumeShop->doorLock, 0, 1);

  for(int i = 0; i < numPirates; i++){
    AdventurerData* threadData = (AdventurerData*) malloc(sizeof(AdventurerData));
    theAdventurers[i] = threadData;
    threadData->theShop = costumeShop;
    threadData->isArr = 1;
    threadData->minutesWaiting = 0;
    pthread_t* pirate = (pthread_t*) malloc(sizeof(pthread_t));
    threadData->theThread = pirate;
    pthread_create(pirate, NULL, (void*) &costumeDept, threadData);
  }
  for(int i = 0; i < numNinjas; i++){
    AdventurerData* threadData = (AdventurerData*) malloc(sizeof(AdventurerData));
    theAdventurers[numPirates + i] = threadData;
    threadData->theShop = costumeShop;
    threadData->isArr = 0;
    threadData->minutesWaiting = 0;
    pthread_t* ninja = (pthread_t*) malloc(sizeof(pthread_t));
    threadData->theThread = ninja;
    pthread_create(ninja, NULL, (void*) &costumeDept, threadData);
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
