//Matthew St. Louis (mastlouis), Fay Whittall (fxwhittall)
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/time.h>

#define MAX_SIDE_CHANGE (30)
#define MAX_VISITS (20)

typedef struct ShopData_{
  //Parameters for the program
  int numPirates;
  int numNinjas;
  int numTeams;
  int* teamsAvailable; //Ints representing whether or not each team is available
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

  //Information for average queue size

  struct timeval *initialTime;
  struct timeval *lastUpdate;
  struct timeval *endTime;
  double peopleTimesTime;

} ShopData;

typedef struct VisitData_{
  int teamUsed;
  int moneySpent;
  int timeWaited;
  int timeInShop;
} VisitData;

typedef struct AdventurerData_{
  ShopData* theShop;
  int isArr;
  int id;
  int minutesWaiting;
  pthread_t* theThread;
  int numVisits;
  int timeInShop;
  int totalTimeInShop;
  int totalTimeWaiting;
  int totalMoneySpent;

  VisitData** visits;

} AdventurerData;

double getRandNormNum(double avg);
char* getPirateName(int i);
char* getNinjaName(int i);
void printStatistics(AdventurerData **theAdventurers, int numPirates, int numNinjas);
int getNextTeam(int numTeams, int* teamsAvailable);

void costumeDept(AdventurerData *person){
  //Extract the data from the person struct to make code more readable
  ShopData *costumeShop = person->theShop;
  int needsCostume = 1;
  int isEntering = 0;
  long int secondsElapsed = 0;
  long int uSecondsElapsed = 0;
  struct timeval currentTime;
  while(needsCostume){
    //Aaaadventure time
    if(person->isArr) sleep(getRandNormNum(costumeShop->avgArrPirate));
    else sleep(getRandNormNum(costumeShop->avgArrNinja));

    while(!isEntering){
      sem_wait(costumeShop->doorLock);

      //Record some statistics about the line
      gettimeofday(&currentTime, NULL);
      secondsElapsed = currentTime.tv_sec - costumeShop->lastUpdate->tv_sec;
      uSecondsElapsed = currentTime.tv_usec - costumeShop->lastUpdate->tv_usec;

      //Add the number of people waiting scaled by the time for which 
      //they've been waiting to the running total
      costumeShop->peopleTimesTime += (costumeShop->numPiratesWaiting + costumeShop->numNinjasWaiting) * 
          (secondsElapsed + (((double) uSecondsElapsed) / 1000000.0));

      //Prepare time statistics for the next thread
      costumeShop->lastUpdate->tv_sec = currentTime.tv_sec;
      costumeShop->lastUpdate->tv_usec = currentTime.tv_usec;

      /*
      Unfortunately, the separation of pirates and ninjas into separate
      variables necessitates large separate blocks for each faction.
      */

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
        else {
          //If the pirate has just started waiting
          if(person->minutesWaiting == 0)
            costumeShop->numPiratesWaiting++;
          //If the shop may lose money by not letting in this Pirate soon
          if(person->minutesWaiting 
              >= ((costumeShop->pirateMaxWait * costumeShop->pirateWaitMultiplier) - 1)
              && !costumeShop->blockNinjas){
            //Block ninjas but let in pirates
            costumeShop->blockNinjas = 1;
            costumeShop->blockPirates = 0;
            printf("A pirate has blocked the ninjas. New multiplier: %d\n", costumeShop->pirateWaitMultiplier + 1);
            //Increase the threshold for pirates to block
            costumeShop->pirateWaitMultiplier++;
            costumeShop->ninjaWaitMultiplier++;
          }
        }
      }

      //If the person is a ninja
      else{
        //If the ninja can enter the shop
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
        else {
          //If the ninja has just started waiting
          if(person->minutesWaiting == 0)
            costumeShop->numNinjasWaiting++;
          //If the shop may lose money by not letting in this Ninja soon
          if((person->minutesWaiting 
              >= ((costumeShop->ninjaMaxWait * costumeShop->ninjaWaitMultiplier) - 1))
              && !costumeShop->blockPirates){
            //Block pirates but let in ninjas
            costumeShop->blockPirates = 1;
            costumeShop->blockNinjas = 0;
            printf("A ninja has blocked the pirates. New Multiplier: %d\n", costumeShop->ninjaWaitMultiplier + 1);
            //Increase the threshold for ninjas to block again
            costumeShop->ninjaWaitMultiplier++;
            costumeShop->pirateWaitMultiplier++;
          }
        }
      }

      if(isEntering){
        person->visits[person->numVisits] = (VisitData*) malloc(sizeof(VisitData));
        if((person->visits[person->numVisits]->teamUsed 
              = getNextTeam(costumeShop->numTeams, costumeShop->teamsAvailable)) == -1)
          //In case something went wrong and no teams are available
          printf(" No teams available, somehow\n");
      }

      //release control of the doors
      sem_post(costumeShop->doorLock);

      if(!isEntering){
        person->minutesWaiting++;
        sleep(1);
      }
      
    }

    //printf("Pirates: %d Ninjas: %d\n", costumeShop->piratesInShop, costumeShop->ninjasInShop); 
    if(person->isArr) printf("%s the pirate is entering the shop with team %d. Time waited: %d\n"
        , getPirateName(person->id), person->visits[person->numVisits]->teamUsed, person->minutesWaiting);
    else printf("%s the ninja is entering the shop with team %d. Time waited: %d\n"
        , getNinjaName(person->id - costumeShop->numPirates), person->visits[person->numVisits]->teamUsed, person->minutesWaiting);

    
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

    //Grab the door lock to leave the shop
    sem_wait(costumeShop->doorLock);


    //relinquish a team
    sem_post(costumeShop->teams);
    costumeShop->numTeamsAvailable++;

    //If the person leaving is a pirate
    if(person->isArr){
      costumeShop->piratesInShop--;
      //If there are no pirates waiting
      if(costumeShop->numPiratesWaiting == 0){
        costumeShop->pirateWaitMultiplier = 1;
        costumeShop->blockNinjas = 0;
      }

    }

    //If the person leaving is a ninja
    else{
      costumeShop->ninjasInShop--;
      //If there are no ninjas waiting
      if(costumeShop->numNinjasWaiting == 0)
        costumeShop->ninjaWaitMultiplier = 1;
        costumeShop->blockPirates = 0;
    }

    //Mark the team used as available
    if(person->visits[person->numVisits]->teamUsed >= 0)
      costumeShop->teamsAvailable[person->visits[person->numVisits]->teamUsed] = 0;

    sem_post(costumeShop->doorLock);

    //Gather those statistics
    //Doesn't change costumeShop data, so locking unnecessary
    person->visits[person->numVisits]->timeWaited = person->minutesWaiting;
    person->visits[person->numVisits]->timeInShop = person->timeInShop;
    person->visits[person->numVisits]->moneySpent = 0;
    if(person->minutesWaiting < 30)
      person->visits[person->numVisits]->moneySpent = person->timeInShop;
    person->totalTimeInShop += person->timeInShop;
    person->totalTimeWaiting += person->minutesWaiting;
    person->totalMoneySpent += person->visits[person->numVisits]->moneySpent;
    person->numVisits++;
    //let there be a 25% chance that the pirate or ninja will return to the costume shop
    if(rand()%4 || person->numVisits >= MAX_VISITS){
      needsCostume = 0;
    }
    //Path for pirates or ninjas that plan to come back later
    else{
      isEntering = 0;
      person->minutesWaiting = 0;
      person->timeInShop = 0;
    }
  }
  //If a thread leaves the while loop, it's done executing
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

  costumeShop->teamsAvailable = (int *) calloc(numTeams, sizeof(int*));
  for(int i = 0; i < numTeams; i++){
    costumeShop->teamsAvailable[i] = 0;
    //0 means that a team is free, while 1 means that the team is occupied
  }

  //Variables for statistics
  costumeShop->initialTime = malloc(sizeof(struct timeval));
  costumeShop->lastUpdate = malloc(sizeof(struct timeval));
  costumeShop->endTime = malloc(sizeof(struct timeval));
  costumeShop->peopleTimesTime = 0;
  gettimeofday(costumeShop->initialTime, NULL);
  gettimeofday(costumeShop->lastUpdate, NULL);

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
    threadData->totalMoneySpent = 0;
    threadData->id = i;

    //Initialize the array of visits
    threadData->visits = (VisitData**) calloc(MAX_VISITS, sizeof(VisitData*));

    //Make the thread and send it on its way
    threadData->theThread = (pthread_t*) malloc(sizeof(pthread_t));
    pthread_create(threadData->theThread, NULL, (void*) &costumeDept, threadData);
  }

  //Join the threads
  for(int i = 0; i < numPirates + numNinjas; i++){
    pthread_join(*(theAdventurers[i]->theThread), NULL);
  }
  gettimeofday(costumeShop->endTime, NULL);
  printStatistics(theAdventurers, numPirates, numNinjas);

  //Free the threads here if that's something we should do

  for(int i = 0; i < (numNinjas + numPirates); i++){
    free(theAdventurers[i]->theThread);
    for(int j = 0; j < theAdventurers[i]->numVisits; j++)
      free(theAdventurers[i]->visits[j]);
    free(theAdventurers[i]->visits);
    free(theAdventurers[i]);
  }
  free(theAdventurers);

  /*free(costumeShop->teamsAvailable);
  free(costumeShop->initialTime);
  free(costumeShop->lastUpdate);
  free(costumeShop->endTime);
  free(costumeShop->doorLock);
  free(costumeShop->teams);
  free(costumeShop);*/

  return 0;
}

