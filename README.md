# Readers-writers-problem

Instrukcja kompilacji i uruchomienia:

	wystarczy skompilować plik i uruchomić.
	
	gcc czytelnicy_pisarze.c -o czytelnicy_pisarze.out
	
	 ./czytelnicy_pisarze.out
	
	Zadanie polega na realizacji problemu czytelników i pisarzy przy czym:
	- jest ustalona liczba procesów — N;
	- każdy proces działa naprzemiennie w dwóch fazach: fazie relaksu i fazie korzystania z czytelni;
	- w dowolnym momencie fazy relaksu proces może (choć nie musi) zmienić swoją rolę: z pisarza na czytelnika lub z czytelnika na pisarza;
	- przechodząc do fazy korzystania z czytelni proces musi uzyskać dostęp we właściwym dla swojej aktualnej roli trybie;
	- pisarz umieszcza efekt swojej pracy — swoje dzieło — w formie komunikatu w kolejce komunikatów, gdzie komunikat ten pozostaje do momentu, aż wszystkie procesy, które w momencie wydania dzieła były w roli czytelnika, nie odczytają go (po odczytaniu przez wszystkie wymagane procesy komunikat jest usuwany);
 	- pojemność kolejki komunikatów — reprezentującej półkę z książkami — jest ograniczona, tzn. nie może ona przechowywać więcej niż K dzieł; 
	- podczas pobytu w czytelni proces (również pisarz) czyta co najwyżej jedno dzieło, po czym czytelnik opuszcza czytelnię, a pisarz czeka na miejsce w kolejce, żeby opublikować kolejne dzieło.
	
	Idea realizacji:
	W swoim rozwiązaniu posiadam kolejkę komunikatów o wielkości K oraz N procesów. Zgodnie z ideą algorytmu uruchamiam N procesów. Procesy działają w dwóch rolach – czytelnika i pisarza. Jeśli pisarz pisze to sprawdza kto obecnie jest w trybie czytelnika i do niego kieruje swoje dzieło. Do realizacji wykorzystałem tablice w której dla każdego komunikatu określam czytelników do których czytelników jest on kierowany (taki spis). Użytkownik wchodząc do biblioteki sprawdza czy jest coś dla niego w tym spisie, jeśli tak to odczytuje i bierze daną pozycję i czyta. Implementuje to poprzez Sleep.
	Semafory a także zmienne współdzielone opisałem w kodzie.
	Każdy proces działa w pętli nieskończonej. W zależności od wartości wylosowanej albo jest czytelnikiem albo jest pisarzem.
	
	Gdy jest czytelnikiem:
	a. Jeśli mu się uda wejść to zwiększa liczbę czytelników
	b. Następnie sprawdza na liście czy został do niego skierowany jakiś komunikat.
	c. Jeśli tak to bierze ten komunikat i czyta. Jako że gdy czyta z kolejki komunikatów komunikat jest usuwany dodaje go ponownie jeśli wie że ktoś jeszcze go nie przeczytał.
	d. W przeciwnym wypadku już go nie dodaje i pisarz może stworzyć nowy komunikat
	e. Następnie wychodzi z czytelni i udaje się na relaks.

	Gdy jest pisarzem:
	a. Pisarz może wejść jedynie do czytelni podczas gdy nie ma w niej innego pisarza ani czytelnika.
	b. Pisarz również może czytać dzieła. Jeśli jest jakieś dzieło skierowane do niego to je czyta (analogicznie jak czytelnik)
	c. Następnie sprawdza czy ma jakieś wolne miejsce w kolejce. Jeśli tak to tworzy dzieło. Przy czym sprawdza kto obecnie jest w czytelnikiem i ustawia w tabeli że dane dzieło (komunikat) mają przeczytać dokładnie te procesy. Następnie wychodzi z czytelni
	W kodzie zawałem dodatkowe komentarze, które ułatwiają analizę kodu, dlatego nie opisuję tutaj dokładnie znaczenia każdego z semaforów czy elementów pamięci współdzielonej.
	



