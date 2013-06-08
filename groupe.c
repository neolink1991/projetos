/*Projet de OS réalisé avec Thibault Dockx, Alexis Dufour, Sebastien Peetermans
  Version avant Fork et Semaphore.
*/


#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <unistd.h>

#define VITESSEMAX1 350 // Vitesse max pour l'accélération classique
#define VITESSEMAX2 400 // Vitesse max pour les variation d'accélération
#define VITESSE1 200 // Vitesse max pour une accélération de 50km/h
#define VITESSE2 300 // Vitesse max pour une accélération de 25km/h
#define ACCEL1 50 // Accélération entre 0 et 200 km/h
#define ACCEL2 25 // Accélération entre 200 et 300km/h
#define ACCEL3 10 // Accélération entre 300 et 350km/h
#define VARIATION 5 // Variation d'accélération entre 200 et 400km/h
#define VITESSEBASE 0 // Vitesse de base
#define DISTANCEBASE 0 // Distance de base
#define NBRSECTEUR 3
#define DISTTOUR 1984
#define DISTSECTEUR1 450
#define DISTSECTEUR2 620
#define DISTSECTEUR3 964
#define IDTOUR 4
#define TAILLEMSGQ 10
#define NBCARS 24
#define MAXPIT 3// Maximum de pit par voitures
#define FormPhys //Cela va nous permettre de prendre en compte l'accélération permatant de calculer si une voiture à fini son tour ou non
/*
*/



typedef struct {
    float s1,s2,s3; // Permet de sauvegarder le temps d'une voiture pour le sector 1 / 2 / 3
    float TotalTime, Tour; //le temps total + tour.
}chrono;

typedef struct{
    float vitesse;
    float distance;
    int numero;
    int nbTour;
    int out;
    int pit; //Arret au stand MAX 3 voir DEFINE
    chrono chrono;
}voiture;

void initialisation_voiture(voiture voit[NBCARS]){
    int i;
    for ( i = 0 ; i<NBCARS; i++ ){
        voit[i].numero=i+1;
        voit[i].chrono.Tour=0.0;
        voit[i].vitesse=0.0;
        voit[i].chrono.s1=0.0;
        voit[i].chrono.s2=0.0;
        voit[i].chrono.s3=0.0;
        voit[i].chrono.TotalTime=0.0;
        voit[i].distance=0.0;
        voit[i].nbTour=0;
        voit[i].out=0;
        voit[i].pit = 0;
    }
    voit[12].numero=25;
}

float chronos(float *chrono){
    *chrono += 1;
}

/**
 * Genere un temps d'arret aléatoire entre ? et ?
 * Genere un nombre aléatoire pour savoir si la voiture va au pit ou pas
 * /!\ gerer avant l'appel un nombre aléatoire pour savoir quelle voiture sera concerner par le pit
**/

void fct_pitstop(voiture *cars ){
    int pit_alea = (rand() % 10) +1;
    float pit_arret = (rand() % 8) + 1;
    if ( pit_alea == 2 && cars->pit < MAXPIT){
      cars->chrono.Tour = pit_arret + 2;
      cars->pit += 1;
      cars->vitesse = 0;
    }
}

void fct_sector(voiture *cars){ //Il faudrait remettre la distance secteur à zéro.
  int brokkenEngine = (rand() % 3000) + 1;
  if ( brokkenEngine == 190 ){
    cars->out = 1;
  }
  else if ( cars->out != 1 && cars->distance >= DISTSECTEUR1 && cars->distance < DISTSECTEUR2 )
    cars->chrono.s1=cars->chrono.Tour;
    //break;
  else if ( cars->out != 1 && cars->distance >= DISTSECTEUR2 && cars->distance < DISTSECTEUR3)
    cars->chrono.s2=cars->chrono.Tour;
    //break;
  else if ( cars->out != 1 && cars->distance >= DISTSECTEUR3 ){
    fct_pitstop(cars);
    cars->chrono.s3 = cars->chrono.Tour;
    cars->chrono.TotalTime += cars->chrono.Tour;
    cars->chrono.Tour = 0;
    cars->nbTour += 1;
    cars->distance = 0;
    //break;
  }
}

void acceleration(float *vitesse) {
  int token = (rand() % 10) + 1;
  if ( token >= 4 ){
    if(*vitesse<=VITESSE1) {
       *vitesse+=ACCEL1;
    }
    else if(*vitesse<=VITESSE2) {
       *vitesse+=ACCEL2;
    }
    else if(*vitesse<=VITESSEMAX1) {
      *vitesse+=ACCEL3;
    }
  }
  else{
      if(*vitesse>=VITESSE2) {
        *vitesse-=5;
      }
      else if(*vitesse>=VITESSEMAX1) {
        *vitesse-=15;
    }
  }
}
float fonctiondistance(float vitesse){
    //Vitesse initiale( Distance + la vitesse * l'accélaration * le temps au carrée )
    //x= xi+v.T+1/2at²
    float x;
    x=vitesse/3.6;
    return (x);
}

//Sans doute placer le fork ici.
void encourse(voiture cars[]){
    int i;
    for (i = 0; i <= NBCARS ; i++){
      if(cars[i].out != 1) {
        chronos(&cars[i].chrono.Tour);
        acceleration(&cars[i].vitesse);
        cars[i].distance += fonctiondistance(cars[i].vitesse);
        fct_sector(&cars[i]);
      }
    }
}

void affichage(voiture cars[NBCARS]){
     int i;
     encourse(cars);
     system("clear");
     printf("Voiture |\t  Chrono total |\t chrono tour |\t  Vitesse |\t Nbtour |\t Distance\n");
     for ( i = 0; i < NBCARS ; i++){
     printf("%d\t\t",cars[i].numero);
     if(cars[i].out == 1) printf("Vout!");
     printf("%4.3lf\t\t",cars[i].chrono.TotalTime);
     printf("%8.3lf\t",cars[i].chrono.Tour);
     printf("%3.0lf km/h\t",cars[i].vitesse);
     printf("%d\t",cars[i].nbTour);
     printf("%5.2lf m\t",cars[i].distance );
     printf("%5.2lf m\t",cars[i].chrono.s1 );
     printf("%5.2lf m\t",cars[i].chrono.s2 );
     printf("%5.2lf m\t",cars[i].chrono.s3 );
     printf("Number Pit : %d\n",cars[i].pit);
     usleep(5000);
     }
}


void depart(int nbvoitures,int id, int vitesse, double distance)
{
  int cpt;
  for (cpt = 0; cpt<=nbvoitures; cpt ++){

  }
}

 int main(int argc, char* argv[]) {
  srand(time(NULL)); //Random aléatoire pour chaque coup du rand();.
  voiture cars[NBCARS];
  initialisation_voiture(cars);
  while(NBRSECTEUR!=1){
  affichage(cars);
  }
}

