#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>

#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/shm.h>
#include<sys/sem.h>

#define N 5
 int K = 1;

static struct sembuf buf;

struct mybuf_msg {
	long mtype;
	char text[4];
	
};

void podnies(int semid, int semnum)
{
	buf.sem_num = semnum;
	buf.sem_op = 1;
	buf.sem_flg = 0;

	if(semop(semid, &buf, 1) == -1) {		perror("Blad podnoszenia semafora");		exit(1);	}
}

void opusc(int semid, int semnum) 
{
	buf.sem_num = semnum;
	buf.sem_op = -1;
	buf.sem_flg = 0;

	if(semop(semid,&buf,1) == -1) {		   perror("Blad opuszczania semfora"); 			exit(1);	}
}

unsigned long long getticks(void)
{
	unsigned a, d;
	
	asm volatile("rdtsc" : "=a" (a), "=d" (d));
	
	return ((unsigned long long)a<<32) | d;
}

int main()
{
	int *numberReaders;
	int (*processStat)[2];
	int *ready;
	int *length;
	int *nextMtype;
	int (*whichRead)[N];
	
	//.....................................WSPOLDZIELONE.........................................//
	
	//Zmienna wspoldzielona, zmienna shid1 - czytelnicy, zmienna shmid2 - tabela procesid i rola dwuwymiarowa, zmienna shmid3 - pozwala aby PIDy sie wpisaly do tablicy dopiero wtedy rusza potomne
	int shmid1, shmid2, shmid3, shmid4, shmid5, shmid6;
	
	shmid1 = shmget(45281,sizeof(int),IPC_CREAT|0600);
	
	if(shmid1 == -1) { 		perror("Utworzenie wspoldzielone czytelnikow"); 	exit(1);	}
	
	numberReaders  = (int*)shmat(shmid1, NULL, 0);
	
	if(numberReaders == NULL) 	{ 	perror("Przypisanie wspoldzielone czytelnika"); exit(1); 	}
	
	
	//shmid2 - tabela procesid i rola - tabela dwuwymiarowa
	shmid2 = shmget(45282,sizeof(int[N][2]),IPC_CREAT|0600);
	
	if(shmid2 == -1) { 		perror("Utworzenie wspoldzielonej tabeli procesów"); 	exit(1);	}
	
	processStat  = (int(*)[2])shmat(shmid2, NULL, 0);
	
	if(processStat == NULL) 	{ 	perror("Przypisanie wspoldzielone tabeli procesow"); exit(1); 	}
	
	
	//shmid3 - gotowosc pid w tabeli
	shmid3 = shmget(45287,sizeof(int),IPC_CREAT|0600);
	
	if(shmid3 == -1) { 		perror("Utworzenie wspoldzielonej zmiennej ready"); 	exit(1);	}
	
	ready  = (int*)shmat(shmid3, NULL, 0);
	
	if(ready == NULL) 	{ 	perror("Przypisanie wspoldzielone ready"); exit(1); 	}
	
	(*ready) = 0;
	
	
	//shmid4 - dlugosc kolejki
	shmid4 = shmget(45288,sizeof(int),IPC_CREAT|0600);
	
	if(shmid4 == -1) { 		perror("Utworzenie wspoldzielonej zmiennej ready"); 	exit(1);	}
	
	length  = (int*)shmat(shmid4, NULL, 0);
	
	if(length == NULL) 	{ 	perror("Przypisanie wspoldzielone ready"); exit(1); 	}
	
	(*length) = 0;
	
	
	//shmid5 - wolne mtype kolejki
	shmid5 = shmget(45292,sizeof(int[K]),IPC_CREAT|0600);
	
	if(shmid5 == -1) { 		perror("Utworzenie wspoldzielonej zmiennej mtype kolejki"); 	exit(1);	}
	
	nextMtype  = (int*)shmat(shmid5, NULL, 0);
	
	if(nextMtype == NULL) 	{ 	perror("Przypisanie wspoldzielone ready"); exit(1); 	}
	
	
	
	//shmid6 - tabela procesid i rola - tabela dwuwymiarowa
	shmid6 = shmget(45294,sizeof(int[K][N]),IPC_CREAT|0600);
	
	if(shmid6 == -1) { 		perror("Utworzenie wspoldzielonej tabeli kto ma odebrac"); 	exit(1);	}
	
	whichRead  = (int(*)[N])shmat(shmid6, NULL, 0);
	
	if(whichRead == NULL) 	{ 	perror("Przypisanie wspoldzielone tabeli kto ma odebrac"); exit(1); 	}
	
	//.....................................SEMAFORY.........................................//
	
	int semid;

	semid = semget(45212, 6, IPC_CREAT|IPC_EXCL|0600);
	if(semid == -1 ) 
	{		
		semid = semget(45283, 6, 0600);
		
		if(semid = -1) {	perror("Utworzenie semafora nie udalo sie"); 		exit(1);	}
	}

	if(semctl(semid,0,SETVAL,1) == -1) { perror("Inicjacja semafora czytelnikow"); exit(1); } //czytelnicy
	if(semctl(semid,1,SETVAL,1) == -1) { perror("Inicjacja semafora pisarzy    "); exit(1); } //pisarze
	if(semctl(semid,2,SETVAL,1) == -1) { perror("Inicjacja semafora tabeli     "); exit(1); } //tabela
	if(semctl(semid,3,SETVAL,1) == -1) { perror("Inicjacja semafora ready     "); exit(1); } //ready
	if(semctl(semid,4,SETVAL,1) == -1) { perror("Inicjacja semafora kolejki     "); exit(1); } //dlugosc kolejki
	if(semctl(semid,5,SETVAL,1) == -1) { perror("Inicjacja semafora nextPID     "); exit(1); } //dlugosc nextPID
	
	
	//.....................................KOLEJKA KOMUNIKATOW.........................................//

	int msgid = msgget(45284,IPC_CREAT|IPC_EXCL|0600);
	if(msgid == -1)
	{
		msgid = msgget(45284,IPC_CREAT|0600);
		if(msgid == -1) { perror("Tworzenie kolejki komunikatow"); exit(1); }
	}
	
	
	
	
	//.....................................URUCHAMIANIE N PROCESOW........................................//
	
	for(int i=0; i<N; i++)
	{
		int fork_value = fork();
		
		if(fork_value == -1) 	{	perror("Nie utworzylem procesu"); exit(1); 		}
		else if(fork_value == 0)					//............................PROCES POTOMNY.............................//
		{
			while(1)
			{
				opusc(semid,3);
				if((*ready) == N) 
				{
					podnies(semid,3);
					break;
				}
				else
				{
					podnies(semid,3);
					sleep(3);
				}
			}
		
			int myPID = processStat[i][0];
			
			

			while(1) 
			{
				struct mybuf_msg buf_msg;
				sleep(getticks()%10); //faza relaksu
				
				int tmp = getticks()%10;
				if(tmp <= 4) //czytelnik
				{
					printf("Proces - %d czytelnik \n",myPID);
					//.....................................ZMIANA TRYBU - CZYTELNIK ........................................//
					opusc(semid,2);
					processStat[i][1] = 0;
					podnies(semid,2);
					
					opusc(semid,0);
					
					if((*numberReaders) == 0)
					{
						(*numberReaders)++;
						opusc(semid,1);
					}
					else
						(*numberReaders)++;
					
					podnies(semid,0);
					
					int mustRead = 0;
					char message[4];
					
					opusc(semid, 4); //opuszczam length czyli jakby anektuje wykaz kolejek
					
					
					
							
			for(int kk=0; kk<K; kk++)//kszukamy po tablicy nextMtype
			{
				if(nextMtype[kk] == 1) //jest komunikat wiec sprawdzamy czy on jest dla nas i on ma swoj mtype = kk+1
				{
					if(msgrcv(msgid, &buf_msg, sizeof(buf_msg.text), kk+1 , 0) != -1)
					{
						for(int j=0; j<N; j++)
						{
							
							if(whichRead[kk][j] == myPID) //ten komunikat byl tez dla mnie
							{
								
								message[0] = buf_msg.text[0];
								message[1] = buf_msg.text[1];
								message[2] = buf_msg.text[2];
								
								mustRead = 1;
								
								whichRead[kk][j] = 0;
								
								
									//sprawdzamy czy ktos jeszcze musi przeczytac jesli tak to wysylamy ten komunikat
									int send = 0;
									for(int jj=0; jj<N; jj++)
									{
										
										if(whichRead[kk][jj] != 0)
										{
											send = 1;
											break;
										}
									}

									if(send) //ktos musi jeszcze przeczytac
									{
										
										if(msgsnd(msgid,&buf_msg,sizeof(buf_msg.text), 0) == -1)
										{
											perror("Blad wyslania komunikatu");
											exit(1);
										}	
									}
									else 
									{
										
										//juz nikt tego nie musi przeczytac wiec nie wysylamy i zmniejszamy length 
										//opusc(semid,4);
										nextMtype[kk] = 0; //zwalniamy komunikat
										
										(*length)--;
										//printf("LENGTH = %d \n", *length);
										//podnies(semid,4);
										
										
									}
								
								
								kk = K; //przerwie nam petle ale dopiero po zakonczeniu tego bloku
								break;
							}
							else//komunikat nie byl dla mnie
							{
								
								
								if(msgsnd(msgid,&buf_msg,sizeof(buf_msg.text), 0) == -1)
									{
										perror("Blad wyslania komunikatu");
										exit(1);
									}	
							}
						}//for wewnetrzny
						
						
						
					}//msgrcv
					
				}//if(nextMtype[kk] == 1;
			}//for przeszukujacy kolejke
					
				
					
					
					podnies(semid,4);
						
						if(mustRead)
						{
							
							//symulujemy czytanie komunikatu - mozliwe z innymi procesami 
							printf("Komunikat ODCZYTANY - proces %d - zawartosc = %s \n", myPID, message);
							sleep(3); //tyle czasu czytam dzielo 
						}
					
					
					
					opusc(semid,0);
					
					(*numberReaders)--;
					
					if((*numberReaders) == 0) {
						podnies(semid,1);	
					}
					
					podnies(semid,0);
					
					
				}											//CZYTELNIK
				else 										//PISARZ
				{
					//.....................................ZMIANA TRYBU - PISARZ........................................//
					opusc(semid,2);
					processStat[i][1] = 1;
					podnies(semid,2);
					
					printf("Proces - %d pisarz \n",myPID);
					
					opusc(semid,1); //mutex pisarza
					
					//.................................... ODCZYT - PISARZ........................................//
					
				
					int mustRead = 0;
					char message[4];
					
					opusc(semid, 4); //opuszczam length czyli jakby anektuje wykaz kolejek
					 
					
							
							
			for(int kk=0; kk<K; kk++)//kszukamy po tablicy nextMtype
			{
				if(nextMtype[kk] == 1) //jest komunikat wiec sprawdzamy czy on jest dla nas i on ma swoj mtype = kk+1
				{
					if(msgrcv(msgid, &buf_msg, sizeof(buf_msg.text), kk+1 , 0) != -1)
					{
						for(int j=0; j<N; j++)
						{
							
							if(whichRead[kk][j] == myPID) //ten komunikat byl tez dla mnie
							{
								//message = buf_msg.text;
								message[0] = buf_msg.text[0];
								message[1] = buf_msg.text[1];
								message[2] = buf_msg.text[2];
								
								mustRead = 1;
								
								whichRead[kk][j] = 0;
								
																		
									//sprawdzamy czy ktos jeszcze musi przeczytac jesli tak to wysylamy ten komunikat
									int send = 0;
									for(int jj=0; jj<N; jj++)
									{
										printf("Sprawdzam czy cos zostalo - CZYTELNIK: %d \n", whichRead[kk][jj]);
										if(whichRead[kk][jj] != 0)
										{
											send = 1;
											break;
										}
									}

									if(send) //ktos musi jeszcze przeczytac
									{
										
										if(msgsnd(msgid,&buf_msg,sizeof(buf_msg.text), 0) == -1)
										{
											perror("Blad wyslania komunikatu");
											exit(1);
										}	
									}
									else 
									{
										
										//juz nikt tego nie musi przeczytac wiec nie wysylamy i zmniejszamy length 
										//opusc(semid,4);
										nextMtype[kk] = 0; //zwalniamy komunikat
										
										(*length)--;
										//printf("LENGTH = %d \n", *length);
										//podnies(semid,4);
										
										
									}
								
								
								kk = K; //przerwie nam petle ale dopiero po zakonczeniu tego bloku
								break;
							}
							else//komunikat nie byl dla mnie
							{
								
								
								if(msgsnd(msgid,&buf_msg,sizeof(buf_msg.text), 0) == -1)
									{
										perror("Blad wyslania komunikatu");
										exit(1);
									}	
							}
						}//for wewnetrzny
						
						
						
					}//msgrcv
					
				}//if(nextMtype[kk] == 1;
			}//for przeszukujacy kolejke
					
				
					
					podnies(semid,4);
						
						if(mustRead)
						{
							
							//symulujemy czytanie komunikatu - mozliwe z innymi procesami 
							printf("Komunikat ODCZYTANY - proces %d - zawartosc = %s \n", myPID, message);
							sleep(3); //tyle czasu czytam dzielo 
						}
					
					//.................................... NOWE DZIELO - PISARZ........................................//
					opusc(semid,4);
					if((*length) < K) //jest miejsce w kolejce komunikatow
					{
						buf_msg.text[0] = 'T';
						buf_msg.text[1] = 'A';
						buf_msg.text[2] = 'K';
						
						//szukamy ktory mtype jest wolny
						int tmp3 = 0;
						for(int u = 0; u < K; u++)
							if(nextMtype[u] == 0)
							{
								tmp3 = u+1;
								break;
							}
						
						buf_msg.mtype = tmp3;					
						
						opusc(semid,2);
							//sprawdzam komu wyslac
							
							for(int j=0; j<N; j++)
							{
								if(processStat[j][1] == 0)
								{
									whichRead[(*length)][j] = processStat[j][0];									
								}
								else
								{
									whichRead[(*length)][j] = 0;
								}								
							}
						
						if(msgsnd(msgid,&buf_msg,sizeof(buf_msg.text), 0) == -1)
						{
							perror("Blad wyslania komunikatu");
							exit(1);
						}
						
						
						(*length)++;
						nextMtype[tmp3-1] = 1; //ten komunikat jest zajety w tej chwili 
						
						printf("WYSYLAM KOMUNIKAT - PROCES %d \n", myPID);
						
						podnies(semid,2);
					}
					podnies(semid,4);
					
				podnies(semid,1);
				
				}											//ELSE PISARZ
				
			}//while(true);

		
		}//koniec kodu procesu potomnnego
		else if(fork_value > 0) //kod procesu macierzystego
		{
			opusc(semid,3);
			opusc(semid,2);
			
			processStat[i][0] = fork_value;
			processStat[i][1] = 0; //0 - oznacza czytelnika, 1 - oznacza pisarza, na poczatku kazdy jest czytelnikiem 
			
			(*ready)++;
			
			podnies(semid,2);
			podnies(semid,3);
			
			printf("Proces macierzysty sie zakonczyl, %d \n", *ready);
			//wait(NULL);
			
		}//koniec kodu procesu macierzytego
			
			
	}//for dla n procesow
	
}//main
			
		
