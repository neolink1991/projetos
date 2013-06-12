/*Projet de OS réalisé avec Thibault Dockx, Alexis Dufour, Sebastien Peetermans
*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

#define DP12 4400 //Temps en seconde pour Durée Période 1 et 2
#define DP3 3600 //Temps en seconde pour Durée période 3
#define Q1 1200 //  |
#define Q2 900  //  | -> Temps en seconde pour les qualifications
#define Q3 600  //  |
#define NBRSECTEUR 3 //Nombre de secteur du circuit
#define DISTSECTEUR1 450 //  longueur du secteur 1
#define DISTSECTEUR2 620    //longueyr du secteur 2
#define DISTSECTEUR3 1600    //longeur du secteur 36
#define NBCARS 24   //Nombre de voitures participantes
#define MAXPIT 3// Maximum de pit par voitures
#define RED "\x1b[31m"
#define YELLOW "\x1b[3m"
#define NBTOURS 42


typedef struct {
  float s1,s2,s3,best_s1,best_s2,best_s3; // Permet de sauvegarder le temps d'une voiture pour le sector 1 / 2 / 3
  float TotalTime, Tour; //le temps total + tour.
} chrono;

typedef struct{
  float vitesse;
  float distance;
  int numero;
  int nbTour;
  int out;
  int pit; //Arret au stand MAX 3 voir DEFINE
  chrono chrono;
} voiture;

int shm_id1;
voiture * shm_Pt;
voiture tricars[NBCARS];
voiture cars[NBCARS];

/**
 * AFFICHER MENU
**/
affiherMenu() {
  system("clear");
  printf(" GRAND PRIX DE FORMULE 1\n");
  printf(" -----------------------\n");
  printf(" 1. ESSAIS\n");
  printf(" 2. QUALIFS\n");
  printf(" 3. COURSE\n\n");
  printf(" Votre choix : ");
}

/**
 * INITIALISATION
**/
void initialisation_voiture() {
  int i;
  for (i=0; i<NBCARS; i++) {
    cars[i].numero = i+1;
    cars[i].chrono.Tour = 0.0;
    cars[i].vitesse = 0.0;
    cars[i].chrono.s1 = 0.0;
    cars[i].chrono.s2 = 0.0;
    cars[i].chrono.s3 = 0.0;
    cars[i].chrono.best_s1 = 0.0;
    cars[i].chrono.best_s2 = 0.0;
    cars[i].chrono.best_s3 = 0.0;
    cars[i].chrono.TotalTime = 0.0;
    cars[i].distance = 0.0;
    cars[i].nbTour = 0;
    cars[i].out = 0;
    cars[i].pit = 0;
  }
  cars[12].numero=25;
}

/**
 * CHRONO
**/
void chronos(int i) {
  double inter1=((rand() % 10) + 1);
  double inter2=((rand() % 10) + 1);
  shm_Pt[i].chrono.Tour += inter1/inter2;
}
/**
 * PIT STOP
**/
void fct_pitstop(int i){
  int pit_alea;
  float pit_arret;

  pit_alea = (rand() % 10) +1;
  pit_arret = (rand() % 8) +1;
  if ((pit_alea == 2) && (shm_Pt[i].pit < MAXPIT)) {
    shm_Pt[i].chrono.Tour += pit_arret + 2;
    shm_Pt[i].pit += 1;
    shm_Pt[i].vitesse = 0;
  }
}

/**
 * SECTEURS
**/
void fct_sector(int i){
  int brokkenEngine;

  brokkenEngine = (rand() % 3000) + 1;
  if ( brokkenEngine == 190 ) {
    shm_Pt[i].out = 1;
  }

  if ((shm_Pt[i].out != 1) && (shm_Pt[i].distance >= DISTSECTEUR1) && (shm_Pt[i].distance < DISTSECTEUR2)) {
    shm_Pt[i].chrono.s1=shm_Pt[i].chrono.Tour;
    if (shm_Pt[i].chrono.best_s1 < shm_Pt[i].chrono.s1){  //Enregistrement du meilleur temps secteur 1 pour voiture i
      shm_Pt[i].chrono.best_s1 = shm_Pt[i].chrono.s1;
    }
  }

  else if ((shm_Pt[i].out != 1) && (shm_Pt[i].distance >= DISTSECTEUR2) && (shm_Pt[i].distance < DISTSECTEUR3)) {
    shm_Pt[i].chrono.s2=shm_Pt[i].chrono.Tour;
    if (shm_Pt[i].chrono.best_s2 < shm_Pt[i].chrono.s2){ //Enregistrement du meilleur temps secteur 2 pour voiture i
      shm_Pt[i].chrono.best_s2 = shm_Pt[i].chrono.s2;
    }
  }
  else if ((shm_Pt[i].out != 1) && (shm_Pt[i].distance >= DISTSECTEUR3)) {
    if (shm_Pt[i].chrono.best_s3 < shm_Pt[i].chrono.s3){ //Enregistrement du meilleur temps secteur 3 pour voiture i
      shm_Pt[i].chrono.best_s3 = shm_Pt[i].chrono.s3;
    }
    fct_pitstop(i);
    shm_Pt[i].chrono.s3 = shm_Pt[i].chrono.Tour;
    shm_Pt[i].chrono.TotalTime += shm_Pt[i].chrono.Tour;
    shm_Pt[i].chrono.Tour = 0;
    shm_Pt[i].nbTour += 1;
    shm_Pt[i].distance = 0;
    shm_Pt[i].chrono.s1 = 0;
    shm_Pt[i].chrono.s2 = 0;
    //shm_Pt[i].chrono.s3 = 0;
  }
}

