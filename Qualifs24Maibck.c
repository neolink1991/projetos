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


/** FORK DANS FOR : http://stackoverflow.com/questions/1381089/multiple-fork-concurrency **/


#define MAXTOURS1 20
#define MAXTOURS2 15
#define MAXTOURS3 10
#define nbVoiture 24
#define tempsMin 10.0
#define tempsMax 30.0


typedef struct voiture{
    int num; //numero de voiture
    float bestS1; // meilleur temps premier secteur personnel pour la voiture
    float bestS2; //..
    float bestS3; //..
    float bestTour; //s1+s2+s3
    float total; //temps de course total pour cette voiture
    char etat; //C = en course, A = abandon
    int arretsStands; //commence à 0, peut aller jusqu'a 3-4 arrets pour la course
}VOITURE;


//conserver les meilleurs temps secteurs
typedef struct secteur{
    int num; //numero de voiture
    float temps; //meilleur temps secteur
}SECTEUR;


//maintenir a jour le classement
typedef struct classement{
    int num;      //numero de voiture
    float secteur[3];  //temps pour chaque secteur recu
    float total; //temps de course totale de la voiture
    float tour; //temps pour tour
    int laps;    //nb de tours que la voiture a fait
    float ecart; //ecart par rapport au premier
    char etat; //C = en course, A = abandon
    int arretsStands; //commence à 0, peut aller jusqu'a 3-4 arrets pour la course
}CLASSEMENT;

typedef struct chrono{
    float temps; //temps généré aléatoirement
    int Num_Voiture; //numéro de voiture
}CHRONO;


    VOITURE tabV[nbVoiture];
    SECTEUR tabS[3];
    CLASSEMENT tabCl[nbVoiture];
    CHRONO tabCh[nbVoiture];
    CHRONO * shmPt ;


void qualifs();
void qualifs1();
void qualifs2();
void qualifs3();
void timer(int nbQualif,int semid);
void lancerQualifs(int nbQualif,int semid);
void initTab(VOITURE tabV[],CLASSEMENT tabCl[],SECTEUR tabS[],CHRONO tabCh[]);
int pause();
int InitSemId(int nbQualif);
int lancerQualifs1();
int lancerQualifs2();
int lancerQualifs3();
void OuvrirMP() ;
int p(int semid);
int v(int semid);
void processusEnfant(int numProcessus);
void creerEnfants(int nbEnfants);
int vParent(int semid,int nbQualif);
int pParent(int semid,int nbQualif);


int main(){


    //INITIALISER TOUS LES TABLEAUX
    initTab(tabV,tabCl,tabS,tabCh);

    qualifs();


return 0;
}

//initialiser les tableaux
void initTab(VOITURE tabV[],CLASSEMENT tabCl[],SECTEUR tabS[],CHRONO tabCh[]){

    int i;

    for (i=0;i<nbVoiture;i++){
        //init tab voiture
        tabV[i].bestS1 = 99999.0;
        tabV[i].bestS2 = 99999.0;
        tabV[i].bestS3 = 99999.0;
        tabV[i].bestTour = 999999.0;
        tabV[i].total = 0.0;
        tabV[i].etat = 'C'; //toutes les voitures commencent en course
        tabV[i].arretsStands = 0; //aucun arret aux stands

        //init tab classement
        tabCl[i].ecart = 0.0;
        tabCl[i].laps = 0;
        tabCl[i].secteur[0] = 0.0;
        tabCl[i].secteur[1] = 0.0;
        tabCl[i].secteur[2] = 0.0;
        tabCl[i].total = 0.0;
        tabCl[i].tour = 0.0;
        tabCl[i].etat ='C'; //toutes les voitures commencent en course
        tabCl[i].arretsStands = 0; //aucun arret aux stands

        //init tab Chrono
        tabCh[i].temps = 0.0;


        //car la voiture numero 13 n'existe pas
        if (i>=12){
            tabV[i].num = i+2;
            tabCl[i].num = i+2;
            tabCh[i].Num_Voiture = i+2;


        }

        else{
            tabV[i].num = i+1;
            tabCl[i].num = i+1;
            tabCh[i].Num_Voiture = i+1;
        }


    }
        //init tab secteur
        for (i=0;i<3;i++){
            tabS[i].num = 0;
            tabS[i].temps = 9999999.0;

        }



}

