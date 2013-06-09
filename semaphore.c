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

int shm_id1;
voiture* shm_Pt;
voiture tab_cars[NBCARS];

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


void fct_pitstop(voiture *cars ){
    int pit_alea = (rand() % 10) +1;
    float pit_arret = (rand() % 8) + 1;
    if ( pit_alea == 2 && cars->pit < MAXPIT){
      cars->chrono.Tour += pit_arret + 2;
      cars->pit += 1;
      cars->vitesse = 0;
    }
}

void fct_sector(voiture **cars){ //Il faudrait remettre la distance secteur à zéro.

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
    float x;
    x=vitesse/3.6;
    return (x);
}

//Sans doute placer le fork ici.
voiture encourse(voiture *cars){
    int i;
      if(cars->out != 1) {
        chronos(&cars->chrono.Tour);
        printf("\nencourse\n");
        acceleration(&cars->vitesse);
        cars->distance += fonctiondistance(cars->vitesse);
        fct_sector(&cars);
        return *cars;
      }
}

void affichage(voiture *cars){
     int i;
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

void fct_open_shm(){
    int taille = sizeof(voiture) * NBCARS;
    int shm_id1 = shmget(1991, (taille), IPC_CREAT | 0666 ); // ouvre ou le creee le segment
    shm_Pt = (voiture*)shmat(shm_id1, NULL, 0); // on obtient l'address, retourn un pointer, retourne -1 si erreur et attache au process
    //printf("Neo c'est le plus beau ! \n");
    if (shm_Pt == (voiture*)(-1)) // check de shmPt
    perror("shmat");
}

int fct_sempetunia()
{
    int semid;
    union semPet{ //declaration de la structure 'union
      struct semid_ds *buf; //IPC_STAT IPC_GET
      ushort *array; //pour GETALL and SETALL
      int value;  //SETVAL uniquement
    };
    union semPet semPetu; //declaration de la variable de type union pour rappel, tout les champs d'une union partage le même espace mémoire qui est de la taille du plus grand champs
    semPetu.value=NBCARS;
    //Create one semaphore
    if ((semid=semget(1991,1,0666 | IPC_CREAT)<0)){  //préférable d'utiliser un ftoken ... peut être !
      perror("creating semaphore");
      exit(EXIT_FAILURE);}
      //init sem pour tous
      if(semctl(semid,0,SETVAL,semPetu)<0){
        perror("init");
        exit(EXIT_FAILURE);
      }
    return semid;
}
//opération p pour locker accès MP pour un semaphore
int p( int semid)
{
    struct sembuf p_buf;
    p_buf.sem_num = 0;
    //attends que ressource soit disponible (sem_op = 1) puis prends la ressource
    p_buf.sem_op = -1;
    //on attends jusqu'à ce que le sémaphore soit libre
    //p_buf.sem_flg = SEM_UNDO;
    int valRetour = semctl(semid, 0, GETVAL, 0);
    //printf("juste avant semop\n");
    //verifie la valeur du sémaphore avant d'effectuer l'opération
    if (valRetour > 0){
      if (semop(semid, &p_buf,1) == -1)  {
        perror("Operation P échoué");
        return 0;
      }
    }
    return 1;
}
//opération pour delocker l'accés a la MP
int v(int semid)
{
   struct sembuf v_buf;
   v_buf.sem_num = 0;
   //indique qu'1 ressource est dispo
   v_buf.sem_op = 1;
   //on attends jusqu'à ce que le sémaphore soit libre
   v_buf.sem_flg = SEM_UNDO;
   int valRetour = semctl(semid, 0, GETVAL, 0);
   //verifie la valeur du sémaphore avant d'effectuer l'opération
   if (valRetour < 24){
     if (semop(semid, &v_buf,1) == -1)  {
       perror("Operation V echoué");
       return 0;
     }
   }
   return 1;
}
//verouille les 24 sémaphores pour lecture processus parent
int pParent(int semid, int nbQualif)
{
    struct sembuf p_buf;
        p_buf.sem_num = 0;
        //soit -24, -17, -10
        p_buf.sem_op = - nbQualif;
        //on attends jusqu'à ce que le sémaphore soit libre
        p_buf.sem_flg = SEM_UNDO;
    //printf ("semid = %d  , SemNum = %d",semid,p_buf.sem_num);
    //verifie la valeur du sémaphore avant d'effectuer l'opération
    int valRetour = semctl(semid, 0, GETVAL, 0);
    if (valRetour == nbQualif){
        if (semop(semid,&p_buf,1) == -1)  {
            perror("Operation P échoué");
            return 0;
        }
    }
    return 1;
}
//deverouille les 24 sémaphores après lecture processus parent
int vParent(int semid,int nbQualif)
{
    struct sembuf v_buf;
    v_buf.sem_num = 0;
    //une ressource est disponible
    v_buf.sem_op = nbQualif;
    //on attends jusqu'à ce que le sémaphore soit libre
    v_buf.sem_flg = SEM_UNDO;
    int valRetour = semctl(semid, 0, GETVAL, 0);
    //verifie la valeur du sémaphore avant d'effectuer l'opération
    if (valRetour == 0){
      if (semop(semid,&v_buf,1) == -1)  {
        perror("Operation P échoué");
        return 0;
      }
    }
    return 1;
}

void processusEnfant(int numProcessus, voiture car)
{
    int semid,x;
    //CHRONO voiture;
    //float chrono;
    //CHRONO tabChTemp[NBCARS];
    //srand((unsigned)time(0));
    //chrono = tempsMin + (float)rand() / ((float) RAND_MAX / (tempsMax - tempsMin));
    //voiture.Num_Voiture = numProcessus;
    //voiture.temps = chrono;
    //printf("dans processus enfant num %d\n",numProcessus);
    //ouvrir les sémaphores qui sont crées dans le main
    semid = semget(1991, 1, 0666);
    if (semid < 0){
      //printf("semaphores introuvables");
      exit(0);
    }
    tab_cars[numProcessus] = encourse(&car);
    //printf("avant lock sem processus enfant num %d, semid numero %d\n",numProcessus, semid);
    printf("voiture numero : %d\n", tab_cars[numProcessus].numero);
    printf("La vitesse :  %lf\n", tab_cars[numProcessus].vitesse);
    printf("Distance parcouru %lf\n", tab_cars[numProcessus].distance);
    printf("Number Tour : %d\n", tab_cars[numProcessus].nbTour);
    printf("Number PIT : %d\n", tab_cars[numProcessus].pit);
    //locker semaphore
    p(semid);
    //tableau temporaire pour ecrire où on veut dans la MP
    *shm_Pt = tab_cars[numProcessus];
    //delocker semaphore
    v(semid);
}
void processusParent(int nbEnfants){
    int x,semid;

    voiture tabCh[nbEnfants];
    //ouvrir les sémaphores qui sont crées dans le main
    semid = semget(1991, 1, 0666);
    if (semid < 0){
      printf("semaphores introuvables processus parent");
      exit(0);
    }
    //on bloque le sémaphore
    pParent(semid,nbEnfants);
    //on va chercher les données en mémoire partagée
    for (x = 0; x<NBCARS;x++){
      tabCh[x] =  *shm_Pt;
    }
    //déverouiller le semaphore
    vParent(semid,nbEnfants);
    int i;
    for(i=0;i<NBCARS;i++){
    affichage(&tabCh[i]);
    }
}

void creerEnfants(int nbEnfants, voiture cars[NBCARS])
{   //tableau de pid enfants
    pid_t tabPidEnfants[nbEnfants];
    pid_t p;
    int enAttente,cpt,i;
    //allouer de la memoire pour le tab
    //printf("avant fork\n");
    //creer les enfants
    fct_sempetunia();
    fct_open_shm();
    for ( cpt = 0; cpt < nbEnfants; cpt++) {
      if ((p = fork()) == 0) {
        //printf("avant processus enfant %d\n",cpt);
        processusEnfant(cpt,cars[cpt]);
        exit(0);
        }
        else {
          tabPidEnfants[cpt] = p;
          //processusParent(cars);
        }
    }
    // il faut attendre que les enfants exitent pour eviter les zombies
     do {
        enAttente = 0;
        for (i = 0; i < nbEnfants; ++i) {
          if (tabPidEnfants[i] > 0) {
           if (waitpid(tabPidEnfants[i], NULL, 0) == tabPidEnfants[i]) {
              //l'enfant a fini
              tabPidEnfants[i] = 0;
            }
            else {
              // l'enfant a pas fini
              enAttente = 1;
            }
          }
            sleep(0);
        }
        //printf("je boucle mais je fais rien !!!\n");
        //if (waitpid(tabPidEnfants[i], NULL, 0) == tabPidEnfants[i])
    } while (enAttente);
    //nettoyage
    //free(tabPidEnfants);
}
 int main(int argc, char* argv[]) {
  srand(time(NULL)); //Random aléatoire pour chaque coup du rand();.
  voiture cars[NBCARS];
  initialisation_voiture(cars);
  while(NBRSECTEUR!=1){
  //affichage(&cars);
  creerEnfants(NBCARS,cars);
  }
}

