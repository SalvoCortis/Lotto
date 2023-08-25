#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024
#define RUOTE 11

char buffer[BUFFER_SIZE];
/* contiene le ruote giocabili */
char* array_ruote[RUOTE] = {"Bari", "Cagliari", "Firenze", "Genova", "Milano", "Napoli", "Palermo", "Roma", "Torino", "Venezia", "Nazionale"}; 


void pulisci() {	/* pulisce il buffer */
	int i;
	for(i = 0; i < BUFFER_SIZE; i++)
		buffer[i] = '\0';
}

int invia_risposta(int cliente, char* msg) { /* invia dim e msg */

	int len = strlen(msg);
	uint16_t dim = htons(len);
	int ret = send(cliente, (void*)&dim, sizeof(uint16_t), 0);
	if (ret < 0)
 		perror("Errore in fase di risposta"); 

	ret = send(cliente, (void*)msg, len, 0);
	if (ret < 0)
		perror("Errore in fase di risposta"); 

	return ret;

}

int ricevi_risposta(int cliente) {	/* riceve dim e msg */

	uint16_t lmsg, len;
	int ret, lunghezza;

	ret = recv(cliente, (void*)&lmsg, sizeof(uint16_t), 0);
	if(ret < 0)
		perror("Errore nella ricezione della dimensione della risposta:\n");

	len = ntohs(lmsg);

	ret = recv(cliente, (void*)buffer, len, 0);
	if(ret < 0)
		perror("Errore nella ricezione della risposta:\n");

	return ret;
}

int controlla_ruote(char* buf) {	/* ritorna l'indice di una ruota */

	int i;
	for(i = 0; i < RUOTE; i++)
		if(strcmp(buf, array_ruote[i]) == 0 || strcmp(buf, "tutte") == 0)
			return i;

	return -1;
}

void inserisci_ruote() {	/* inserisce le ruote di una giocata */

	char buf[15];
	char str[BUFFER_SIZE];
	char cmd[5];
	int i, cfor;
	int ret = 0;
	
	char* vett_ruote[RUOTE] = {"\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0"};

	i = 0;
	while(1) {
		i++;

		for(cfor = 0; cfor < BUFFER_SIZE; cfor++)	/* svuota str */
			str[cfor] = '\0';

		if(i >= 12)
			break;

		if(i != 1) {
			while(1) {
 				printf("Finito? (s o n): ");
				scanf("%s", cmd);
				while(getchar() != '\n');
				if(strcmp(cmd, "n") == 0)
					break;
				else
					if(strcmp(cmd, "s") == 0 )
						return;
			}			
		}
		printf("Ruota: ");
		fgets(str, BUFFER_SIZE, stdin);
		str[strlen(str)-1] = '\0';
		strcpy(buf, str);
		ret = controlla_ruote(buf);
		if(ret < 0) {
			printf("La ruota scelta è inesistente. Per favore, riprova.\n");
			continue;
		}

		if(i == 1) {	/* inserisce la prima ruota */
			if(strcmp(vett_ruote[ret], array_ruote[ret]) != 0) {
				vett_ruote[ret] = (char*)malloc((strlen(array_ruote[ret])+1)*sizeof(char));
				strcpy(vett_ruote[ret], array_ruote[ret]);
				strcpy(buffer, buf);
			}
			if(strcmp(buf, "tutte") == 0)	/* gioca tutte le ruote */
				return;
		}
		else { 
			if(strcmp(buf, "tutte") == 0) {	/* controlla se viene indicato 'tutte' dopo che era già stata inserita una ruota, in tal caso le gioca tutte */
				pulisci();
				strcpy(buffer, buf);
				break;
			}
			else {	/* inserisce le altre ruote */
				if(strcmp(vett_ruote[ret], array_ruote[ret]) != 0) {
					vett_ruote[ret] = (char*)malloc((strlen(array_ruote[ret])+1)*sizeof(char));
					strcpy(vett_ruote[ret], array_ruote[ret]);
					strcat(buffer, " ");
					strcat(buffer, buf);
				}
			}
		}
	}	
}