/*
* Takes as parameters the number of costuming teams
* and an array of the availability of the costuming
* teams, wherein 0 represents a free team and 1
* represents a busy team. 
* Also changes the availability of the returned team
* from free to busy.
* 
* Returns the index of the next available team or 
* -1 if all teams are busy.
*/
int getNextTeam(int numTeams, int* teamsAvailable){
  //Uses a static variable to distribute the work
  //more evenly amongst the teams
  static int nextTeam = 0;
  int initialTeam = nextTeam;
  int teamToReturn = -1;
  while(teamToReturn == -1){
    if(teamsAvailable[nextTeam] == 0){
      teamsAvailable[nextTeam] = 1;
      teamToReturn = nextTeam;
    }
    nextTeam++;
    nextTeam %= numTeams;
    if(nextTeam == initialTeam)
      return -1;
  }
  return teamToReturn;
}

void printStatistics(AdventurerData **theAdventurers, int numPirates, int numNinjas){
  ShopData* theShop;
  int *teamTimeBusy;
  int grossRevenue = 0;
  int totalVisits = 0;
  double timeElapsed = 0;
  printf("\n\n**** Statistics ****\n\n\n");

  if(numPirates + numNinjas > 0){
    theShop = theAdventurers[0]->theShop;
    teamTimeBusy = (int*) calloc(theShop->numTeams, sizeof(int));
  }
  else{
    printf("There were no pirates or ninjas. Nothing to report.\n");
    return;
  }
  //Itemized bills for each pirate and ninja

  //For each person:
    //Number of visits
    //For each visit
      //Time for each visit
      //Wait times
    //Total gold owed
  for(int i = 0; i < numPirates + numNinjas; i++){
    if(i < numPirates){
      printf("%19s the pirate went to the shop %d time%s\n", getPirateName(i), 
        theAdventurers[i]->numVisits, (theAdventurers[i]->numVisits > 1)? ".": "s.");
    }
    else{
      printf("%20s the ninja went to the shop %d times.\n", getNinjaName(i - numPirates), theAdventurers[i]->numVisits);
    }
    totalVisits += theAdventurers[i]->numVisits;
    for(int j = 0; j < theAdventurers[i]->numVisits; j++){
      printf("\tVisit %d wait time: %d\n", j, theAdventurers[i]->visits[j]->timeWaited);
      printf("\t\tTeam used: %d", theAdventurers[i]->visits[j]->teamUsed); //Lack of new line intended
      printf("  Time in store: %d\n", theAdventurers[i]->visits[j]->timeInShop);
      teamTimeBusy[theAdventurers[i]->visits[j]->teamUsed] += theAdventurers[i]->visits[j]->timeInShop;
    }
    printf("\tTotal Gold owed: %d\n", theAdventurers[i]->totalMoneySpent);
    grossRevenue += theAdventurers[i]->totalMoneySpent;
  }

  //Expenses (5 gold/team/day)
  //Amount of time each team is busy
  //Average queue length
  //Gross revenue
  //Gold per visit
  //Total profits

  
  printf("The shop spent %d gold on %d teams\n", theShop->numTeams * 5, theShop->numTeams);
  printf("Gross revenue: %d\n", grossRevenue);
  printf("Total visits: %d\n", totalVisits);
  printf("Gold per visit: %f\n", ((double) grossRevenue) / ((double) totalVisits));
  printf("Total profit: %d\n", grossRevenue - (5 * theShop->numTeams));

  timeElapsed = (theShop->endTime->tv_sec - theShop->initialTime->tv_sec) + (0.000001 * (theShop->endTime->tv_usec - theShop->initialTime->tv_usec));
  printf("The shop was open for %f minutes\n", timeElapsed);
  printf("Average line size was %f people\n", theShop->peopleTimesTime / timeElapsed);

  //printf("Team time busy array: %p\n", teamTimeBusy);
  for(int i = 0; i < theShop->numTeams; i++){
    printf("Team %d was busy for %d minutes and free for %f minutes\n", i, teamTimeBusy[i], timeElapsed - teamTimeBusy[i]);
  }

  //free(teamTimeBusy);
}

