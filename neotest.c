/*Projet de OS réalisé avec Thibault Dockx, Alexis Dufour, Sebastien Peetermans
  Réalisation du Sema ... mise en place des tris ( Voir version prochaine ) .
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

#define VITESSEMAX1 350 // Vitesse max pour l'accélération classique
#define VITESSEMAX2 400 // Vitesse max pour les variation d'accélération
#define VITESSE1 200 // Vitesse max pour une accélération de 50km/h
#define VITESSE2 300 // Vitesse max pour une accélération de 25km/h
#define ACCEL1 50 // Accélération entre 0 et 200 km/h
#define ACCEL2 25 // Accélération entre 200 et 300km/h
#define ACCEL3 10 // Accélération entre 300 et 350km/h
#define VARIATION 5 // Variation d'accélération entre 200 et 400km/h
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
void fct_sector(int i){ //Il faudrait remettre la distance secteur à zéro.
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
  }
}

/**
 * ACCELERATION
**/
void acceleration(int i) {
  int token;
  //printf("token:%d\n",token);
  token = (rand() % 10) + 1;
  if (token >= 4) {
    if(shm_Pt[i].vitesse <= VITESSE1) {
        shm_Pt[i].vitesse += ACCEL1;

    } else if (shm_Pt[i].vitesse <= VITESSE2) {
      shm_Pt[i].vitesse += ACCEL2;

    } else if(shm_Pt[i].vitesse <= VITESSEMAX1) {
      shm_Pt[i].vitesse += ACCEL3;

    } else {
      if(shm_Pt[i].vitesse >= VITESSE2) {
        shm_Pt[i].vitesse -= 40;
      } else if(shm_Pt[i].vitesse >= VITESSEMAX1) {
        shm_Pt[i].vitesse -= 70;
      }
    }
  }
}

/**
 * DISTANCE
**/
float fonctiondistance(int i) {
  //Vitesse initiale( Distance + la vitesse * l'accélaration * le temps au carrée )
  //float x;
  //x=shm_Pt[i].vitesse/3.6;
  //return (x);
  return (shm_Pt[i].vitesse/3.6);
}