void qualifs(){
    //crée de la memoire partagé de 24 places de type CHRONO
    OuvrirMP();

    //20min
   qualifs1();
   pause();
   //15min
   qualifs2();
   pause();
   //10min
   qualifs3();


}

void qualifs1(){
/**  DUREE DE 20 MIN **/
    int semid;

    semid = InitSemId(1);

    int i = 0;
    while (i<MAXTOURS1) {

        timer(1,semid);
        i++;
    }
    printf("Seance de Qualification numero 1 Fini !\n");
}

void qualifs2(){
/**  DUREE DE 15 MIN **/
    int semid;

    semid = InitSemId(2);

   int i = 0;
    while (i<MAXTOURS2) {

        timer(2,semid);
        i++;
    }


    printf("Seance de Qualification numero 2 Fini !\n");



}

void qualifs3(){
/**  DUREE DE 10 MIN **/

    int semid;

    semid = InitSemId(3);
   int i = 0;
    while (i<MAXTOURS3) {

        timer(3,semid);
        i++;
    }


    printf("Seance de Qualification numero 3 Fini !\n");


}

void timer(int nbQualif,int semid){


    struct timeval t;



    t.tv_sec = 1;
    t.tv_usec = 0;

    select(0, NULL, NULL, NULL, &t);

    lancerQualifs(nbQualif,semid);


     }

void lancerQualifs(int nbQualif,int semid){


    switch (nbQualif){


        case 1:{


            lancerQualifs1( semid);


        }break;
         case 2:{

            //printf("%d\n",nbQualif);
            lancerQualifs2( semid);


        }break;
         case 3:{

            //printf("%d\n",nbQualif);
            lancerQualifs3(semid);


        }break;
         default:{

            printf("ERROR");

        }break;





    }


}

int lancerQualifs1(){

    printf("Qualifs %d lance \n",1);

    //24 forks


    creerEnfants(24);




return 0;
}

int lancerQualifs2(){

    printf("Qualifs %d lance \n",2);

    //17 forks


    creerEnfants(17);








return 0;
}

int lancerQualifs3(){

    printf("Qualifs %d lance \n",3);

    //10 forks

    creerEnfants(10);





return 0;

}

//creation initialisation sémaphore
int InitSemId(int nbQualif){


    int valeur;

    //semaphore est initialisé a la valeur correspondant au nombre de voitures
    switch (nbQualif){

        case 1 : valeur = 24;break;

        case 2 : valeur = 17;break;

        case 3 : valeur = 10;break;

        default: printf("error semid nbqualif");break;

    }



    // utilisé pour semctl
    union semun  {
        int val;               /* used for SETVAL only */
        struct semid_ds *buf;  /* used for IPC_STAT and IPC_SET */
        ushort *array;         /* used for GETALL and SETALL */
    };


    union semun semun1;
    //initialisé pour correspondre au nombre de voitures
    semun1.val = valeur;

    int semid;

    //crée 1 semaphore
   if ((semid = semget(1991, 1, 0666 | IPC_CREAT) < 0))  {
      perror("creating semaphore");
      exit(EXIT_FAILURE);
   }



    //initialise le semaphore numero 0 à 24
   if (semctl(semid, 0, SETVAL, semun1 ) < 0){
        perror("initialisation semaphore ");
        exit(EXIT_FAILURE);
   }
    else printf("semaphore de %d cree\n",valeur);


return semid;

}


//créer ou ouvrir MP si éxiste dèja
void OuvrirMP() {


    int taille = sizeof(CHRONO);
    taille = taille * 24;

    int shmid1 = shmget(1990, (taille),  IPC_CREAT | 0666 ); // ouvre ou le creee le segment

    shmPt = (CHRONO*)shmat(shmid1, NULL, 0);           // on obtient l'address, retourn un pointer, retourne -1 si erreur

    if (shmPt == (CHRONO*)(-1))   // check de shmPt
            perror("shmat");

}