int inserisci_numeri() {	/* inserisce i numeri di una giocata e ritorna 								quanti numeri sono stati giocati */

	int buf, i, j, cfor;
	char cmd[5];
	char str[BUFFER_SIZE];

	int vettore[90];
	for(j = 0; j < 90; j++)
		vettore[j] = 0;

	i = 0;
	while(1) {
		i++;

		for(cfor = 0; cfor < BUFFER_SIZE; cfor++)	/* svuota str */
			str[cfor] = '\0';

		if(i >= 11)
			return 10;
		
		if(i != 1) {
			while(1) {
 				printf("Finito? (s o n): ");
				scanf("%s", cmd);
				while(getchar() != '\n');
				if(strcmp(cmd, "n") == 0)
					break;
				else
					if(strcmp(cmd, "s") == 0 )
						return i-1;
			}
		}	
		printf("Numero: ");
		fgets(str, BUFFER_SIZE, stdin);
		str[strlen(str)-1] = '\0';
		for(cfor = 0; cfor < BUFFER_SIZE; cfor++)
			if(str[cfor] == ' ') {
				cfor = -1;
				break;
			}

		if((atoi(str) <= 0) || (atoi(str) > 90) || cfor == -1) {
			printf("Numero non valido. Per favore, riprova.\n");
			i--;
			continue;
		}		
	
		buf = atoi(str);	

		if(i == 1) {	/* primo numero */
			if(!vettore[buf-1]) {
				vettore[buf-1] = buf;
				sprintf(buffer, "%d", buf);
			}
			else {
				i--;
			}
		}
		else { 
			if(!vettore[buf-1]) {
				vettore[buf-1] = buf;
				char b[3];
				sprintf(b, "%d", buf);
				strcat(buffer, " ");
				strcat(buffer, b);
			}
			else {
				i--;
			}
		}
	}	
}

void inserisci_importi(int numeri) {	/* inserisce gli importi giocati */

	double buf;
	char cmd[5];
	char str[BUFFER_SIZE];
	int i, cfor;

	i = 0;
	while(1) {
		i++;

		for(cfor = 0; cfor < BUFFER_SIZE; cfor++)	/* svuota str */
			str[cfor] = '\0';

		if(i >= 6 || i > numeri)	/* non permette di giocare un importo su una determinata giocata se non ci sono abbastanza numeri. Esempio: tre numeri, massima giocata possibile il terno. */
			break;

		if(i != 1) {
			while(1) {
 				printf("Finito? (s o n): ");
				scanf("%s", cmd);
				while(getchar() != '\n');
				if(strcmp(cmd, "n") == 0)
					break;
				else
					if(strcmp(cmd, "s") == 0 )
						return;
			}
		}	
		printf("Importo: ");
		fgets(str, BUFFER_SIZE, stdin);
		str[strlen(str)-1] = '\0';
		for(cfor = 0; cfor < BUFFER_SIZE; cfor++)
			if(str[cfor] == ' ') {
				cfor = -1;
				break;
			}

		sscanf(str, "%lf", &buf);
	
		if(i == 1) {	/* primo importo: estratto */
			sprintf(buffer, "%.2f ", buf);
			strcat(buffer, "estratto *");
		}
		else { 
			char b[100];
			sprintf(b, "%.2f", buf);
			strcat(buffer, " ");
			strcat(buffer, b);
			if(i == 2)
				strcat(buffer, " ambo *");
			if(i == 3)
				strcat(buffer, " terno *");
			if(i == 4)
				strcat(buffer, " quaterna *");
			if(i == 5)
				strcat(buffer, " cinquina *");
		}
	}	
}