char* getPirateName(int i){
  switch (i){
    case(0): return "Mr. Scurvington (sr.)";
    case(1): return "Arrthurr";
    case(2): return "Bearded Dave";
    case(3): return "The Flying Dutchman";
    case(4): return "Scurvy Jim";
    case(5): return "Arrnold";
    case(6): return "The Walking Dutchman";
    case(7): return "Mr. Scurvington (jr.)";
    case(8): return "The Crawling Dutchman";
    case(9): return "Scurvy Tom";
    case(10): return "Howling Harrrold";
    case(11): return "The Laying Dutchman";
    case(12): return "Arrrline";
    case(13): return "Quatheryne";
    case(14): return "ma~ (pronounced Matilda)";
    case(15): return "Johnny Depp";
    case(16): return "Patchy";
    case(17): return "Peggy";
    case(18): return "Candybeard";
    case(19): return "Nabstr";
    case(38): return "Professor Shue";
    case(39): return "Professor Walls";
    default: return "Pirate McPirateson";
  }
}

char* getNinjaName(int i){
  switch(i){
    case(0): return "...";
    case(1): return "Swift Wind";
    case(2): return "Donatello";
    case(3): return "Michaelangelo";
    case(4): return "Rafael";
    case(5): return "Leonardo";
    case(6): return "Bruce Lee";
    case(7): return "Danny Avidan";
    case(8): return "Striking Kobra";
    case(9): return "Silent Lightning";
    case(10): return "Kung Foo Panda";
    case(11): return "Kung Bar Panda";
    case(12): return "Kung Baz Panda";
    case(13): return "Dave";
    case(14): return "Sasuke";
    case(15): return "That girl from Kingsman";
    case(16): return "That DC superhero Katana";
    case(49): return "Smoke Weed";
    //Make the TA's ninjas
    default: return "Ninja Ninjington";
  }
}
