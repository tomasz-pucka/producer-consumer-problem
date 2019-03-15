#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/msg.h>
//niezbedne dane wejsciowe do uruchomienia programu: liczba buforow. najpierw uruchamiamy sterowanie, nastepnie dowolna ilosc producentow lub konsumentow.
//liczba -1 oznacza brak elementu w buforze
int main(int argc, char *argv[])
{
int *l_buforow,*bufor,*l_procesow,*wolne,*zajete,*wolne_odczyt,*wolne_zapis,*zajete_odczyt,*zajete_zapis;
int temp_sz_val,temp_sw_val,shmid0,shmid1,shmid2,shmid3,shmid4,shmid5,shmid6,shmid7,shmid8,z;

//inicjalizacja pamieci dzielonej
shmid0 = shmget(0x9,sizeof(int),0600 | IPC_CREAT | IPC_EXCL);
l_buforow=shmat(shmid0,NULL,0);
if (argc>=2) *l_buforow=atoi(argv[1]); 
else {
    printf("brak wystarczajacych danych wejsciowych (liczba buforow)\n");
    shmdt(l_buforow);
    shmctl(shmid0,IPC_RMID,NULL);
    exit(1);
}
shmid1 = shmget(0x1,*l_buforow*sizeof(int),0600 | IPC_CREAT | IPC_EXCL);
bufor= shmat(shmid1,NULL,0);
shmid2 = shmget(0x2,sizeof(int),0600 | IPC_CREAT | IPC_EXCL);
l_procesow=shmat(shmid2,NULL,0);
shmid3 = shmget(0x3,*l_buforow*sizeof(int),0600 | IPC_CREAT | IPC_EXCL);
wolne=shmat(shmid3,NULL,0);
shmid4 = shmget(0x4,*l_buforow*sizeof(int),0600 | IPC_CREAT | IPC_EXCL);
zajete=shmat(shmid4,NULL,0);
shmid5 = shmget(0x5,sizeof(int),0600 | IPC_CREAT | IPC_EXCL);
wolne_odczyt=shmat(shmid5,NULL,0);
shmid6 = shmget(0x6,sizeof(int),0600 | IPC_CREAT | IPC_EXCL);
wolne_zapis=shmat(shmid6,NULL,0);
shmid7 = shmget(0x7,sizeof(int),0600 | IPC_CREAT | IPC_EXCL);
zajete_odczyt=shmat(shmid7,NULL,0);
shmid8 = shmget(0x8,sizeof(int),0600 | IPC_CREAT | IPC_EXCL);
zajete_zapis=shmat(shmid8,NULL,0);

//kontrola bledow inicjalizacji pamieci wspoldzielonej
if(shmid0==-1 || shmid1==-1 || shmid2==-1 || shmid3==-1 || shmid4==-1 || shmid5==-1 || shmid6==-1 || shmid7==-1 || shmid8==-1) {
    printf("blad. najpierw wlacz program memory, nastepnie program producent lub konsument.\n");
    perror("shmget");
    shmdt(l_buforow);
    shmdt(bufor);
    shmdt(l_procesow);
    shmdt(wolne);
    shmdt(zajete);
    shmdt(wolne_odczyt);
    shmdt(zajete_odczyt);
    shmdt(wolne_zapis);
    shmdt(zajete_zapis);
    shmctl(shmid0,IPC_RMID,NULL);
    shmctl(shmid1,IPC_RMID,NULL);
    shmctl(shmid2,IPC_RMID,NULL);
    shmctl(shmid3,IPC_RMID,NULL);
    shmctl(shmid4,IPC_RMID,NULL);
    shmctl(shmid5,IPC_RMID,NULL);
    shmctl(shmid6,IPC_RMID,NULL);
    shmctl(shmid7,IPC_RMID,NULL);
    shmctl(shmid8,IPC_RMID,NULL);
    exit(1);
}

//inicjalizacja semaforow
sem_t *sw = sem_open("/sw", O_CREAT | O_EXCL, 0600, *l_buforow);
sem_t *sz = sem_open("/sz", O_CREAT | O_EXCL, 0600, 0);
sem_t *s_wolne_odczyt = sem_open("/s_w_o", O_CREAT | O_EXCL, 0600, 1);
sem_t *s_wolne_zapis = sem_open("/s_w_z", O_CREAT | O_EXCL, 0600, 1);
sem_t *s_zajete_odczyt = sem_open("/s_z_o", O_CREAT | O_EXCL, 0600, 1);
sem_t *s_zajete_zapis = sem_open("/s_z_z", O_CREAT | O_EXCL, 0600, 1);
sem_t *s_start = sem_open("/s_start", O_CREAT | O_EXCL, 0600, 0);

//kontrola bledow inicjalizacji semaforow
if (sw==SEM_FAILED || sz==SEM_FAILED || s_wolne_odczyt==SEM_FAILED || s_wolne_zapis==SEM_FAILED || 
    s_zajete_odczyt==SEM_FAILED || s_zajete_zapis==SEM_FAILED || s_start==SEM_FAILED) {
    printf("blad. najpierw wlacz program memory, nastepnie program producent lub konsument.\n");
    perror("sem_open");
    shmdt(l_buforow);
    shmdt(bufor);
    shmdt(l_procesow);
    shmdt(wolne);
    shmdt(zajete);
    shmdt(wolne_odczyt);
    shmdt(zajete_odczyt);
    shmdt(wolne_zapis);
    shmdt(zajete_zapis);
    shmctl(shmid0,IPC_RMID,NULL);
    shmctl(shmid1,IPC_RMID,NULL);
    shmctl(shmid2,IPC_RMID,NULL);
    shmctl(shmid3,IPC_RMID,NULL);
    shmctl(shmid4,IPC_RMID,NULL);
    shmctl(shmid5,IPC_RMID,NULL);
    shmctl(shmid6,IPC_RMID,NULL);
    shmctl(shmid7,IPC_RMID,NULL);
    shmctl(shmid8,IPC_RMID,NULL);
    sem_unlink("/sw");
    sem_unlink("/sz");
    sem_unlink("/s_w_o");
    sem_unlink("/s_w_z");
    sem_unlink("/s_z_o");
    sem_unlink("/s_z_z");
    sem_unlink("/s_start");
    exit(1);
}

//inicjalizacja zmiennych i tablic wspoldzielonych
*l_procesow=0;
*wolne_odczyt=0;
*wolne_zapis=0;
*zajete_odczyt=0;
*zajete_zapis=0;
for (z=0; z<*l_buforow; z++){
    bufor[z] = -1;
    zajete[z] = -1;
    wolne[z] = z;
}

//wyswietlenie stanu poczatkowego buforow
system("clear");
sem_getvalue(sz,&temp_sz_val);
sem_getvalue(sw,&temp_sw_val);
printf("liczba wolnych: %d, liczba zajetych: %d\n", temp_sw_val, temp_sz_val);
printf("indeks:\t");
for (z=0; z<*l_buforow; z++) printf("%d\t",z);
printf("\nbufory:\t");
for (z=0; z<*l_buforow; z++) printf("%d\t",bufor[z]);
printf("\nwolne:\t");
for (z=0; z<*l_buforow; z++) printf("%d\t",wolne[z]);
printf("\nzajete:\t");
for (z=0; z<*l_buforow; z++) printf("%d\t",zajete[z]); 
printf("\n");

sem_wait(s_start);//oczekiwanie na jakikolwiek proces

while(*l_procesow!=0){//wyswietlanie stanu buforow w czasie rzeczywistym
    system("clear");
    sem_getvalue(sz,&temp_sz_val);
    sem_getvalue(sw,&temp_sw_val);
    printf("liczba wolnych: %d, liczba zajetych: %d\n", temp_sw_val, temp_sz_val);
    printf("indeks:\t");
    for (z=0; z<*l_buforow; z++) printf("%d\t",z);
    printf("\nbufory:\t");
    for (z=0; z<*l_buforow; z++) printf("%d\t",bufor[z]);
    printf("\nwolne:\t");
    for (z=0; z<*l_buforow; z++) printf("%d\t",wolne[z]);
    printf("\nzajete:\t");
    for (z=0; z<*l_buforow; z++) printf("%d\t",zajete[z]); 
    printf("\n");
}

//zwalnianie pamieci
shmdt(l_buforow);
shmdt(bufor);
shmdt(l_procesow);
shmdt(wolne);
shmdt(zajete);
shmdt(wolne_odczyt);
shmdt(zajete_odczyt);
shmdt(wolne_zapis);
shmdt(zajete_zapis);
shmctl(shmid0,IPC_RMID,NULL);
shmctl(shmid1,IPC_RMID,NULL);
shmctl(shmid2,IPC_RMID,NULL);
shmctl(shmid3,IPC_RMID,NULL);
shmctl(shmid4,IPC_RMID,NULL);
shmctl(shmid5,IPC_RMID,NULL);
shmctl(shmid6,IPC_RMID,NULL);
shmctl(shmid7,IPC_RMID,NULL);
shmctl(shmid8,IPC_RMID,NULL);
sem_close(sw);
sem_close(sz);
sem_close(s_wolne_odczyt);
sem_close(s_wolne_zapis);
sem_close(s_zajete_odczyt);
sem_close(s_zajete_zapis);
sem_close(s_start);
sem_unlink("/sw");
sem_unlink("/sz");
sem_unlink("/s_w_o");
sem_unlink("/s_w_z");
sem_unlink("/s_z_o");
sem_unlink("/s_z_z");
sem_unlink("/s_start");
return 0;
}