void help(int comando) {

	switch(comando) {
		case 0:
				printf("\n1) help --> mostra i dettagli di un comando specificato.\n2) signup --> viene richiesto di specificare username e password per creare un nuovo utente.\n3) login --> viene richiesto di specificare username e password per effettuare l'autenticazione.\n4) invia_giocata --> invia una giocata.\n5) vedi_giocate --> visualizza le giocate passate o attive in base ad un determinato parametro chiamato 'tipo'.\n6) vedi_estrazione --> visualizza le ultime n estrazioni, dove n e' un parametro che viene richiesto per determinare quante estrazioni si vuole visualizzare partendo dall'ultima estratta.\n7) vedi_vincite --> visualliza le vincite se sono state realizzate e un consuntivo per tipologia di giocata.\n8) esci --> effettua il logout.\n");
				break;

		case 1:
				printf("\nRichiede di inserire un username e una password non superiori a 28 caratteri. Se l'username e' gia' esistente verra' richiesto di inserire un altro username e password.\n");
				break;

		case 2:
				printf("\nRichiede username e password. Se sono valide, l'autenticazione avra' successo e vi verra' assegnato un id di questa sessione. Nel caso di errore, avrete a disposizione altri due tentativi per un totale di tre tentativi. In seuito al fallimento del terzo tentativo, il vostro indirizzo ip sara' bloccato per trenta minuti.\n");
				break;

		case 3:
				printf("\nPermette di inviare una nuova giocata. Bisognera' inserire prime le ruote, poi i numeri ed infine gli importi che si vuole giocare. Sia le ruote, sia i numeri e sia gli importi vengono inseriti uno alla volta. Ogni volta che inserite una ruota, un numero o un importo verra' richiesta conferma se si vuole continuare a giocare un'altra ruota o un altro numero. Per quanto riguarda gli importi, il primo importo che giocate e' l'estratto, il secondo l'ambo, il terzo il terno, il quarto la quaterna e il quinto la cinquina. Ovviamente non e' possibile giocare l'ambo ad esempio se si e' giocato un solo numero.\n");
				break;

		case 4:
				printf("\nMostra le giocate effettuate. Viene richiesto di specificare un paramentro 'tipo'. Se viene specificato '0' verranno mostrate tutte le giocate passate, se invece viene specificato '1' verranno mostrate tutte le giocate in corso(che ancora devono essere estratte).\n");
				break;

		case 5:
				printf("\nMostra le ultime n estrazioni. Il paramentro n determina quante estrazioni volete che vi siano mostrate a partire dalla piu' recente(l'ultima estrazione).\n");
				break;

		case 6:
				printf("\nMostra tutte le vostre vincite se sono state realizzate. Infine mostra anche un consuntivo per tipologia di giocata.\n");
				break;

		case 7:
				printf("\nEffettua il logout(Disconnette il vostro client dal server. Un messaggio di arrivderci verra' visualizzato se il logout e' stato effettuato con successo.\n");
				break;

		default:
				break;

	}

}

void signup(int cliente, char* username, char* password) {

	int ret, cfor;
	
	while(1) {
		pulisci();
		ret = ricevi_risposta(cliente);
		printf("%s\n", buffer);

		if (strcmp(buffer, "Inserisci username: ") == 0) {
			/* printf("Sono entrato nel primo if.\n"); */
			while(1) {
				fgets(buffer, BUFFER_SIZE, stdin);
				buffer[strlen(buffer)-1] = '\0';
				for(cfor = 0; cfor < BUFFER_SIZE; cfor++)
					if(buffer[cfor] == ' ') {
						cfor = -1;
						break;
					}
				if(strlen(buffer) < 30 && cfor != -1)
					break;

				printf("Spazio trovato o numero di caratteri non validi. Username non valido. Inserisci username: ");
			}
			ret = invia_risposta(cliente, buffer);
		}

		if (strcmp(buffer, "Registrazione eseguita con successo") == 0) {
			/* printf("Sono entrato nel secondo if.\n"); */
			return;
		}
	}	
}

char* login(int cliente, char* username, char* password) {

	int ret, cfor;
	char* session_id;

	while(1) {
		pulisci();
		ret = ricevi_risposta(cliente);
		printf("%s\n", buffer);

		if(strcmp(buffer, "Il tuo ID di questa sessione e': ") == 0) {
			/* printf("Sono entrato nel primo if.\n"); */
			pulisci();
			ret = ricevi_risposta(cliente);
			session_id = (char*)malloc((strlen(buffer)+1)*sizeof(char));
			strcpy(session_id, buffer);
			printf("%s\n", session_id);
			return session_id;
		}

		if(strcmp(buffer, "Dati errati, per favore ritenta.") == 0) {
			/* printf("Sono entrato nel secondo if.\n"); */
			
			while(1) {
				printf("Inserire username: ");
				pulisci();
				fgets(buffer, BUFFER_SIZE, stdin);
				buffer[strlen(buffer)-1] = '\0';
				for(cfor = 0; cfor < BUFFER_SIZE; cfor++)
					if(buffer[cfor] == ' ') {
						cfor = -1;
						break;
				}
				if(strlen(buffer) < 30 && cfor != -1) {
					strcpy(username, buffer);
					break;
				}
				printf("Username non valido. Numero di caratteri superiori a 28 o trovato spazio. Non rientra nel conteggio dei tentativi. Riprova.\n");
			}
			ret = invia_risposta(cliente, username);

			while(1) {
				printf("Inserire password: ");
				pulisci();
				fgets(buffer, BUFFER_SIZE, stdin);
				buffer[strlen(buffer)-1] = '\0';
				for(cfor = 0; cfor < BUFFER_SIZE; cfor++)
					if(buffer[cfor] == ' ') {
						cfor = -1;
						break;
				}
				if(strlen(buffer) < 30 && cfor != -1) {
					strcpy(password, buffer);
					break;
				}
				printf("Password non valida. Numero di caratteri superiori a 28 o trovato spazio. Non rientra nel conteggio dei tentativi. Riprova.\n");
			}
			printf("\n");
			ret = invia_risposta(cliente, password);
		}

		if(strcmp(buffer, "Numero di tentativi massimi raggiunto. Sei stato bloccato per 30 minuti.") == 0 || strcmp(buffer, "Numero tentativi massimi raggiunto. Ip bloccato, impossibile accedere al momento.") == 0) {
			close(cliente);
			exit(1);
		}
	}
}