/**
 * ACCELERATION
**/
void acceleration(int i) {
  int token;
  token = (rand() % 10) + 1;
  if (token >= 4) {
    if(shm_Pt[i].vitesse <= 150) {
        shm_Pt[i].vitesse += (rand()%100)+1;
    } else if(shm_Pt[i].vitesse <= 250) {
      shm_Pt[i].vitesse += (rand()%50)+1;
    } else {
      if(shm_Pt[i].vitesse >= 170) {
        shm_Pt[i].vitesse -= (rand()%30)+1;
      } else if(shm_Pt[i].vitesse >= 325) {
        shm_Pt[i].vitesse -= (rand()%70)+1;
      }
    }
  }
}

/**
 * DISTANCE
**/
float fonctiondistance(int i) {
  return (shm_Pt[i].vitesse/3.6);
}

/**
 * EN COURSE
**/
void encourse(int i){
  srand(time(NULL) ^ (getpid() << 16));

  if(shm_Pt[i].out != 1) {
    chronos(i);
    acceleration(i);
    shm_Pt[i].distance += fonctiondistance(i);
    fct_sector(i);
  }
}

int find_best_sector1 (){
  int numvoiture,i;
  float refe = 50000;
  for (i=0; i<NBCARS; i++) {
    if ( shm_Pt[i].chrono.best_s1 < refe){
      refe = shm_Pt[i].chrono.best_s1;
      numvoiture = shm_Pt[i].numero;
    }
  }
  return numvoiture;
}

int find_best_sector2 (){
  int numvoiture,i;
  float refe = 50000;
  for (i=0; i<NBCARS; i++) {
    if ( shm_Pt[i].chrono.best_s2 < refe){
      refe = shm_Pt[i].chrono.best_s2;
      numvoiture = shm_Pt[i].numero;
    }
  }
  return numvoiture;
}

int find_best_sector3 (){
  int numvoiture,i;
  float refe = 50000;
  for (i=0; i<NBCARS; i++) {
    if ( shm_Pt[i].chrono.best_s3 < refe){
      refe = shm_Pt[i].chrono.best_s3;
      numvoiture = shm_Pt[i].numero;
    }
  }
  return numvoiture;
}

/**
 * AFFICHAGE
**/
void affichage(){
  int i, copie, besttours1, besttours2, besttours3;
  system("clear");
  printf("Num |\tChrono | Tour | Vitesse | Nbtour | Distance  |  s1   |  s2  |  s3   | PIT |\n");
  for(i=0; i<NBCARS; i++) {
    if(shm_Pt[i].out == 1) printf(RED);
    printf("%2d\t",shm_Pt[i].numero);
    printf("%4.3lf ",shm_Pt[i].chrono.TotalTime);
    printf("%8.3lf ",shm_Pt[i].chrono.Tour);
    printf("%3.0lf km/h      ",shm_Pt[i].vitesse);
    printf("%2d       ",shm_Pt[i].nbTour);
    printf("%5.2lf m ",shm_Pt[i].distance );
    printf("%5.2lf s ",shm_Pt[i].chrono.s1 );
    printf("%5.2lf s ",shm_Pt[i].chrono.s2 );
    printf("%5.2lf s    ",shm_Pt[i].chrono.s3 );
    printf("%d\n",shm_Pt[i].pit);
    printf("\x1b[37m");
  }
  besttours1 = find_best_sector1();
  besttours2 = find_best_sector2();
  besttours3 = find_best_sector3();
  printf("\nBest voiture Sector 1 : %d\n", besttours1);
  printf("\nBest voiture Sector 2 : %d\n", besttours2);
  printf("\nBest voiture Sector 3 : %d\n\n", besttours3);
}

