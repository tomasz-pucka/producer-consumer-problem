#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>
//niezbedne dane wejsciowe do uruchomienia programu: liczba buforow. najpierw uruchamiamy sterowanie, nastepnie dowolna ilosc producentow lub konsumentow. 
//liczba -1 oznacza brak elementu w buforze
#define MAX_SLEEP 4 //maksymalna ilosc sekund jaka bedzie produkowany element
int main(int argc, char *argv[])
{
srand(time(NULL));
int *l_buforow,*bufor,*l_procesow,*wolne,*zajete,*wolne_zapis,*zajete_odczyt;
int shmid0,shmid1,shmid2,shmid3,shmid4,shmid6,shmid7,z,produkt_index,pid,l_iteracji,sekundy;

//inicjalizacja pamieci dzielonej
if(argc>=2) l_iteracji=atoi(argv[1]);
else {
    printf("blad. brak wystarczajacych danych wejsciowych (liczba iteracji)\n");
    exit(1);
}
//pamiec dzielona
shmid0 = shmget(0x9,sizeof(int),0600);
l_buforow=shmat(shmid0,NULL,0);
shmid1 = shmget(0x1,*l_buforow*sizeof(int),0600);
bufor= shmat(shmid1,NULL,0);
shmid2 = shmget(0x2,sizeof(int),0600);
l_procesow=shmat(shmid2,NULL,0);
shmid3 = shmget(0x3,*l_buforow*sizeof(int),0600);
wolne=shmat(shmid3,NULL,0);
shmid4 = shmget(0x4,*l_buforow*sizeof(int),0600);
zajete=shmat(shmid4,NULL,0);
shmid6 = shmget(0x6,sizeof(int),0600);
wolne_zapis=shmat(shmid6,NULL,0);
shmid7 = shmget(0x7,sizeof(int),0600);
zajete_odczyt=shmat(shmid7,NULL,0);

//kontrola bledow inicjalizacji pamieci wspoldzielonej lub uruchomienia programow w niewlasciwej kolejnosci
if(shmid0==-1 || shmid1==-1 || shmid2==-1 || shmid3==-1 || shmid4==-1 || shmid6==-1 || shmid7==-1) {
    printf("blad. najpierw wlacz program memory, nastepnie program producent lub konsument.\n");
    perror("shmget");
    exit(1);
}

//inicjalizacja semforow
sem_t *sw = sem_open("/sw", O_CREAT, 0600, *l_buforow);
sem_t *sz = sem_open("/sz", O_CREAT, 0600, 0);
sem_t *s_wolne_zapis = sem_open("/s_w_z", O_CREAT, 0600, 1);
sem_t *s_zajete_odczyt = sem_open("/s_z_o", O_CREAT, 0600, 1);
sem_t *s_start = sem_open("/s_start", O_CREAT, 0600, 0);

//kontrola bledow inicjalizacji semaforow
if (sw==SEM_FAILED || sz==SEM_FAILED || s_wolne_zapis==SEM_FAILED || s_zajete_odczyt==SEM_FAILED || s_start==SEM_FAILED){
    perror("sem_open");
    exit(1);
}
pid=getpid();
(*l_procesow)++;
sem_post(s_start); //podniesienie semaforu blokujacego sterowanie
for(z=0;z<l_iteracji;z++)
{
    sem_wait(sz);
        sem_wait(s_zajete_odczyt);
            produkt_index=zajete[*zajete_odczyt];
            zajete[*zajete_odczyt]=-1;
            *zajete_odczyt=(*zajete_odczyt+1)%(*l_buforow);
        sem_post(s_zajete_odczyt);
    
    sekundy=(rand()%MAX_SLEEP)+1; //czas konsumpcji
    sleep(sekundy);
    bufor[produkt_index]=-1;
    printf("%d skonsumowal bufor: #%d w %d sek\n",pid,produkt_index,sekundy);

        sem_wait(s_wolne_zapis);
            wolne[*wolne_zapis]=produkt_index;
            *wolne_zapis=(*wolne_zapis+1)%*l_buforow;
        sem_post(s_wolne_zapis);
    sem_post(sw);
}
(*l_procesow)--;
return 0;
}