void invia_giocata(int cliente) {

	int ret, i, numeri;
	char* buf;
	pulisci();
	ret = ricevi_risposta(cliente);
	printf("%s\n", buffer);	

	if(strcmp(buffer, "Session id non valido, effettuare il login per favore.") == 0)
		return;
	
	/* Ruote */
	pulisci();
	printf("\nInserire le ruote in cui si desidera giocare. Scrivere 'tutte' nel caso si vogliano giocare tutte le ruote. Inserire una ruota alla volta. Non inserire spazi.\n");
	printf("Queste sono le ruote disponibili: ");
	for(i = 0; i < RUOTE; i++)
		printf("%s ", array_ruote[i]);
	printf("\n");
	inserisci_ruote();
	printf("%s\n", buffer);
	ret = invia_risposta(cliente, buffer);
	
	/* Numeri */
	pulisci();
	printf("\nInserire i numeri che si vuole giocare.(Essi vanno da 1 a 90) Inserire un numero alla volta. Non inserire spazi.\n");
	numeri = inserisci_numeri();
	printf("%s\n", buffer);
	ret = invia_risposta(cliente, buffer);

	/* Importi */
	pulisci();
	printf("\nInserire gli importi che si vuole giocare. Il primo e' l'estratto, il secondo l'ambo, il terzo il terno, il quarto la quaterna e il quinto la cinquina. Inserire un importo alla volta. Non inserire spazi. (Nel caso di importo con la virgola usare il punto, qualsiasi altro carattere sara' ignorato e verra' considerato solo il numero prima di tale carattere)\n");
	inserisci_importi(numeri);
	printf("%s\n", buffer);
	ret = invia_risposta(cliente, buffer);
	printf("\n");

	/* Notifica dell'avvenuta giocata */
	pulisci();
	ret = ricevi_risposta(cliente);
	printf("%s", buffer);

}

void vedi_giocate(int cliente) {

	int ret;

	while(1) {
		pulisci();
		ret = ricevi_risposta(cliente);
		printf("%s\n", buffer);

		if(strcmp(buffer, "Session id non valido, effettuare il login per favore.") == 0 || strcmp(buffer, "Giocate inviate con successo.") == 0 || strcmp(buffer, "Nessuna giocata con quel tipo.") == 0)
			return;
	}
}

void vedi_estrazione(int cliente) {

	int ret;
	
	while(1) {
		pulisci();
		ret = ricevi_risposta(cliente);
		printf("%s", buffer);

		if(strcmp(buffer, "Non ci sono estrazioni disponibili.") == 0 || strcmp(buffer, "Estrazioni richieste superiori rispetto a quelle effettivamente disponibili.") == 0 || strcmp(buffer, "Estrazioni inviate con successo.") == 0 || strcmp(buffer, "Session id non valido, effettuare il login per favore.") == 0) {
			printf("\n");
			return;
		}
	}		
}

void vedi_vincite(int cliente) {

	int ret;

	while(1) {
		pulisci();
		ret = ricevi_risposta(cliente);
		printf("%s", buffer);

		if(strcmp(buffer, "Session id non valido, effettuare il login per favore.") == 0 || strcmp(buffer, "Nessuna vincita trovata.") == 0 || strcmp(buffer, "Vincite inviate con successo.") == 0) {
			printf("\n");
			return;
		}
	}
}