/**
 * OPEN SHARE MEMORY
**/
void fct_open_shm(){
  int token = ftok("/tmp", 'n');
  int taille = sizeof(voiture) * NBCARS;
  int shm_id1 = shmget(token, (taille), IPC_CREAT | 0666 );
  shm_Pt = (voiture*)shmat(shm_id1, NULL, 0); // on obtient l'addresse, retourne un pointer, retourne -1 si erreur et attache au process
  if (shm_Pt == (voiture*)(-1)) { // check nombre tiré au hazard fork shmPt
    perror("shmat");
  }
}

/**
 * SEMPETUNIA
**/
int fct_sempetunia() {
  int token = ftok("/tmp", 'q');
  int semid;

  union semPet{ //declaration de la structure 'union
    struct semid_ds *buf; //IPC_STAT IPC_GET
    ushort *array; //pour GETALL and SETALL
    int value;  //SETVAL uniquement
  };

  union semPet semPetu; //declaration de la variable de type union pour rappel, tout les champs d'une union partage le même espace mémoire qui est de la taille du plus grand champs
  semPetu.value=NBCARS;
  //Create one semaphore
  semid = semget(token,1,0666 | IPC_CREAT);
  if (semid < 0) {
    perror("creating semaphore");
    exit(EXIT_FAILURE);
  }
  if(semctl(semid,0,SETVAL,semPetu) < 0) {
    perror("init");
    exit(EXIT_FAILURE);
  }
  return semid;
}

/**
 * FONCTION P(mutex)
**/
int p( int semid) {
  struct sembuf p_buf;
  p_buf.sem_num = 0;
  p_buf.sem_op = -1;
  int valRetour = semctl(semid, 0, GETVAL, 0);
  if (valRetour > 0) {
    if (semop(semid, &p_buf,1) == -1) {
      perror("Operation P échoué");
      return 0;
    }
  }
  return 1;
}

/**
 * FONCTION V(mutex)
**/
int v(int semid) {
  int valRetour;
  struct sembuf v_buf;
  v_buf.sem_num = 0;
  //indique qu'1 ressource est dispo
  v_buf.sem_op = 1;
  //on attends jusqu'à ce que le sémaphore soit libre
  v_buf.sem_flg = SEM_UNDO;
  valRetour = semctl(semid, 0, GETVAL, 0);
  //verifie la valeur du sémaphore avant d'effectuer l'opération
  if (valRetour < 24) {
    if (semop(semid, &v_buf, 1) == -1)  {
      perror("Operation V echoué");
      return 0;
    }
  }
  return 1;
}

//verouille les 24 sémaphores pour lecture processus parent
/**
 * FONCTION PPARENT
**/
int pParent(int semid, int nbQualif) {
  int valRetour;
  struct sembuf p_buf;
  p_buf.sem_num = 0;
  //soit -24, -17, -10
  p_buf.sem_op = - nbQualif;
  //on attends jusqu'à ce que le sémaphore soit libre
  p_buf.sem_flg = SEM_UNDO;
  //printf ("semid = %d  , SemNum = %d",semid,p_buf.sem_num);
  //verifie la valeur du sémaphore avant d'effectuer l'opération
  valRetour = semctl(semid, 0, GETVAL, 0);
  if (valRetour == nbQualif) {
    if (semop(semid, &p_buf, 1) == -1)  {
      perror("Operation P échoué");
      return 0;
    }
  }
  return 1;
}

//deverouille les 24 sémaphores après lecture processus parent
/**
 * FONCTION VPARENT
**/
int vParent(int semid, int nbQualif) {
  int valRetour;
  struct sembuf v_buf;
  v_buf.sem_num = 0;
  //une ressource est disponible
  v_buf.sem_op = nbQualif;
  //on attends jusqu'à ce que le sémaphore soit libre
  v_buf.sem_flg = SEM_UNDO;
  valRetour = semctl(semid, 0, GETVAL, 0);
  //verifie la valeur du sémaphore avant d'effectuer l'opération
  if (valRetour == 0){
    if (semop(semid, &v_buf, 1) == -1) {
      perror("Operation P échoué");
      return 0;
    }
  }
  return 1;
}

/**
 * PROCESSUS ENFANT
**/
void f_un(int numProcessus) {
  int semid;
  semid = semget(ftok("/tmp", 'q'), 1, 0666);
  if (semid < 0) {
    printf("semaphores introuvables");
    exit(0);
  }
  encourse(numProcessus);
  p(semid);
  v(semid);
}

