//Matthew St. Louis (mastlouis), Fay Whittall (fxwhittall)
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <sem.h>

void costumeDept(int isArr){
  int numNinjas;
  int numPirates;
  if(isArr){
    sleep(getRandNormNum(avgArrPirate));
  }
  else sleep(getRandNormNum(avgArrNinja));
  if(isArr){
    while(numNinjas){
      sem_wait(&hasPirates);
    }
    sem_post(&hasPirates);
    sem_wait(&teams);
    sleep(getRandNormNum(avgCostPirate));
    sem_post(&teams);

  }
  else{
    while(numPirates){
      sem_wait(&hasNinjas);
    }
    sem_post(&hasPirates);
    sem_wait(&teams);
    sleep(getRandNormNum(avgCostNinja));
    sem_post(&teams);
  }
}

double getRandNormNum(int avg){
  return avg*(sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48()) + 1);
}

int main(int argc, char* argv[]){
  sem_t hasPirates;
  sem_t hasNinjas;
  sem_t teams;

  srand48(time(0));
  int numTeams, numPirates, numNinjas, avgCostPirate, avgCostNinja, avgArrPirate, avgArrNinja;
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
  avgCostPirate = atoi(argv[4]);
  avgCostNinja = atoi(argv[5]);
  avgArrPirate = atoi(argv[6]);
  avgArrNinja = atoi(argv[7]);

  sem_init(&hasPirates, 0, 0);
  sem_init(&hasNinjas, 0, 0);
  sem_init(&teams, 0, numTeams);

  printf("%f\n", getRandNormNum(avgCostPirate));

  return 0;
}
