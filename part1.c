//Matthew St. Louis (mastlouis), Fay Whittall (fxwhittall)
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

typedef struct shopData_{
  int numPirates;
  int numNinjas;
  int numTeams;
  double avgCostPirate, avgCostNinja, avgArrPirate, avgArrNinja;
  sem_t *doorLock;
  int ninjasInShop;
  int piratesInShop;
} shopData;

double getRandNormNum(double avg);

void costumeDept(int isArr, shopData* costumeShop){
  int needsCostume = 1;
  int isEntering = 0;
  while(needsCostume){
    //Aaaadventure time
    if(isArr){
      sleep(getRandNormNum(costumeShop->avgArrPirate));
    }
    else sleep(getRandNormNum(costumeShop->avgArrNinja));

    //Walk up to the shop like what up I got a big sword
    while(!isEntering){
      sem_wait(costumeShop->doorLock);
      if(isArr){
        if(!costumeShop->ninjasInShop){
          costumeShop->piratesInShop++;
          isEntering = 1;
        }
        sem_post(costumeShop->doorLock);
      }
      else{
        if(!costumeShop->piratesInShop){
          costumeShop->ninjasInShop++;
          isEntering = 1;
        }
        sem_post(costumeShop->doorLock);
      }
    }

    //if there is an available team, go into the costume shop
    sem_wait(&teams);
    if(isArr){
      sleep(getRandNormNum(costumeShop->avgCostPirate));
    }
    else sleep(getRandNormNum(costumeShop->avgCostNinja));
    sem_post(&teams);

    //leave the costume shop
    sem_wait(costumeShop->doorLock);
    if(isArr){
      costumeShop->piratesInShop--;
    }
    else costumeShop->ninjasInShop--;
    sem_wait(costumeShop->doorLock);

    //let there be a 25% chance that the pirate or ninja will return to the costume shop
    if(!rand()%4){
      needsCostume = 0;
    }
  }
}

double getRandNormNum(double avg){
  double num = avg*(sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48()) + 1);
  while(num < 0){
    num = avg*(sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48()) + 1);
  }
  return num;
}

int main(int argc, char* argv[]){
  sem_t teams;
  srand(time(0));
  srand48(time(0));
  int numTeams, numPirates, numNinjas;
  double avgCostPirate, avgCostNinja, avgArrPirate, avgArrNinja;
  shopData *costumeShop;
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

  *costumeShop = (shopData*) malloc(sizeof(shopData));

  costumeShop->numPirates = numPirates;
  costumeShop->numNinjas = numNinjas;
  costumeShop->numTeams = numTeams;
  costumeShop->ninjasInShop = 0;
  costumeShop->piratesInShop = 0;
  costumeShop->avgArrNinja = avgArrNinja;
  costumeShop->avgArrPirate = avgArrPirate;
  costumeShop->avgCostNinja = avgCostNinja;
  costumeShop->avgCostPirate = avgCostPirate;

  sem_init(&teams, 0, numTeams);
  sem_init(costumeShop->doorLock, 0, 1);

  for(int i = 0; i < numPirates; i++){
    pthread_t pirate;
    pthread_create(pirate, NULL, costumeDept, 1, NULL);
  }
  for(int i = 0; i < numNinjas; i++){
    pthread_t ninja;
    pthread_create(ninja, NULL, costumeDept, 0, NULL);
  }

  return 0;
}