/**
 * INITMP
**/
void initMp() {
  int i;
  for(i=0; i<NBCARS; i++) {
    shm_Pt[i].numero = cars[i].numero;
    shm_Pt[i].vitesse = cars[i].vitesse;
    shm_Pt[i].distance = cars[i].distance;
    shm_Pt[i].nbTour = cars[i].nbTour;
    shm_Pt[i].pit = cars[i].pit;
    shm_Pt[i].out = cars[i].out;
    shm_Pt[i].chrono.Tour = cars[i].chrono.Tour;
    shm_Pt[i].chrono.TotalTime = cars[i].chrono.TotalTime;
    shm_Pt[i].chrono.s1 = cars[i].chrono.s1;
    shm_Pt[i].chrono.s2 = cars[i].chrono.s2;
    shm_Pt[i].chrono.s3 = cars[i].chrono.s3;
    shm_Pt[i].chrono.best_s1 = cars[i].chrono.best_s1;
    shm_Pt[i].chrono.best_s1 = cars[i].chrono.best_s2;
    shm_Pt[i].chrono.best_s1 = cars[i].chrono.best_s3;
  }
}

/**
 * CREER ENFANTS
 * changer nom anti foutre Seb Et shubba dasn la merde
**/
void go_f1(int nfn1) {   //tableau de pid enfants
  pid_t *tab_f1;
  pid_t p;
  int a, waitte, cpt, i;
  int iii=0;
  tab_f1 = malloc(nfn1 * sizeof(pid_t));
  a = fct_sempetunia();
  for ( cpt = 0; cpt < nfn1; cpt++) {
    if ((p = fork()) == 0) {
      f_un(cpt);
      exit(0);
    } else {
      tab_f1[cpt] = p;
      usleep(1000); /** ---------------------REGULE LA VITESSE DU PROGRAMME-------------------------- **/
      iii++;
    }
  }
  do {
    waitte = 0;
    for (i=0; i<nfn1; i++) {
      if (tab_f1[i] > 0) {
        if (waitpid(tab_f1[i], NULL, 0) == tab_f1[i]) {
          tab_f1[i] = 0;
        } else {
          waitte = 1;
        }
      }
      sleep(0);
    }
  } while (waitte);
  free(tab_f1);
}

/**
 * FONCTION DE TRI
 * shm_Pt, tricars
**/

void triCourse() {
  voiture tmp;
  int i, j;

  for (i=0; i<NBCARS-1; i++) {
    for (j=i+1; j<NBCARS; j++) {
      if (shm_Pt[i].nbTour < shm_Pt[j].nbTour) {
        tmp = shm_Pt[i];
        shm_Pt[i] = shm_Pt[j];
        shm_Pt[j] = tmp;
      } else if (shm_Pt[i].nbTour == shm_Pt[j].nbTour) {
        if (shm_Pt[i].distance < shm_Pt[j].distance) {
          tmp = shm_Pt[i];
          shm_Pt[i] = shm_Pt[j];
          shm_Pt[j] = tmp;
        }
      }
    }
  }
}

afficheVainqueurs() {
  int i;

  system("\n\n\n");
  printf(" PODIUM\n");
  printf(" ------\n");
  for (i=0; i<3; i++) {
    printf(" %d : %2d avec %lf\n", i+1, shm_Pt[i].numero, shm_Pt[i].chrono.TotalTime);
  }
}

void periode_essai(){
  int i = 0;
  printf("Debut séance d'essai 1\n");
  sleep(5);
  initialisation_voiture();
  initMp();
  system("clear");
  while( i < DP12 ){
    go_f1(NBCARS);
    usleep(20);
    triCourse();
    affichage();
    i++;
  }
  i=0;
  printf("Debut séance d'essai 2\n");
  sleep(5);
  initialisation_voiture();
  initMp();
  system("clear");
  while( i < DP12 ){
    go_f1(NBCARS);
    usleep(20);
    triCourse();
    affichage();
    i++;
  }
  i=0;
  printf("Debut séance d'essai 3\n");
  sleep(5);
  initialisation_voiture();
  initMp();
  system("clear");
  while( i < DP3 ){
    go_f1(NBCARS);
    usleep(20);
    triCourse();
    affichage();
    i++;
  }
}

void competition(){
  int last = NBCARS -1;
  while (shm_Pt[last].nbTour < NBTOURS) {
    go_f1(NBCARS);
    usleep(20);
    triCourse();
    affichage();
    if(shm_Pt[last].out) {
      last -= 1;
    }
  }
 afficheVainqueurs();
 sleep(10);
}

/**
 * MAIN
**/
int main() {
  //Random aléatoire pour chaque coup du rand();.
  int i, copie, last, choix;

  fct_open_shm();
  initialisation_voiture();
  initMp();
  while(1) {
    affiherMenu();
    fflush(stdin);
  scanf("%d", &choix);

  switch(choix) {
    case 1 :  periode_essai(); break;
    case 2 :  break;
    case 3 :  { /** COURSE**/
                competition();
                break;
              } /** FIN COURSE**/
    }
  }
  return(0);
}