//un processus enfant qui va generer son chrono et écrire dans la MP
void processusEnfant(int numProcessus){
    int semid;

    CHRONO voiture;
    float chrono;
    CHRONO tabChTemp[nbVoiture];

    srand((unsigned)time(0));
    chrono = tempsMin + (float)rand() / ((float) RAND_MAX / (tempsMax - tempsMin));

    voiture.Num_Voiture = numProcessus;
    voiture.temps = chrono;

    printf("dans processus enfant num %d\n",numProcessus);


    //ouvrir les sémaphores qui sont crées dans le main
    semid = semget(1991, 1, 0666);
    if (semid < 0){
        printf("semaphores introuvables");
        exit(0);
    }

    printf("avant lock sem processus enfant num %d, semid numero %d\n",numProcessus, semid);
    //locker semaphore
    p(semid);

    //tableau temporaire pour ecrire où on veut dans la MP
    printf("apres lock sem processus enfant num %d\n",numProcessus);

    int x;
     for (x = 0; x<nbVoiture;x++){

            tabChTemp[x] =  *shmPt;
        }

    tabChTemp[numProcessus] = voiture;

    for (x = 0; x<nbVoiture;x++){

            *shmPt = tabChTemp[x];
        }


    //delocker semaphore
    v(semid);


}


//serveur qui lit et traite les infos
void processusParent(int nbEnfants){
    int x,semid;
    //ouvrir les sémaphores qui sont crées dans le main
    semid = semget(1991, 1, 0666);
    if (semid < 0){
        printf("semaphores introuvables processus parent");
        exit(0);
    }


    //on bloque le sémaphore
    pParent(semid,nbEnfants);

    //on va chercher les données en mémoire partagée
        for (x = 0; x<nbVoiture;x++){

            tabCh[x] =  *shmPt;
        }

    //déverouiller le semaphore
    vParent(semid,nbEnfants);


    for (x = 0; x<nbVoiture;x++){

            printf("NumVoiture = %d \t Chrono = %lf  \n",tabCh[x].Num_Voiture, tabCh[x].temps);
        }


}



//opération p pour locker accès MP pour un sem
int p( int semid) {

    struct sembuf p_buf;

    p_buf.sem_num = 0;

    //attends que ressource soit disponible (sem_op = 1) puis prends la ressource
    p_buf.sem_op = -1;

    //on attends jusqu'à ce que le sémaphore soit libre
    p_buf.sem_flg = SEM_UNDO;

    int valRetour = semctl(semid, 0, GETVAL, 0);


    printf("juste avant semop\n");
    //verifie la valeur du sémaphore avant d'effectuer l'opération
    if (valRetour > 0){



    if (semop(semid, &p_buf,1) == -1)  {
        perror("Operation P échoué");
        return 0;
    }
    printf("juste apres semop\n");
    }
    return 1;
}

//opération pour delocker l'accés a la MP
int v(int semid) {

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
int pParent(int semid, int nbQualif){

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
int vParent(int semid,int nbQualif){

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

void creerEnfants(int nbEnfants){

   //tableau de pid enfants
    pid_t tabPidEnfants[nbEnfants];
    pid_t p;
    int enAttente,ii,i;


    //allouer de la memoire pour le tab


    printf("avant fork\n");
    //creer les enfants
    for ( ii = 0; ii < nbEnfants; ++ii) {
        if ((p = fork()) == 0) {
            printf("avant processus enfant %d\n",ii);
            /**  PROCESSUS ENFANT VIENT ICI **/
            processusEnfant(ii);


            exit(0);
        }
        else {
            /**  PROCESSUS PARENT VIENT ICI **/

            tabPidEnfants[ii] = p;
            processusParent(nbEnfants);


        }
    }


    // il faut attendre que les enfants exitent pour eviter les zombies
     do {
        enAttente = 0;
        for (i = 0; i < nbEnfants; ++i) {
            if (tabPidEnfants[i] > 0) {
                if (waitpid(tabPidEnfants[i], NULL, WNOHANG) == 0) {
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
    } while (enAttente);

    //nettoyage
    free(tabPidEnfants);



}

//faire une pause apres chaque séance de qualification
int pause(){


    struct timeval t;

    printf("\n LA PROCHAINE SEANCE DE QUALIFICATIONS COMMENCERA DANS 2 HEURES  \n");

    t.tv_sec = 5;
    t.tv_usec = 0;

    select(0, NULL, NULL, NULL, &t);

    printf("\n LA PROCHAINE SEANCE DE QUALIFICATIONS VA COMMENCER \n");

return 0;
}
