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
int *l_buforow,*bufor,*l_procesow,*wolne,*zajete,*wolne_odczyt,*zajete_zapis;
int shmid0,shmid1,shmid2,shmid3,shmid4,shmid5,shmid8,z,produkt_index,pid,l_iteracji,sekundy;

//inicjalizacja pamieci dzielonej
if(argc>=2) l_iteracji=atoi(argv[1]);
else {
    printf("blad. brak wystarczajacych danych wejsciowych (liczba iteracji)\n");
    exit(1);
}
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
shmid5 = shmget(0x5,sizeof(int),0600);
wolne_odczyt=shmat(shmid5,NULL,0);
shmid8 = shmget(0x8,sizeof(int),0600);
zajete_zapis=shmat(shmid8,NULL,0);

//kontrola bledow inicjalizacji pamieci wspoldzielonej lub uruchomienia programow w niewlasciwej kolejnosci
if(shmid0==-1 || shmid1==-1 || shmid2==-1 || shmid3==-1 || shmid4==-1 || shmid5==-1 || shmid8==-1) {
    printf("blad. najpierw wlacz program memory, nastepnie program producent lub konsument.\n");
    perror("shmget");
    exit(1);
}

//inicjalizacja semforow
sem_t *sw = sem_open("/sw", O_CREAT, 0600, *l_buforow);
sem_t *sz = sem_open("/sz", O_CREAT, 0600, 0);
sem_t *s_wolne_odczyt = sem_open("/s_w_o", O_CREAT, 0600, 1);
sem_t *s_zajete_zapis = sem_open("/s_z_z", O_CREAT, 0600, 1);
sem_t *s_start = sem_open("/s_start", O_CREAT, 0600, 0);

//kontrola bledow inicjalizacji semaforow
if (sw==SEM_FAILED || sz==SEM_FAILED || s_wolne_odczyt==SEM_FAILED || s_zajete_zapis==SEM_FAILED || s_start==SEM_FAILED){
    perror("sem_open");
    exit(1);
}
pid=getpid(); //produkowana wartosc to ID procesu
(*l_procesow)++;
sem_post(s_start); //podniesienie semaforu blokujacego sterowanie
for(z=0;z<l_iteracji;z++)
{
    sem_wait(sw);
        sem_wait(s_wolne_odczyt);
            produkt_index=wolne[*wolne_odczyt];
            wolne[*wolne_odczyt]=-1;
            *wolne_odczyt=(*wolne_odczyt+1)%(*l_buforow);
        sem_post(s_wolne_odczyt);
    
    sekundy=(rand()%MAX_SLEEP)+1;//czas produkcji
    sleep(sekundy);
    bufor[produkt_index]=pid;
    printf("wyprodukowano %d w buforze: #%d w %d sek\n",pid,produkt_index, sekundy);

        sem_wait(s_zajete_zapis);
            zajete[*zajete_zapis]=produkt_index;
            *zajete_zapis=(*zajete_zapis+1)%(*l_buforow);
        sem_post(s_zajete_zapis);
    sem_post(sz);
}
(*l_procesow)--;
return 0;
}
