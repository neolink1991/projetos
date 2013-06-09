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
#define DISTANCEBASE 0 // Distance de base
#define NBRSECTEUR 3 //Nombre de secteur du circuit
#define DISTTOUR 1984 // taille du circuit
#define DISTSECTEUR1 450 //  longueur du secteur 1
#define DISTSECTEUR2 620    //longueyr du secteur 2
#define DISTSECTEUR3 964    //longeur du secteur 36
#define NBCARS 2   //Nombre de voitures participantes
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
voiture cars[NBCARS];

void initialisation_voiture(){
    int i;
    for ( i = 0 ; i<NBCARS; i++ ){
        cars[i].numero=i+1;
        cars[i].chrono.Tour=0.0;
        cars[i].vitesse=0.0;
        cars[i].chrono.s1=0.0;
        cars[i].chrono.s2=0.0;
        cars[i].chrono.s3=0.0;
        cars[i].chrono.TotalTime=0.0;
        cars[i].distance=0.0;
        cars[i].nbTour=0;
        cars[i].out=0;
        cars[i].pit = 1;
    }
    cars[12].numero=25;
}
void chronos(int i){
    cars[i].chrono.Tour += 1.0;
}


void fct_pitstop(int i){
    int pit_alea = (rand() % 10) +1;
    float pit_arret = (rand() % 8) + 1;
    if ( pit_alea == 2 && cars[i].pit < MAXPIT){
      cars[i].chrono.Tour += pit_arret + 2;
      cars[i].pit += 1;
      cars[i].vitesse = 0;
    }
}

void fct_sector(int i){ //Il faudrait remettre la distance secteur à zéro.

  int brokkenEngine = (rand() % 3000) + 1;
  if ( brokkenEngine == 190 ){
   // cars[i].out = 1;
  }
  if ( cars[i].out != 1 && cars[i].distance >= DISTSECTEUR1 && cars[i].distance < DISTSECTEUR2 )
    cars[i].chrono.s1=cars[i].chrono.Tour;

  else if ( cars[i].out != 1 && cars[i].distance >= DISTSECTEUR2 && cars[i].distance < DISTSECTEUR3)
    cars[i].chrono.s2=cars[i].chrono.Tour;

  else if ( cars[i].out != 1 && cars[i].distance >= DISTSECTEUR3 ){
    fct_pitstop(i);
    cars[i].chrono.s3 = cars[i].chrono.Tour;
    cars[i].chrono.TotalTime += cars[i].chrono.Tour;
    cars[i].chrono.Tour = 0;
    cars[i].nbTour += 1;
    cars[i].distance = 0;

  }
}

void acceleration(int i) {
  printf("Je vous encule\n");
  int token = (rand() % 10) + 1;
 if ( token >= 4 ){
    if(cars[i].vitesse<=VITESSE1) {
       cars[i].vitesse+=ACCEL1;
    }
    else if(cars[i].vitesse<=VITESSE2) {
       cars[i].vitesse+=ACCEL2;
    }
    else if(cars[i].vitesse<=VITESSEMAX1) {
      cars[i].vitesse+=ACCEL3;
    }

  else{
      if(cars[i].vitesse>=VITESSE2) {
        cars[i].vitesse-=40;
      }
      else if(cars[i].vitesse>=VITESSEMAX1) {
        cars[i].vitesse-=70;
    }
  }
}
}
float fonctiondistance(int i){
    //Vitesse initiale( Distance + la vitesse * l'accélaration * le temps au carrée )
    float x;
    x=cars[i].vitesse/3.6;
    return (x);
}

//Sans doute placer le fork ici.
void encourse(int i){
      if(cars[i].out != 1) {
        chronos(i);
        printf("\nencourse\n");
        acceleration(i);
        cars[i].distance += fonctiondistance(i);
        fct_sector(i);

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
    if ((semid=semget(1779,1,0666 | IPC_CREAT)<0)){  //préférable d'utiliser un ftoken ... peut être !
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

void processusEnfant(int numProcessus)
{
    voiture carse;
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
   // semid = semget(1991, 1, 0666);
    //if (semid < 0){
      //printf("semaphores introuvables");
     // exit(0);
   // }
    sleep(1);
    encourse(numProcessus);
    //printf("avant lock sem processus enfant num %d, seWmid numero %d\n",numProcessus, semid);
    printf("voiture numero : %d\n", cars[numProcessus].numero);
    printf("La vitesse :  %lf\n", cars[numProcessus].vitesse);
    printf("Distance parcouru %lf\n", cars[numProcessus].distance);
    printf("Number Tour : %d\n", cars[numProcessus].nbTour);
    printf("Number PIT : %d\n", cars[numProcessus].pit);
    printf("Numbertour : %5.2lf\n", cars[numProcessus].chrono.Tour);
    //locker semaphore

   //p(semid);
    //tableau temporaire pour ecrire où on veut dans la MP
    //*shm_Pt = tab_cars[numProcessus];
    //delocker semaphoreW
    //v(semid);

}
void processusParent(int nbEnfants){
    int x,semid;

    voiture tabCh[nbEnfants];
    //ouvrir les sémaphores qui sont crées dans le main
    semid = semget(1977, 1, 0666);
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

void creerEnfants(int nbEnfants)
{   //tableau de pid enfants
    pid_t tabPidEnfants[nbEnfants];
    pid_t p;
    int a,enAttente,cpt,i;
    //allouer de la memoire pour le tab
    //printf("avant fork\n");
    //creer les enfants
    a=fct_sempetunia();
    printf("a: %d\n",a);
   // fct_open_shm();
    for ( cpt = 0; cpt < nbEnfants; cpt++) {
      if ((p = fork()) == 0) {
        //printf("avant processus enfant %d\n",cpt);
        processusEnfant(cpt);
        printf("cpt: %d\n",cpt);
        sleep(1);
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
            sleep(1);
        }
        //printf("je boucle mais je fais rien !!!\n");
        //if (waitpid(tabPidEnfants[i], NULL, 0) == tabPidEnfants[i])
    } while (enAttente);
    //nettoyage
    //free(tabPidEnfants);
}
 int main(int argc, char* argv[]) {
  srand(time(NULL)); //Random aléatoire pour chaque coup du rand();.
    int numProcessus;

initialisation_voiture();
  while(NBRSECTEUR!=1){
  //affichage(&cars);
  creerEnfants(NBCARS);
  numProcessus=0;

    printf("voiture numero : %d\n", cars[numProcessus].numero);
    printf("La vitesse :  %lf\n", cars[numProcessus].vitesse);
    printf("Distance parcouru %lf\n", cars[numProcessus].distance);
    printf("Number Tour : %d\n", cars[numProcessus].nbTour);
    printf("Number PIT : %d\n", cars[numProcessus].pit);
    printf("Numbertour : %5.2lf\n", cars[numProcessus].chrono.Tour);
  printf("-------------------------------------------------------------------------------------");
  numProcessus=1;

    printf("voiture numero : %d\n", cars[numProcessus].numero);
    printf("La vitesse :  %lf\n", cars[numProcessus].vitesse);
    printf("Distance parcouru %lf\n", cars[numProcessus].distance);
    printf("Number Tour : %d\n", cars[numProcessus].nbTour);
    printf("Number PIT : %d\n", cars[numProcessus].pit);
    printf("Numbertour : %5.2lf\n", cars[numProcessus].chrono.Tour);
  printf("-------------------------------------------------------------------------------------");
  }
}

