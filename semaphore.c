/*Projet de OS réalisé avec Thibault Dockx, Alexis Dufour, Sebastien Peetermans
  Réalisation du Sema ... mise en place des tris ( Voir version prochaine ) .
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
#define NBRSECTEUR 3 //Nombre de secteur du circuit
#define DISTTOUR 1984 // taille du circuit
#define DISTSECTEUR1 450 //  longueur du secteur 1
#define DISTSECTEUR2 620    //longueyr du secteur 2
#define DISTSECTEUR3 964    //longeur du secteur 36
#define NBCARS 24   //Nombre de voitures participantes
#define MAXPIT 3// Maximum de pit par voitures
#define FormPhys //Cela va nous permettre de prendre en compte l'accélération permatant de calculer si une voiture à fini son tour ou non


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

typedef struct {
    int pid[NBCARS];
}pid;

void initialisation_voiture(voiture *voit){
        voit->numero = 1;
        voit->chrono.Tour=0.0;
        voit->vitesse=0.0;
        voit->chrono.s1=0.0;
        voit->chrono.s2=0.0;
        voit->chrono.s3=0.0;
        voit->chrono.TotalTime=0.0;
        voit->distance=0.0;
        voit->nbTour=0;
        voit->out=0;
        voit->pit = 0;
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
      cars->chrono.Tour += pit_arret + 2;
      cars->pit += 1;
      cars->vitesse = 0;
    }
}

void fct_sector(voiture *cars){ //Il faudrait remettre la distance secteur à zéro.

  int brokkenEngine = (rand() % 3000) + 1;
  if ( brokkenEngine == 190 ){
    cars->out = 1;
  }
  if ( cars->out != 1 && cars->distance >= DISTSECTEUR1 && cars->distance < DISTSECTEUR2 )
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
        *vitesse-=40;
      }
      else if(*vitesse>=VITESSEMAX1) {
        *vitesse-=70;
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
void encourse(voiture *cars){
    int i;
      if(cars->out != 1) {
        chronos(&cars->chrono.Tour);
        acceleration(&cars->vitesse);
        cars->distance += fonctiondistance(cars->vitesse);
        fct_sector(cars);
      }
}

void affichage(voiture *cars){
     int i;
     encourse(cars);
     system("clear");
     printf("Voiture |\t  Chrono total |\t chrono tour |\t  Vitesse |\t Nbtour |\t Distance\n");
     printf("%d\t\t",cars->numero);
     if(cars->out == 1) printf("Vout!");
     printf("%4.3lf\t\t",cars->chrono.TotalTime);
     printf("%8.3lf\t",cars->chrono.Tour);
     printf("%3.0lf km/h\t",cars->vitesse);
     printf("%d\t",cars->nbTour);
     printf("%5.2lf m\t",cars->distance );
     printf("%5.2lf m\t",cars->chrono.s1 );
     printf("%5.2lf m\t",cars->chrono.s2 );
     printf("%5.2lf m\t",cars->chrono.s3 );
     printf("Number Pit : %d\n",cars->pit);
     usleep(5000);
}

 int main(int argc, char* argv[]) {
  srand(time(NULL)); //Random aléatoire pour chaque coup du rand();.
  voiture cars;
  initialisation_voiture(&cars);
  while(NBRSECTEUR!=1){
  affichage(&cars);
  }
}