//Sans doute placer le fork ici.
/**
 * EN COURSE
**/
void encourse(int i){
  srand(time(NULL) ^ (getpid() << 16));

  if(shm_Pt[i].out != 1) {
    chronos(i);
    // printf("\nencourse\n");
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

//http://en.wikipedia.org/wiki/ANSI_escape_code
/**
 * AFFICHAGE
**/
void affichage(){
  int i, copie, besttours1, besttours2, besttours3;
  system("clear");
  printf("Num |\tChrono | Tour | Vitesse | Nbtour | Distance  |  s1   |  s2  |  s3   | PIT |\n");
  for(i=0; i<NBCARS; i++) {

   printf("%2d\t",shm_Pt[i].numero);
   //CHANGER PAR COULEUR
   if(shm_Pt[i].out == 1) printf("Vout!");
   printf("%4.3lf ",shm_Pt[i].chrono.TotalTime);
   printf("%8.3lf ",shm_Pt[i].chrono.Tour);
   printf("%3.0lf km/h      ",shm_Pt[i].vitesse);
   printf("%2d       ",shm_Pt[i].nbTour);
   printf("%5.2lf m ",shm_Pt[i].distance );
   printf("%5.2lf m ",shm_Pt[i].chrono.s1 );
   printf("%5.2lf m ",shm_Pt[i].chrono.s2 );
   printf("%5.2lf m    ",shm_Pt[i].chrono.s3 );
   printf("%d\n",shm_Pt[i].pit);
   }
   besttours1 = find_best_sector1();
   besttours2 = find_best_sector2();
   besttours3 = find_best_sector3();
   printf("\nBest voiture Sector 1 : %d\n", besttours1);
   printf("\nBest voiture Sector 2 : %d\n", besttours2);
   printf("\nBest voiture Sector 3 : %d\n\n", besttours3);
   //usleep(100);
}

/**
 * OPEN SHARE MEMORY
**/
void fct_open_shm(){
  int token = ftok("/tmp", 'n');
  int taille = sizeof(voiture) * NBCARS;
  int shm_id1 = shmget(token, (taille), IPC_CREAT | 0666 ); // ouvre ou le creee le segment
  shm_Pt = (voiture*)shmat(shm_id1, NULL, 0); // on obtient l'address, retourn un pointer, retourne -1 si erreur et attache au proces
  if (shm_Pt == (voiture*)(-1)) { // check dnombre tiré au hazard forke shmPt
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
  if (semid < 0) {  //préférable d'utiliser un ftoken ... peut être !
    perror("creating semaphore");
    exit(EXIT_FAILURE);
  }
  //init sem pour tousnombre tiré au hazard fork
  printf("======%d\n",semid);
  if(semctl(semid,0,SETVAL,semPetu) < 0) {
    perror("init");
    exit(EXIT_FAILURE);
  }
  return semid;
}

//opération p pour locker accès MP pour un semaphore
/**
 * FONCTION P
**/
int p( int semid) {
  struct sembuf p_buf;
  p_buf.sem_num = 0;
  //attends que ressource soit disponible (sem_op = 1) puis prends la ressource
  p_buf.sem_op = -1;
  //on attends jusqu'à ce que le sémaphore soit libre
  //p_buf.sem_flg = SEM_UNDO;nombre tiré au hazard fork
  int valRetour = semctl(semid, 0, GETVAL, 0);
  //printf("juste avant semop\n");
  //verifie la valeur du sémaphore avant d'effectuer l'opération
  if (valRetour > 0) {
    if (semop(semid, &p_buf,1) == -1) {
      perror("Operation P échoué");
      return 0;
    }
  }
  return 1;
}

//opération pour delocker l'accés a la MP
/**
 * FONCTION V
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
void processusEnfant(int numProcessus) {
  //voiture tuture;
  int semid;
  //CHRONO voiture;
  //float chrono;
  //CHRONO tabChTemp[NBCARS];
  //srand((unsigned)time(0));
  //chrono = tempsMin + (float)rand() / ((float) RAND_MAX / (tempsMax - tempsMin));
  //voiture.Num_Voiture = numProcessus;
  //voiture.temps = chrono;
  //printf("dans processus enfant num %d\n",numProcessus);
  //ouvrir les sémaphores qui sont crées dans le main
  semid = semget(ftok("/tmp", 'q'), 1, 0666);
  if (semid < 0) {
    printf("semaphores introuvables");
    exit(0);
  }
  // sleep(1);
  encourse(numProcessus);
  //printf("avant lock sem processus enfant num %d, seWmid numero %d\n",numProcessus, semid);
  /**
  SERT D'EXEMPLE ECRITURE MEMOIRE PARTAGER
  tuture.numero = cars[numProcessus].numero;
  tuture.vitesse = cars[numProcessus].vitesse;
  tuture.distance = cars[numProcessus].distance;
  tuture.nbTour = cars[numProcessus].nbTour;
  tuture.pit = cars[numProcessus].pit;
  tuture.out = cars[numProcessus].out;
  tuture.chrono.Tour = cars[numProcessus].chrono.Tour;
  tuture.chrono.TotalTime = cars[numProcessus].chrono.TotalTime;
  tuture.chrono.s1 = cars[numProcessus].chrono.s1;
  tuture.chrono.s2 = cars[numProcessus].chrono.s2;
  tuture.chrono.s3 = cars[numProcessus].chrono.s3;
  **/
  p(semid);
  //tableau temporaire pour ecrire où on veut dans la MP
  // shm_Pt[numProcessus] = tuture;
  //delocker semaphoreW
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
 * PROCESSUS PARENT
**/
void processusParent() {
  int x, semid;
  //voiture tabCh[nbEnfants];
  //ouvrir les sémaphores qui sont crées dans le main
  semid = semget(ftok("/tmp", 'q'), 1, 0666);
  // if (semid < 0){
  //  printf("semaphores introuvables processus parent");
  //   exit(0);
  // }
  //on bloque le sémaphore
  pParent(semid, NBCARS);
  //on va chercher les données en mémoire partagée
  // for (x = 0; x<NBCARS;x++){
  //  tabCh[x] =  *shm_Pt;
  // }
  //affichage();
  //déverouiller le semaphore
  vParent(semid, NBCARS);
  //int i;
  //for(i=0;i<NBCARS;i++){
  // affichage(&tabCh[i]);
  //}
}

/**
 * CREER ENFANTS
 * changer nom anti foutre Seb Et shubba dasn la merde
**/
void creerEnfants(int nbEnfants) {   //tableau de pid enfants
  pid_t *tabPidEnfants;
  pid_t p;
  int a, enAttente, cpt, i;
  int iii=0;
  //allouer de la memoire pour le tab
  //printf("avant fork\n");
  tabPidEnfants = malloc(nbEnfants * sizeof(pid_t));
  //creer les enfants
  a = fct_sempetunia();
  printf("a: %d\n", a);
  //int x=0;
  for ( cpt = 0; cpt < nbEnfants; cpt++) {
    if ((p = fork()) == 0) {
    //while(x<nbEnfants/24){
      processusEnfant(cpt);
    //printf("cptloolol: %d¹¹%d,\n",cpt,nbEnfants);
    //sleep(1);
    //x++;
    //}
      exit(0);
    } else {
      tabPidEnfants[cpt] = p;
      usleep(1000); /** ---------------------REGULE LA VITESSE DU PROGRAMME-------------------------- **/
      processusParent(NBCARS);
      //printf("=======%d\n",iii);
      iii++;
    }
  }
  //il faut attendre que les enfants exitent pour eviter les zombies
  do {
    enAttente = 0;
    for (i=0; i<nbEnfants; i++) {
      if (tabPidEnfants[i] > 0) {
        if (waitpid(tabPidEnfants[i], NULL, 0) == tabPidEnfants[i]) {//=>>go man
          //l'enfant a fini
          tabPidEnfants[i] = 0;
        } else {
          // l'enfant a pas fini
          enAttente = 1;
        }
      }
      sleep(0);
    }
    //printf("je boucle mais je fais rien !!!\n");
  } while (enAttente);
  //nettoyage
  free(tabPidEnfants);
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


/**
 * MAIN
**/
int main() {
  //Random aléatoire pour chaque coup du rand();.
  int valeurMax = 100;
  int i, copie;

  fct_open_shm();
  initialisation_voiture();
  initMp();

  for(i=0; i<valeurMax; i++) {
    //affichage(&cars);
    creerEnfants(NBCARS);
    /**COPIE TABLEAU 1 DANS TABLEAU 2 **/
    /**
    for(copie=0; copie<NBCARS; copie++) {
      tricars[i] = shm_Pt[i];
    }
    **/
    /**FONCTION DE TRI --ICI--**/
    usleep(20);
    triCourse();
    affichage();
    printf("===========%d\n",i);
    //printf("=======num boucle main %d\n",i);
  }

  return(0);
}