int esci(int cliente) {	
/* restituisce -1 in caso di errore, 0 se ha avuto successo */

	int ret;

	pulisci();
	ret = ricevi_risposta(cliente);
	printf("%s\n", buffer);

	if(strcmp(buffer, "Session id non valido, effettuare il login per favore.") == 0 || strcmp(buffer, "Errore esterno. Logout non effettuato.") == 0)
		return -1;

	if(strcmp(buffer, "Logout effettuato con successo. Grazie e arrivederci.") == 0)
		return 0;
	
}

int main(int argc, char* argv[]){

	int porta;
	char* indirizzo_server;
	int ret, sd, len, cfor;
	bool comando_trovato;	/* controlla che è stato trovato un comando */
	bool ritorna; 			/* serve per ritornare dal login */
	char session_id[11];
	char username[30];
	char password[30];
	uint16_t lmsg;
	struct sockaddr_in srv_addr;

	session_id[0] = '\0';

	if(argc < 2 || argc > 3) {
		porta = 4243;
		indirizzo_server = "127.0.0.1";
	}
	else {
		inet_aton(argv[1], &srv_addr.sin_addr);
		indirizzo_server = inet_ntoa(srv_addr.sin_addr);
		porta = atoi(argv[2]);
	}

	/* Creazione socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd < 0) {
		perror("Errore in fase di creazione del socket\n");
		exit(-1);
	}
	else
		printf("Socket creato con successo.\n");

	/* Crezione indirizzo */
	memset(&srv_addr, 0, sizeof(srv_addr)); /* Pulizia */
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(porta);
	inet_pton(AF_INET, indirizzo_server, &srv_addr.sin_addr);

	/* Connessione */
	ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
	printf("%d\n", ret);
	if(ret < 0) {
		perror("Errore in fase di connessione: \n");
		exit(-1);
	}
	else
		printf("Client connesso.\n");

	printf("******************GIOCO DEL LOTTO******************\nSono disponibili i seguenti comandi:\n\n1) help --> mostra i dettagli di un comando specificato.\n2) signup --> viene richiesto di specificare username e password per creare un nuovo utente.\n3) login --> viene richiesto di specificare username e password per effettuare l'autenticazione.\n4) invia_giocata --> invia una giocata.\n5) vedi_giocate --> visualizza le giocate passate o attive in base ad un determinato parametro chiamato 'tipo'.\n6) vedi_estrazione --> visualizza le ultime n estrazioni, dove n e' un parametro che viene richiesto per determinare quante estrazioni si vuole visualizzare partendo dall'ultima estratta.\n7) vedi_vincite --> visualliza le vincite se sono state realizzate e un consuntivo per tipologia di giocata.\n8) esci --> effettua il logout.\n\n");

	comando_trovato = false;
	while(1) {

		if(!comando_trovato)
			printf("Inserisci un comando: ");
		else
			comando_trovato = false;

		/* Comando */
		cfor = 0;
		while(1) {
			pulisci();

			fgets(buffer, BUFFER_SIZE, stdin);
			buffer[strlen(buffer)-1] = '\0';
			for(cfor = 0; cfor < BUFFER_SIZE; cfor++)
				if(buffer[cfor] == ' ') {
					cfor = -1;
					break;
				}

			if(cfor == -1) 
				printf("Inserisci un comando: ");
			else
				break;
		}

		if(strcmp(buffer, "help") == 0) {
			/* Help */
			bool ok;
			int cfor;
			char* comandi[8] = {" ", "signup", "login", "invia_giocata", "vedi_giocate", "vedi_estrazione", "vedi_vincite", "esci"};
			char comando[30];
			printf("\nInserisci un comando per saperne i dettagli(se si vuole avere una breve descrizione di tutti i comandi premere la barra spaziatrice e invio): ");
			ok = false;
			comando_trovato = true;
			while(!ok) {
				/* while(getchar() != '\n'); pulisce il buffer dello stdin */
				fgets(comando, 30, stdin);
				comando[strlen(comando)-1] = '\0';
				for(cfor = 0; cfor < 8; cfor++) {
					if(strcmp(comando, comandi[cfor]) == 0) {
						ok = true;
						break;
					}
				}
				if(!ok)
					printf("Comando richiesto non valido. Per favore, inserisci un comando valido: ");
			}
			
			/* Chiama la help */
			help(cfor);
			printf("\nInserisci un comando: ");
			continue;

		}

		ret = invia_risposta(sd, buffer); /* Manda il comando */

		if(strcmp(buffer, "signup") == 0) {
			/* Signup */
			comando_trovato = true;			

			/* Chiama la signup */
			while(1) {
				printf("Inserire username: ");
				fgets(buffer, BUFFER_SIZE, stdin);
				buffer[strlen(buffer)-1] = '\0';
				for(cfor = 0; cfor < BUFFER_SIZE; cfor++)
					if(buffer[cfor] == ' ') {
						cfor = -1;
						break;
				}
				if(strlen(buffer) < 30 && cfor != -1) {
					strcpy(username, buffer); /* Controllare */
					break;
				}
				printf("Username non valido. Numero di caratteri superiori a 28 o trovato spazio. Riprova.\n");
			}
			ret = invia_risposta(sd, username);

			while(1) {
				printf("Inserire password: ");
				fgets(buffer, BUFFER_SIZE, stdin);
				buffer[strlen(buffer)-1] = '\0';
				for(cfor = 0; cfor < BUFFER_SIZE; cfor++)
					if(buffer[cfor] == ' ') {
						cfor = -1;
						break;
				}
				if(strlen(buffer) < 30 && cfor != -1) {
					strcpy(password, buffer);
					break;
				}
				printf("Password non valida. Numero di caratteri superiori a 28 o trovato spazio. Riprova.\n");
			}
			printf("\n");
			ret = invia_risposta(sd, password);
			signup(sd, username, password);
			pulisci();
			ret = ricevi_risposta(sd);
			printf("%s ", buffer);
		}
		
		if(strcmp(buffer, "login") == 0) {
			ritorna = false; /* permette di tornare indietro */
			comando_trovato = true;	
			while(1) {
				printf("Se non si possiede alcun account, scrivere 'ritorna' per tornare indietro, altrimenti premi invio: ");
				pulisci();
				fgets(buffer, BUFFER_SIZE, stdin);
				if(strcmp(buffer, "\n") != 0)
					buffer[strlen(buffer)-1] = '\0';

				invia_risposta(sd, buffer);
				if(strcmp(buffer, "ritorna") == 0) {
					ritorna = true;
					break;
				}
				if(strcmp(buffer, "\n") == 0) 
					break;
			}
			if(ritorna) {
				printf("Inserisci un comando: ");
				continue;
			}
		
			if(session_id[0] == '\0') {	/* controlla se si è già effettuato 										l'accesso */
				/* Login */	
				printf("*** Attenzione! Hai solo tre tentativi ***\n");
				/* Chiama la login */
				while(1) {
					printf("Inserire username: ");
					pulisci();
					fgets(buffer, BUFFER_SIZE, stdin);
					buffer[strlen(buffer)-1] = '\0';
					for(cfor = 0; cfor < BUFFER_SIZE; cfor++)
						if(buffer[cfor] == ' ') {
							cfor = -1;
							break;
					}
					if(strlen(buffer) < 30 && cfor != -1) {
						strcpy(username, buffer);
						break;
					}
					printf("Username non valido. Numero di caratteri superiori a 28 o trovato spazio. Non rientra nel conteggio dei tentativi. Riprova.\n");
				}
				ret = invia_risposta(sd, "ok"); /* Auntenticazione non 													effettuata */
				ret = invia_risposta(sd, username);

				while(1) {
					printf("Inserire password: ");
					pulisci();
					fgets(buffer, BUFFER_SIZE, stdin);
					buffer[strlen(buffer)-1] = '\0';
					for(cfor = 0; cfor < BUFFER_SIZE; cfor++)
						if(buffer[cfor] == ' ') {
							cfor = -1;
							break;
					}
					if(strlen(buffer) < 30 && cfor != -1) {
						strcpy(password, buffer);
						break;
					}
					printf("Password non valida. Numero di caratteri superiori a 28 o trovato spazio. Non rientra nel conteggio dei tentativi. Riprova.\n");
				}
				
				printf("\n");
				ret = invia_risposta(sd, password);
				strcpy(session_id, login(sd, username, password));
				pulisci();
				ret = ricevi_risposta(sd);
				printf("%s ", buffer);
			}
			else {
				comando_trovato = true;
				ret = invia_risposta(sd, "ko");
				printf("Errore: Autenticazione gia' effettuata!\n");
				printf("Inserisci un comando: ");
			}
		}

		if(strcmp(buffer, "invia_giocata") == 0) {
			/* Invia giocata */
			comando_trovato = true;			

			/* Chiama la invia giocata */
			ret = invia_risposta(sd, session_id);
			ret = invia_risposta(sd, username);
			invia_giocata(sd);
			pulisci();
			ret = ricevi_risposta(sd);
			printf("%s ", buffer);

		}

		if(strcmp(buffer, "vedi_giocate") == 0) {
			/* Vedi giocate */
			char tipo[2];
			printf("Inserisci il tipo di giocate che vuoi vedere. Se inserisci '0' vedrai le giocate relative a estrazioni gia' effettuate, altrimenti inserendo '1' vedrai le giocate relative a estrazioni non ancora effettuate. (Verra' considerato solo il primo numero inserito, il resto sara' ignorato)\n");
			comando_trovato = true;
			while(1) {
				printf("Tipo: ");
				scanf("%s", tipo);
				if(strcmp(tipo, "1") == 0 || strcmp(tipo, "0") == 0)
					break;
				else
					printf("Tipo non valido. Per favore, inserisci un tipo corretto.\n");
			}
			/* Chiama la vedi giocate */
			ret = invia_risposta(sd, session_id);
			ret = invia_risposta(sd, username);
			ret = invia_risposta(sd, tipo);
			vedi_giocate(sd);
			pulisci();
			ret = ricevi_risposta(sd);
			printf("%s ", buffer);

		}

		if(strcmp(buffer, "vedi_estrazione") == 0) {
			/* Vedi estrazione */
			int n;
			char ruota[11];
			char num[4];

			printf("Inserisci il numero n per richiedere le ultime ennesime estrazioni. (Sara' considerato solo il primo numero inserito, tutto il resto sara' ignorato)\n");
			comando_trovato = true;
			while(1) {
				printf("n: ");
				scanf("%d", &n);
				while(getchar() != '\n');
				if(n <= 0 || n >= 1000)
					printf("n non valido. Inserisci un numero maggiore di zero e che non sia uguale o superiore a mille, per favore.\n");
				else {
					sprintf(num, "%d", n);
					break;
				}
			}
			printf("Inserisci la ruota di cui vuoi vedere le estrazioni. Se vuoi vederle tutte non inserire nulla, premi la barra spaziatrice e invio.\nRuota: ");
			fgets(ruota, 11, stdin);
			
			/* Chiama la vedi estrazione */
			ret = invia_risposta(sd, session_id);
			/* printf("Ho inviato il session_id.\n"); */
			ret = invia_risposta(sd, num);
			/* printf("Ho inviato il num.\n"); */
			ret = invia_risposta(sd, ruota);
			/* printf("Ho inviato la ruota.\n"); */
			vedi_estrazione(sd);
			pulisci();
			ret = ricevi_risposta(sd);
			printf("%s ", buffer);
			
		}

		if(strcmp(buffer, "vedi_vincite") == 0) {
			/* Vedi_vincite */
			comando_trovato = true;
			
			/* Chiama la vedi_vincite */
			ret = invia_risposta(sd, username);
			/* printf("Ho inviato l'username.\n"); */
			ret = invia_risposta(sd, session_id);
			/* printf("Ho inviato il session_id.\n"); */
			vedi_vincite(sd);
			pulisci();
			ret = ricevi_risposta(sd);
			printf("%s ", buffer);

		}

		if(strcmp(buffer, "esci") == 0) {
			/* Esci */
			int logout;
			comando_trovato = true;		

			/* Chiama la esci */
			ret = invia_risposta(sd, username);
			/* printf("Ho inviato l'username.\n"); */
			ret = invia_risposta(sd, session_id);
			/* printf("Ho inviato il session_id.\n"); */
			logout = esci(sd);
			if(logout == 0)
				break;
			else {
				pulisci();
				ret = ricevi_risposta(sd);
				printf("%s ", buffer);
			}
				
		}		
	}
	close(sd);

return 0;
}
