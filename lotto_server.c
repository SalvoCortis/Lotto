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

#define BUFFER_SIZE 1024
#define RUOTE   11


char buffer[1024];
char* array_ruote[RUOTE] = {"Bari", "Cagliari", "Firenze", "Genova", "Milano", "Napoli", "Palermo", "Roma", "Torino", "Venezia", "Nazionale"};  /* contiene tutte le ruote del lotto */


void pulisci() {	/* pulisce il buffer */
	int i;
	for(i = 0; i < BUFFER_SIZE; i++)
		buffer[i] = '\0';
}

int invia_risposta(int cliente, char *msg) {	/* invia dim e msg */
   
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
		perror("Errore nella ricezione della risposta");
	
	len = ntohs(lmsg);

	ret = recv(cliente, (void*)buffer, len, 0);
	if(ret < 0)
		perror("Errore nella ricezione della risposta");

	return ret;		
}

char *time_stamp(){		/* ritorna il timestamp */

	char *timestamp = (char*)malloc(sizeof(char) * 16);
	time_t ltime;
	ltime=time(NULL);
	struct tm *tm;
	tm=localtime(&ltime);

	 sprintf(timestamp,"%04d%02d%02d%02d%02d%02d", tm->tm_year+1900, tm->tm_mon+1, 
    	tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec); 
	return timestamp;

}

void time_stamp_estrazione(char* timestamp){	/* timestamp formato 
												 estrazione */

	time_t ltime;
	ltime=time(NULL);
	struct tm *tm;
	tm=localtime(&ltime);

	 sprintf(timestamp,"Estrazione del %02d-%02d-%04d ore %02d:%02d", tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900, tm->tm_hour, tm->tm_min); 

}

int estrai_numero(char* stringa) { /* estrae ore minuti secondi */

	char* ris = (char*)malloc(7*sizeof(char));
	int x;
	strcpy(ris, stringa+8);
	x = atoi(ris);

	return x;
}

int estrai_ora(int numero) {	/* estrae l'ora da una stringa che ha 									chiamato l'estrai_numero(char*stringa) */

	numero = (numero/10000)%24;
	return numero;
}



char* randomly() {		/* genera un id alfanumerico */

	int x, len;
	char alfanumerico;
	char* ris;
	int i;

	srand(time(NULL));
	len = 10;
	ris = (char*)malloc((len+1)*sizeof(char));

	for(i = 0; i < 10; i++) {
		x = rand();
		if (x < 0)
			x = x * (-1);
		alfanumerico = "abcdefghijklmnopqrstuvwxyz0123456789"[x % 36];
		ris[i] = alfanumerico;
	}
	ris[10] = '\0';
	/* printf("Ris e' %s e la sua strlen e' %d", ris, strlen(ris)); */
	return ris;
}

int fatt(int x) {		/* calcola il fattoriale di un numero */

	int numero;

	if(x == 0)
		return 1;

	numero = x;

	while(x > 1) {
		numero = numero*(x-1);
		x--;
	}

	return numero;

}

int Cs(int n, int k) {		/* calcola le combinazioni semplici */

	return (fatt(n))/(fatt(k)*fatt((n-k)));

}

void uscita_forzata(int cliente) { /* effettua l'uscita forzata */

	close(cliente);
	printf("Client bloccato. Disconesso con uscita_forzata.\n");

}

void sblocca(const struct sockaddr_in* cl_addr) { /* sblocca un utente */

	int ret, len, i, j, cfor;
	char buf[BUFFER_SIZE];
	char indirizzo_ip[100];
	char* res;
	FILE* fd;

	sprintf(indirizzo_ip, "%s", inet_ntoa(cl_addr->sin_addr));
	len = (strlen(indirizzo_ip)+1)*sizeof(char);
	fd = fopen("./files/utenti/bloccati.txt", "r");
	
	i = 0;
	j = 0;
	while(1) {
		i++;
		res = fgets(buf, BUFFER_SIZE, fd);
		if(res == NULL) {
			fclose(fd);
			break;
		}
		
		buf[strlen(buf)-1] = '\0';
		if(strcmp(buf, indirizzo_ip) == 0) {
			fclose(fd);
			fd = fopen("./files/utenti/bloccati.txt", "r+");
			while(1) {
				j++;
				if(j == i) {
					for(cfor = 0; cfor < len-1; cfor++)
						ret = fprintf(fd, " ");	/* sovrascrive l'indirizzo 														IP con degli spazi  */
					break;
				}
				res = fgets(buf, BUFFER_SIZE, fd);
			}
			fclose(fd);
		}
		if(j != 0)
			break;
	}
}

int estrazione() {	/* effettua l'estrazione di un numero */

	int x;

	x = rand();
	if(x<0)
		x = x*(-1);
	x = x%90;
	if(x == 0)
		x = 90;

	return x;
}

void estrazione_lotto(char* timestamp) {

	int ret, i, x, j, k, spazio;
	int numero1, numero2, numero3, numero4;
	char time_estrazione[36];
	char* ruota;
	FILE* fd;
	srand(time(NULL));
	x = 0; numero1 = 0; numero2 = 0; numero3 = 0; numero4 = 0;

	time_stamp_estrazione(time_estrazione);
	strcpy(timestamp, time_estrazione);
	fd = fopen("./files/estrazioni/estrazioni.txt", "a+");
	ret = fprintf(fd, "%s\n", timestamp);
	for(i = 0; i < RUOTE; i++) {
		strcpy(buffer, array_ruote[i]);
		ret = fprintf(fd, "%s", buffer);
		for(j = 0; j < 5; j++) {
			x = estrazione(); 
			while(x == numero1 || x == numero2 || x == numero3 || x == numero4) {
				x = estrazione();
			} 
			switch(j) {
				case 0: numero1 = x; 
						break;
				case 1: numero2 = x; 
						break;
				case 2: numero3 = x; 
						break;
				case 3: numero4 = x; 
						break;
				default: 
						break;
			} 
			if(j == 0) {
				spazio = 9-strlen(array_ruote[i]);	/* calcola il numero 														di spazi da inserire */
				for(k = 0; k < spazio; k++)
					ret = fprintf(fd, " ");
			}
			else
				ret = fprintf(fd, "  ");
			
			if(x < 10)	/* numero ad una cifra, quindi mette uno spazio */
				ret = fprintf(fd, " ");

			sprintf(buffer, "%d", x);
			ret = fprintf(fd, "%s", buffer);
		 }
		ret = fprintf(fd, "\n");
	}
	ret = fprintf(fd, "\n");
	fclose(fd);
	
}

int ultima_riga(char* filename) {	/* ritorna l'indice dell'ultima riga 										di un file */

	int i;
	char* res;
	FILE* fd;

	fd = fopen(filename, "r");
	
	i = 0;
	while(1) {
		i++;
		res = fgets(buffer, BUFFER_SIZE, fd);
		if(res == NULL) {
			fclose(fd);
			return i-1;
		}
	}
}

void controlla_giocata(char* username, char** vett_ruote, int* vett_numeri, double* vett_importi, char* timestamp) {
	/* Controlla se una giocata è vincente */

	int ret, i, j, k, indovinato; /* i: ruote j: numeri_ruota k: n_giocati 									  */
	int indovinato2; /* nel caso in cui si siano indovinati numeri in 							ruote */
					/* diverse */
	int h; /* h: importi */
	int numeri_giocati, numero_ruote_giocate;
	int C;	/* combinazioni semplici */
	char* res;
	char file[250];
	char buf[BUFFER_SIZE];
	FILE* fd; /* punta al file estrazioni */
	FILE* fd2; /* punta al file username_vincite */

	fd = fopen("./files/estrazioni/estrazioni.txt", "r");
	strcpy(file, "./files/utenti/");
	strcat(file, username);
	strcat(file, "_vincite.txt");
	fd2 = fopen(file, "a+");
	
	numeri_giocati = 0;
	numero_ruote_giocate = 0;

	for(k = 0; k < 10; k++)
		if(vett_numeri[k])
			numeri_giocati++;

	for(i = 0; i < 11; i++)
		if(strcmp(vett_ruote[i], "\0") != 0)
			numero_ruote_giocate++;

	indovinato = 0;

	ret = fprintf(fd2, "%s", timestamp);

	while(1) {
		res = fgets(buf, BUFFER_SIZE, fd);
		if(res == NULL)
			break;

		buf[strlen(buf)-1] = '\0';
		if(strcmp(buf, timestamp) == 0) { /* Ha trovato l'estrazione */
			for(i = 0; i < RUOTE; i++) {
				indovinato = 0;
				res = fgets(buf, strlen(array_ruote[i])+1, fd);
				if(res == NULL)
					break;
				if(strcmp(vett_ruote[i], buf) == 0) { /* Trovata la ruota 														  */
					res = fgets(buf, 9-(strlen(array_ruote[i])-1), fd);
					for(j = 0; j < 5; j++) {
						res = fgets(buf, 3, fd);
						for(k = 0; k < 10; k++) {
							if(vett_numeri[k] == atoi(buf)) { /* Trovato 																il numero */
								if(indovinato) {
									indovinato++;
									ret = fprintf(fd2, "%d ", vett_numeri[k]);
									break;
								}
								else{	/* primo numero trovato */
									indovinato++;
									ret = fprintf(fd2, "\n%s  ", array_ruote[i]);
									ret = fprintf(fd2, "%d ", vett_numeri[k]);
									break;
								}
							}
						}
						res = fgets(buf, 2, fd);
						if(strcmp(buf, "\n") == 0) { /* fine riga */
							if(indovinato) {
								ret = fprintf(fd2, "  >>  ");
								for(h = 0; h < indovinato; h++) {
									if(vett_importi[h] != 0) {
										switch(h) {
											case 0: ret = fprintf(fd2, "Estratto ");	
													ret = fprintf(fd2, "%.2f  ", (vett_importi[h]*11.23)/(numeri_giocati*numero_ruote_giocate));
													break;

											case 1: ret = fprintf(fd2, "Ambo ");
													C = Cs(numeri_giocati, 2);
													ret = fprintf(fd2, "%.2f  ", (vett_importi[h]*250)/(C*numero_ruote_giocate));
													break;

											case 2: ret = fprintf(fd2, "Terno ");
													C = Cs(numeri_giocati, 3);
													ret = fprintf(fd2, "%.2f  ", (vett_importi[h]*4500)/(C*numero_ruote_giocate));
													break;

											case 3: ret = fprintf(fd2, "Quaterna ");
													C = Cs(numeri_giocati, 4);
													ret = fprintf(fd2, "%.2f  ", (vett_importi[h]*120000)/(C*numero_ruote_giocate));
													break;

											case 4: ret = fprintf(fd2, "Cinquina ");
													C = Cs(numeri_giocati, 5);
													ret = fprintf(fd2, "%.2f  ", (vett_importi[h]*6000000)/(C*numero_ruote_giocate));
													break;

											default: break;
	
										}
									}
								}
							}
							break;
						}
						res = fgets(buf, 2, fd);
					}	
				}
				else {
					res = fgets(buf, BUFFER_SIZE, fd);
				}
			}

		}
			
	}
	fclose(fd);
	ret = fprintf(fd2, "\n*********************************************************************************\n"); /* separatore tra le varie vincite */
	fclose(fd2);
}

void inserisci_vincite(char* username, char* timestamp) {
	/* Se ha trovato una giocata vincente la inserisce nel file 		   		username_vincite. Chiama la controlla_giocata(....) */

	int ret, i, j, k; /* i righe, j posizione_numeri, k posizione_vettori 						  */
	int cfor; /* contatore dei for */
	bool _i; /* segnala che ha trovato -i */
	bool _n; /* segnala che ha trovato -n */
	bool _r; /* segnala che ha trovato -r */
	char num[3];
	char importo[100];
	char* res1;
	char* res2;
	char file[250];
	char filename[250];
	char buf[BUFFER_SIZE];
	char b[2];
	char ruota[10];
	char* vett_ruote[RUOTE] = {"\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0"};	/* contiene le ruote giocate */
	int vett_numeri[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  /* contiene i 															numeri giocati */
	double vett_importi[5] = {0, 0, 0, 0, 0};	/* contiene gli importi 												giocati */
	FILE* fd;	/* punta al file dell'username */
	
	strcpy(filename, "./files/utenti/");
	strcat(filename, username);
	strcat(filename, ".txt");
	fd = fopen(filename, "r");
	/* printf("%s\n", filename); */
	
	i = 0;
	while(1) {
		_i = false;
		_n = false;
		_r = false;
		k = 0;
		j = 0;
		for(cfor = 0; cfor < 3; cfor++)
			num[cfor] = '\0';
		for(cfor = 0; cfor < 100; cfor++)
			importo[cfor] = '\0';

		i++;
		res1 = fgets(b, 2, fd);
		if(i < 4)	
			res2 = fgets(buf, BUFFER_SIZE, fd);
		if(res1 == NULL || res2 == NULL)
			break;
		
		if(i < 4)	/* prosegue fino alla prima giocata */
			continue;

		if(b[0] == '0') {
			res2 = fgets(buf, BUFFER_SIZE, fd);
			continue;
		}


		while(1) { /* j:1, k:0 */
			
			j++;
			res1 = fgets(b, 2, fd);
			if(b[0] == '\n' || res1 == NULL) {
				break;
			}
			if((b[0] < 'A' || b[0] > 'Z') && b[0] != 't') { /* Non trova 																una ruota */
				if(b[0] >= '0' && b[0] <= '9') { /* Non trova un numero */
					if(!_i) {
						if(j == 1) {
							num[0] = b[0];
						}
						else {
							num[1] = b[0];
							j = 0;
						}
					}
					else {
						importo[j-1] = b[0];
					}
				}
				else {	/* verifica se ha trovato -r o -n o -i */

					if(b[0] == 'r')	/* Segna l'inizio delle ruote giocate 										*/
						_r = true;

					if(b[0] == 'n' && !_r)	/* Segna l'inizio dei numeri */
						_n = true;

					if(b[0] == '-' && _r)
						_r = false;

					if(b[0] == 'i' && _n && !_i) {	/* Inizio degli 													importi */
						k = 0;
						_i = true;
					}
						
					if(b[0] == '.' && _i) {	/* Ha trovato la virgola */
						importo[j-1] = b[0];
						continue;
					}

					if(b[0] == ' ' && !_i && num[0] != '\0') {	/* Ha 						trovato uno spazio, quindi inserisce il numero giocato 																	*/
						vett_numeri[k] = atoi(num);
						k++;
						for(cfor = 0; cfor < 3; cfor++)
							num[cfor] = '\0'; /* Svuota num per contenere 													un altro numero */
					}

					if(_i && b[0] == ' ' && importo[0] != '\0') { /* Ha trovato uno spazio quindi inserisce l'importo giocato */
						sscanf(importo, "%lf", &vett_importi[k]);
						k++;
						for(cfor = 0; cfor < 100; cfor++)
							importo[cfor] = '\0'; /* Svuota importo per 												contenere un altro importo 													*/
					}
					j = 0;
					continue;
				}
			}
			else {
				if(!_i) { /* Non entra se trova la 't' di estratto ad esempio ma entra solo nel caso in cui si sia giocato 'tutte' nelle ruote */
					switch(b[0]) {	/* un caso per ogni ruota */
						case 'B':
								vett_ruote[0] = (char*)malloc((strlen(array_ruote[0])+1)*sizeof(char));
								strcpy(vett_ruote[0], array_ruote[0]);
								break;

						case 'C':
								vett_ruote[1] = (char*)malloc((strlen(array_ruote[1])+1)*sizeof(char));
								strcpy(vett_ruote[1], array_ruote[1]);
								break;

						case 'F':
								vett_ruote[2] = (char*)malloc((strlen(array_ruote[2])+1)*sizeof(char));
								strcpy(vett_ruote[2], array_ruote[2]);
								break;

						case 'G':
								vett_ruote[3] = (char*)malloc((strlen(array_ruote[3])+1)*sizeof(char));
								strcpy(vett_ruote[3], array_ruote[3]);
								break;

						case 'M':
								vett_ruote[4] = (char*)malloc((strlen(array_ruote[4])+1)*sizeof(char));
								strcpy(vett_ruote[4], array_ruote[4]);
								break;

						case 'N':
								res1 = fgets(b, 3, fd);
								if(strcmp(b, "ap") == 0) {
									vett_ruote[5] = (char*)malloc((strlen(array_ruote[5])+1)*sizeof(char));
									strcpy(vett_ruote[5], array_ruote[5]);
								}
								else {
									vett_ruote[10] = (char*)malloc((strlen(array_ruote[10])+1)*sizeof(char));
									strcpy(vett_ruote[10], array_ruote[10]);
								}
								break;

						case 'P':
								vett_ruote[6] = (char*)malloc((strlen(array_ruote[6])+1)*sizeof(char));
								strcpy(vett_ruote[6], array_ruote[6]);
								break;

						case 'R':
								vett_ruote[7] = (char*)malloc((strlen(array_ruote[7])+1)*sizeof(char));
								strcpy(vett_ruote[7], array_ruote[7]);
								break;

						case 'T':
								vett_ruote[8] = (char*)malloc((strlen(array_ruote[8])+1)*sizeof(char));
								strcpy(vett_ruote[8], array_ruote[8]);
								break;

						case 'V':
								vett_ruote[9] = (char*)malloc((strlen(array_ruote[9])+1)*sizeof(char));
								strcpy(vett_ruote[9], array_ruote[9]);
								break;

						case 't':
								for(cfor = 0; cfor < RUOTE; cfor++) {
									vett_ruote[cfor] = (char*)malloc((strlen(array_ruote[cfor])+1)*sizeof(char));
									strcpy(vett_ruote[cfor], array_ruote[cfor]);
								}
								break;
					
						default: 
								break;
					}	
				}
			}
		}
		controlla_giocata(username, vett_ruote, vett_numeri, vett_importi, timestamp);	/* Controlla se la giocata è vincente */

		/* svuota i vari vettori per controllare un'altra giocata */
		for(cfor = 0; cfor < RUOTE; cfor++)
			vett_ruote[cfor] = "\0";

		for(cfor = 0; cfor < 10; cfor++)
			vett_numeri[cfor] = 0;
	
		for(cfor = 0; cfor < 5; cfor++)
			vett_importi[cfor] = 0;

	}
	fclose(fd);
}

void modifica_estrazioni(char* timestamp) {
/* Mette a 0 il tipo di una giocata e chiama la inserisci_vincite(...) */

	int ret, i, k;
	char* res1;
	char filename[250];
	char buf[2];
	FILE* fuser;
	FILE* fd;

	fuser = fopen("./files/utenti/usernames.txt", "r");
	if(fuser == NULL) {
		printf("Nessun username.\n");
		return;
	}

	while(1) {
		res1 = fgets(buffer, BUFFER_SIZE, fuser);
		if(res1 == NULL) {
			fclose(fuser);
			break;
		}
		buffer[strlen(buffer)-1] = '\0';
		strcpy(filename, "./files/utenti/");
		strcat(filename, buffer);
		strcat(filename, ".txt");
		/* printf("%s\n", filename); */
		inserisci_vincite(buffer, timestamp);
		k = ultima_riga(filename);
		fd = fopen(filename, "r+"); /* apre un file di testo in r/w */

		i = 0;
		while(1) {
			
			i++;
			if(i == k) {
				fclose(fd);
				break;
			}

			res1 = fgets(buffer, BUFFER_SIZE, fd);

			if(i < 3)
				continue;

			ret = fprintf(fd, "0");
		}
	}	
}

int tipo_vincita(bool* tipologia) {	/* Ritorna la tipologia di una vincita(estratto, ambo, terno, quaterna, cinquina) */

	int i;
	for(i = 0; i < 5; i++)
		if(tipologia[i])
			return i;

}

void consuntivo(int cliente, char* username) { /* Invia il consuntivo */

	int ret, cfor, i, k;
	char* messaggio;
	char* res;
	char filename[250];
	char buf[2];
	char vincita[100];
	double importi[5]; /*0:estratto 1:ambo 2:terno 3:quaterna 4:cinquina*/
	double importo;
	bool tipologia[5]; /*0:estratto 1:ambo 2:terno 3:quaterna 4:cinquina*/
	FILE* fd;

	strcpy(filename, "./files/utenti/");
	strcat(filename, username);
	strcat(filename, "_vincite.txt");
	/* printf("%s\n", filename); */
	fd = fopen(filename, "r");

	for(cfor = 0; cfor < 5; cfor++)
		importi[cfor] = 0;

	for(cfor = 0; cfor < 5; cfor++)
		tipologia[cfor] = false;

	for(cfor = 0; cfor < 100; cfor++)
		vincita[cfor] = '\0';

	i = 0;
	k = 0;
	while(1) {
		i++;
		res = fgets(buf, 2, fd);
		if(res == NULL) {
			fclose(fd);
			break;
		}

		if(buf[0] == '*') {
			res = fgets(buffer, BUFFER_SIZE, fd);
			i = 0;
			continue;
		}

		switch(buf[0]) { /* un caso per ogni tipologia */


			case 'E': 		/* controllo se è l'estratto */
					res = fgets(buffer, 6, fd); /* mi sposto sulla sesta 
												 lettera */
					buffer[5] = '\0';
					if(buffer[4] == 'z') {
						res = fgets(buffer, BUFFER_SIZE, fd);
						i = 0;
						continue;
					}
					tipologia[0] = true;	/* estratto */
					res = fgets(buffer, 4, fd); /* mi sposto fino alla
												cifra */
					i = 0;
					break;

			case 'A':
					tipologia[1] = true;	/* ambo */
					res = fgets(buffer, 5, fd); /* mi sposto fino alla
												cifra */
					i = 0;
					break;

		
			case 'T':		/* controllo se è il terno */
					res = fgets(buf, 2, fd); /* mi sposto sulla seconda
											lettera */
					if(buf[0] == 'o') {
						i = 0;
						continue;
					}

					tipologia[2] = true;	/* terno */
					res = fgets(buffer, 5, fd); /* mi sposto fino alla
												cifra */
					i = 0;
					break;

			case 'Q':
					tipologia[3] = true;	/* quaterna */
					res = fgets(buffer, 9, fd); /* mi sposto fino alla
												cifra */
					i = 0;
					break;	
		
			case 'C':		/* controllo se è la cinquina */
					res = fgets(buf, 2, fd); /* mi sposto sulla seconda
											lettera */
					if(buf[0] == 'a') {
						i = 0;
						continue;
					}
		
					tipologia[4] = true;	/* cinquina */
					res = fgets(buffer, 8, fd), /* mi sposto fino alla
												cifra */
					i = 0;
					break;

			default:
					break;

		}
		
		if(((buf[0] >= '0' && buf[0] <= '9') || buf[0] == '.') && (tipologia[0] || tipologia[1] || tipologia[2] || tipologia[3] || tipologia[4])) {

			vincita[i-1] = buf[0]; /* prende ogni cifra e la virgola di un 										determinato importo vinto */
			
		}
		else {
			i = 0;
		}
		
		if((buf[0] == ' ' || buf[0] == '\n')  && vincita[0] != '\0') {
			/* Ha trovato lo spazio o è finita la riga */
			sscanf(vincita, "%lf", &importo);
			k = tipo_vincita(tipologia);
			importi[k] = importi[k] + importo; /* Somma tutte le vincite 													di quella tipologia */
			/* reset vincita */
			for(cfor = 0; cfor < 100; cfor++)
				vincita[cfor] = '\0';
			/* reset tipologia */
			for(cfor = 0; cfor < 5; cfor++)
				tipologia[cfor] = false;
		}
		
	}

	for(cfor = 0; cfor < 5; cfor++) {
		switch(cfor) {
			case 0: messaggio = "Vincite su ESTRATTO:"; break;
			case 1: messaggio = "Vincite su AMBO:"; break;
			case 2: messaggio = "Vincite su TERNO:"; break;
			case 3: messaggio = "Vincite su QUATERNA:"; break;
			case 4: messaggio = "Vincite su CINQUINA:"; break;
			default: break;
		}			
		/* Invia il consuntivo */
		ret = invia_risposta(cliente, messaggio);
		ret = invia_risposta(cliente, " ");
		sprintf(buffer, "%.2f", importi[cfor]);
		ret = invia_risposta(cliente, buffer);
		ret = invia_risposta(cliente, "\n");
	}

}

void signup(int cliente, char* username, char* password) {

	/* printf("Sono dentro la signup.\n"); */
	char* messaggio;
	int ret;
	char filename[250];
	char fileuser[250];
	char* user;
	FILE* fd;
	FILE* fuser;

	strcpy(fileuser, "./files/utenti/usernames.txt");
	fuser = fopen(fileuser, "a+");
	while(1) {
		strcpy(filename, "./files/utenti/");
		strcat(filename, username);
		strcat(filename, ".txt");
		fd = fopen(filename, "a+");
		if(fd == NULL) {
			printf("Errore nella creazione del file.\n");
			messaggio = "Errore esterno registrazione non riuscita.\n";
			ret = invia_risposta(cliente, messaggio);
			fclose(fuser);
			return;
		}
		user = (char*)malloc((strlen(username)+1)*sizeof(char));
		ret = fscanf(fd, "%s", &user);
		if(ret > 0) {
			messaggio = "Utente gia' esistente. Per favore, riprova.";
			ret = invia_risposta(cliente, messaggio);
			messaggio = "Inserisci username: ";
			ret = invia_risposta(cliente, messaggio);
			pulisci();
			ret = ricevi_risposta(cliente);
	
			username = (char*)malloc((strlen(buffer)+1)*sizeof(char));
			strcpy(username, buffer);
			fclose(fd);
		}
		else {
			ret = fprintf(fd, "%s\n", username);
			ret = fprintf(fuser, "%s\n", username);
			break;
		}
	}
	
	ret = fprintf(fd, "%s", password);
	fclose(fuser);
	fclose(fd);
	/*	printf("%d  %d\n", strlen(username), strlen(password)); */
	printf("Registrazione eseguita.\n");
	messaggio = "Registrazione eseguita con successo";
	ret = invia_risposta(cliente, messaggio);
	messaggio = "Inserisci un comando:";
	ret = invia_risposta(cliente, messaggio);

}	


void login(int cliente, char* username, char* password, const struct sockaddr_in* cl_addr) {

	char* messaggio;
	int ret, i, j;
	char* session_id;
	int tentativi;
	FILE* fd;
	char filename[250];
	char* buf;
	char buf2[30];
	char* user;
	char* pass;
	char* res;
	char* ip = (char*)malloc(sizeof(char) * 16);
	session_id = (char*)malloc(11*sizeof(char));
	
	tentativi = 2;
	i = 0;
	j = 0;
	
	/* Verifica se l'indirizzo IP è bloccato */
	fd = fopen("./files/utenti/bloccati.txt", "r");
	if(fd == NULL)
		printf("File inesistente. Nessun utente bloccato.\n");
	else {
		sprintf(ip, "%s", inet_ntoa(cl_addr->sin_addr));
		buf = (char*)malloc((strlen(ip)+1)*sizeof(char));
		while(1) {
			i++;
			if((i%2) == 0)	/* salta le righe pari e si posiziona solo sui 								timestamp */
				continue;
			res = fgets(buf, (strlen(ip)+1)*sizeof(char), fd);
			if(res == NULL) {
				fclose(fd);
				break;
			}
			if(strcmp(buf, ip) == 0) {
				i++;
				buf = (char*)malloc(16*sizeof(char));
				while(1) {
					j++;
					res = fgets(buf, 16*sizeof(char), fd);
					if(j == i) {	/* Ha trovato il timestamp */
						int now, before, ora_n, ora_b;
						now = estrai_numero(time_stamp()); /* Prende 6 																cifre */
						before = estrai_numero(buf); /* Prende 6 cifre */
						ora_n = estrai_ora(now);
						ora_b = estrai_ora(before);
						if(ora_n == ora_b) { /* ore */
							if(now-before<3000) { /* Nella stessa ora */
							messaggio = "Numero tentativi massimi raggiunto. Ip bloccato, impossibile accedere al momento.";
							ret = invia_risposta(cliente, messaggio);
							fclose(fd);
							uscita_forzata(cliente);
							return;
							}
							else {
								sblocca(cl_addr);
								fclose(fd);
								break;
							}
						}
						else {
							if(now-before<7000) { /* In ore differenti */
								messaggio = "Numero tentativi massimi raggiunto. Ip bloccato, impossibile accedere al momento.";
								ret = invia_risposta(cliente, messaggio);
								fclose(fd);
								uscita_forzata(cliente);
								return;
							}
							else {
								sblocca(cl_addr);
								fclose(fd);
								break;
							}
						}
					}
				} /* secondo while */
				if(j == i)
					break;
			} 
		} /* primo while */
	}	/* else iniziale */

	while(1) {
		strcpy(filename,"./files/utenti/");
		strcat(filename, username);
		strcat(filename, ".txt");
		fd = fopen(filename, "r");
		if(fd == NULL) {	/* Username errato al primo tentativo */
			if(tentativi) {
				tentativi--;
				printf("Errore nella lettura del file.\n");
				messaggio = "Dati errati, per favore ritenta.";
				ret = invia_risposta(cliente, messaggio);
				pulisci();
				ret = ricevi_risposta(cliente);
				username = (char*)malloc((strlen(buffer)+1)*sizeof(char));
				strcpy(username, buffer);
				pulisci();
				ret = ricevi_risposta(cliente);
				password = (char*)malloc((strlen(buffer)+1)*sizeof(char));
				strcpy(password, buffer);
			}
			else {
				messaggio = "Numero di tentativi massimi raggiunto. Sei stato bloccato per 30 minuti.";
				ret = invia_risposta(cliente, messaggio);
				uscita_forzata(cliente);
				fd = fopen("./files/utenti/bloccati.txt", "a+");
				ret = fprintf(fd, "%s\n", inet_ntoa(cl_addr->sin_addr));  	/* essendo stato inserito tra i bloccati, automaticamente l'indirizzo ip risulta bloccato */
				printf("Ho scritto in bloccati.\n");
				printf("%s\n", inet_ntoa(cl_addr->sin_addr));
				ret = fprintf(fd, "%s\n", time_stamp());
				fclose(fd);
				/* printf("Sono arrivato alla fine.\n"); */
				return;
			}
		}
		else
			break;
	}
	i = 0;
	while(1) {
		char* b;
		i++;

		if(i > 2)
			break;

		res = fgets(buf2, 30, fd);
		/* printf("%s", buf); */
		if(res == NULL)
			printf("Non ho letto nulla.\n"); 
		
		if(i == 1) {
			buf2[strlen(buf2)-1] = '\0';
			/* printf("La lunghezza di buf2 e':%d\n", strlen(buf2)); */
			user = (char*)malloc((strlen(buf2)+1)*sizeof(char));
			strcpy(user, buf2);
			/* printf("%s\n", user); */
		}

		if(i == 2) {
			if(buf2[strlen(buf2)-1] == '\n')
				buf2[strlen(buf2)-1] = '\0';
			/* printf("La lunghezza di buf2 e'%d\n", strlen(buf2)); */
			/* printf("Sono entrato in if == 2"); */
			pass = (char*)malloc((strlen(buf2)+1)*sizeof(char));
			strcpy(pass, buf2);
			/* printf("%s", pass); */
		}
		/* printf("Sono arrivato qui\n"); */
	}

	/* printf("\n%d  %d\n%d  %d\n", strlen(user), strlen(username), strlen(pass), strlen(password)); */

	if((strcmp(user, username) == 0) && (strcmp(pass, password)) == 0) {
		/* Username e password corretti */
		res = fgets(buf2, 30, fd);
		if(res == NULL) {	/* primo login */
			fclose(fd);
			fd = fopen(filename, "a");
			strcpy(session_id, randomly());
			ret = fprintf(fd, "\n%s", session_id);
		}
		else {	/* Ci sono stati altri login in precedenza */
			fclose(fd);
			fd = fopen(filename, "r+");
			res = fgets(buf2, 30, fd);
			res = fgets(buf2, 30, fd);
			strcpy(session_id, randomly());
			/* printf("Sono arrivato qui, session_id: %s\n", session_id); 				*/
			ret = fprintf(fd, "%s", session_id);
		}
		fclose(fd);
		messaggio = "Il tuo ID di questa sessione e': ";
		ret = invia_risposta(cliente, messaggio);
		ret = invia_risposta(cliente, session_id);
		messaggio = "Inserisci un comando:";
		ret = invia_risposta(cliente, messaggio);
	}
	else {
		while((strcmp(user, username) != 0) || (strcmp(pass, password)) != 0) {	/* Password errata al primo tentativo */
			if(tentativi) {
				tentativi--;
				messaggio = "Dati errati, per favore ritenta.";
				ret = invia_risposta(cliente, messaggio);
				pulisci();
				ret = ricevi_risposta(cliente);
				username = (char*)malloc((strlen(buffer)+1)*sizeof(char));
				strcpy(username, buffer);
				pulisci();
				ret = ricevi_risposta(cliente);
				password = (char*)malloc((strlen(buffer)+1)*sizeof(char));
				strcpy(password, buffer);
			}
			else {
				messaggio = "Numero di tentativi massimi raggiunto. Sei stato bloccato per 30 minuti.";
				ret = invia_risposta(cliente, messaggio);
				fclose(fd);
				uscita_forzata(cliente);
				fd = fopen("./files/utenti/bloccati.txt", "a+");
				ret = fprintf(fd, "%s\n", inet_ntoa(cl_addr->sin_addr)); 	/* essendo stato inserito tra i bloccati, automaticamente l'indirizzo ip risulta bloccato */
				printf("Ho scritto in bloccati.\n");
				printf("%s\n", inet_ntoa(cl_addr->sin_addr));
				ret = fprintf(fd, "%s\n", time_stamp());
				fclose(fd);
				printf("Sono arrivato alla fine.\n");
				break;
			}
		}
		/* Username e password corretti */
		res = fgets(buf2, 30, fd);
		if(res == NULL) {
			fclose(fd);
			fd = fopen(filename, "a");
			strcpy(session_id, randomly());
			ret = fprintf(fd, "\n%s", session_id);
		}
		else {
			fclose(fd);
			fd = fopen(filename, "r+");
			res = fgets(buf2, 30, fd);
			res = fgets(buf2, 30, fd);
			strcpy(session_id, randomly());
			ret = fprintf(fd, "%s", session_id);
		}
		fclose(fd);
		messaggio = "Il tuo ID di questa sessione e': ";
		ret = invia_risposta(cliente, messaggio);
		ret = invia_risposta(cliente, session_id);
		messaggio = "Inserisci un comando:";
		ret = invia_risposta(cliente, messaggio);
	}		
}


void invia_giocata(int cliente, char* username, char* session_id) {

	char* messaggio;
	int ret;
	FILE* fd;
	char filename[250];

	if(session_id[0] == '\0'){	/* Controlla se l'utente ha un session_id 									valido */
		printf("Session id non valido.\n");
		messaggio = "Session id non valido, effettuare il login per favore.";
		ret = invia_risposta(cliente, messaggio);
		messaggio = "Inserisci un comando:";
		ret = invia_risposta(cliente, messaggio);
		return;
	}
		

	strcpy(filename,"./files/utenti/");
	strcat(filename, username);
	strcat(filename, ".txt");

	messaggio = "Inserisci giocata.";
	ret = invia_risposta(cliente, messaggio);
	fd = fopen(filename, "a");
	ret = fprintf(fd, "\n1 -r ");/* inserisce un separatore per le ruote */
	pulisci();
	ret = ricevi_risposta(cliente);
	/* printf("%s\n", buffer); */
	ret = fprintf(fd, "%s ", buffer);
	ret = fprintf(fd, "-n ");	/* inserisce un separatore per i numeri */
	pulisci();
	ret = ricevi_risposta(cliente);
	ret = fprintf(fd, "%s ", buffer);
	ret = fprintf(fd, "-i "); /* inserisce un separatore per gli importi */
	pulisci();
	ret = ricevi_risposta(cliente);
	ret = fprintf(fd, "%s", buffer);
	fclose(fd);

	messaggio = "Giocata effettuata con successo.";
	ret = invia_risposta(cliente, messaggio);
	messaggio = "Inserisci un comando:";
	ret = invia_risposta(cliente, messaggio);
	
}


void vedi_giocate(int cliente, char* username, char* session_id, char* tipo) {

	int ret, i;
	char* messaggio;
	char* res1;
	char* res2;
	FILE* fd;
	char filename[250];
	char buf[2];
	bool nessuna_giocata = true; /* diversifica la stampa nel caso non ci 									siano giocate con quel tipo */

	if(session_id[0] == '\0'){ /* Controllo sul session_id */
		printf("Session id non valido.\n");
		messaggio = "Session id non valido, effettuare il login per favore.";
		ret = invia_risposta(cliente, messaggio);
		messaggio = "Inserisci un comando:";
		ret = invia_risposta(cliente, messaggio);
		return;
	}

	strcpy(filename,"./files/utenti/");
	strcat(filename, username);
	strcat(filename, ".txt");
	fd = fopen(filename, "r");

	i = 0;
	while(1) {
		i++;

		res1 = fgets(buf, 2, fd);
		res2 = fgets(buffer, BUFFER_SIZE, fd);
		if(res1 == NULL || res2 == NULL) {
			fclose(fd);
			break;
		}

		if(i < 4)	/* si porta sulla prima giocata */
			continue;
		if(strcmp(buf, tipo) == 0) {
			ret = invia_risposta(cliente, buffer);
			nessuna_giocata = false;
		}
	}

	if(nessuna_giocata)
		messaggio = "Nessuna giocata con quel tipo.";
	else
		messaggio = "Giocate inviate con successo.";
	ret = invia_risposta(cliente, messaggio);
	messaggio = "Inserisci un comando:";
	ret = invia_risposta(cliente, messaggio);

}

void vedi_estrazione(int cliente, char* session_id, char* n, char* ruota) {

	int ret, num, i, j, k, spazio;
	char* messaggio;
	char buf[10];
	char* res1;
	char* res2;
	FILE* fd;

	if(session_id[0] == '\0'){	/* Controllo sul session_id */
		printf("Session id non valido.\n");
		messaggio = "Session id non valido, effettuare il login per favore.";
		ret = invia_risposta(cliente, messaggio);
		messaggio = "Inserisci un comando:";
		ret = invia_risposta(cliente, messaggio);
		return;
	}



	fd = fopen("./files/estrazioni/estrazioni.txt", "r");
	if(fd == NULL) {
		messaggio = "Non ci sono estrazioni disponibili.";
		ret = invia_risposta(cliente, messaggio);
		printf("Errore: nessuna estrazione.\n");
		messaggio = "Inserisci un comando:";
		ret = invia_risposta(cliente, messaggio);
		return;
	}

	k = 0;
	j = 0;
	i = 0;
	while(1) {
		i++;
		res1 = fgets(buffer, BUFFER_SIZE, fd);
		if(res1 == NULL) {
			fclose(fd);
			break;
		}
	}

	fd = fopen("./files/estrazioni/estrazioni.txt", "r");
	num = atoi(n); /* prende la n indicata dall'utente */
	j = (i-1)-((11*num)+((num*2)-1)); /* punta alla riga da dove bisogna 											stampare le estrazioni */

	if(j <= 0) {
		messaggio = "Estrazioni richieste superiori rispetto a quelle effettivamente disponibili.";
		ret = invia_risposta(cliente, messaggio);
		printf("Errore, richieste troppe estrazioni.\n");
		messaggio = "Inserisci un comando:";
		ret = invia_risposta(cliente, messaggio);
		return;
	}

	if(ruota[0] != ' ')
		spazio = 9-(strlen(ruota)-1);

	pulisci();
	i = 0;
	while(1) {
		i++;
		res2 = fgets(buf, 10, fd);
		if(res2 != NULL && strcmp(buf, "\n") == 0)
			continue;
		res1 = fgets(buffer, BUFFER_SIZE, fd);
		if(res1 == NULL || res2 == NULL) {
			fclose(fd);
			break;
		}

		if(i < j)
			continue;

		if(ruota[0] == ' ') {	/* invia tutte le ruote */
			ret = invia_risposta(cliente, buf);
			ret = invia_risposta(cliente, buffer);
			if(strcmp(buf, "Nazionale") == 0)
				ret = invia_risposta(cliente, "\n");
		}
		else {
			if(i != j+(13*k)) {
				buf[9-spazio] = '\0';
				ruota[9-spazio] = '\0';
			}
			if(strcmp(buf, ruota) == 0 || i == j+(13*k)) {
				if(i == j+(13*k)) {	/* invia la ruota o il timestamp 										dell'estrazione */
					ret = invia_risposta(cliente, buf);
					ret = invia_risposta(cliente, buffer);
					k++;
				}
				else {
					ret = invia_risposta(cliente, buf);
					ret = invia_risposta(cliente, " ");
					ret = invia_risposta(cliente, buffer);
					ret = invia_risposta(cliente, "\n");
				}
			}
		}	
	}

	messaggio = "Estrazioni inviate con successo.";
	ret = invia_risposta(cliente, messaggio);
	messaggio = "Inserisci un comando:";
	ret = invia_risposta(cliente, messaggio);

}

void vedi_vincite(int cliente, char* username, char* session_id) {
/* Legge il file username_vincite e lo invia */

	int ret;
	char* messaggio;
	char* res;		
	char filename[250];
	FILE* fd;

	if(session_id[0] == '\0'){	/* Controllo sul session_id */
		printf("Session id non valido.\n");
		messaggio = "Session id non valido, effettuare il login per favore.";
		ret = invia_risposta(cliente, messaggio);
		messaggio = "Inserisci un comando:";
		ret = invia_risposta(cliente, messaggio);
		return;
	}

	strcpy(filename, "./files/utenti/");
	strcat(filename, username);
	strcat(filename, "_vincite.txt");
	fd = fopen(filename, "r");
	if(fd == NULL) {
		printf("Il file vincite dell'utente non e' stato trovato.\n");
		messaggio = "Nessuna vincita trovata.";
		ret = invia_risposta(cliente, messaggio);
		messaggio = "Inserisci un comando:";
		ret = invia_risposta(cliente, messaggio);
		return;
	}

	while(1) {
		res = fgets(buffer, BUFFER_SIZE, fd);
		if(res == NULL) {
			fclose(fd);
			break;
		}
		ret = invia_risposta(cliente, buffer);
	}

	consuntivo(cliente, username);

	messaggio = "Vincite inviate con successo.";
	ret = invia_risposta(cliente, messaggio);
	messaggio = "Inserisci un comando:";
	ret = invia_risposta(cliente, messaggio);

}

void esci(int cliente, char* username, char* session_id) {

	int ret, i;
	char* messaggio;
	char* res;
	char filename[250];
	FILE* fd;

	if(session_id[0] == '\0'){	/* Controllo sul session_id */
		printf("Session id non valido.\n");
		messaggio = "Session id non valido, effettuare il login per favore.";
		ret = invia_risposta(cliente, messaggio);
		messaggio = "Inserisci un comando:";
		ret = invia_risposta(cliente, messaggio);
		return;
	}

	strcpy(filename, "./files/utenti/");
	strcat(filename, username);
	strcat(filename, ".txt");
	fd = fopen(filename, "r+");
	if(fd == NULL) {
		printf("Errore esterno.\n");
		messaggio = "Errore esterno. Logout non effettuato.";
		ret = invia_risposta(cliente, messaggio);
		messaggio = "Inserisci un comando:";
		ret = invia_risposta(cliente, messaggio);
		return;
	}

	i = 0;
	while(1) {
		i++;
		res = fgets(buffer, BUFFER_SIZE, fd);
		if(i == 2) {
			ret = fprintf(fd, "          ");
			fclose(fd);
			break;
		}
	}
	
	messaggio = "Logout effettuato con successo. Grazie e arrivederci.";
	ret = invia_risposta(cliente, messaggio);
	close(cliente);
	printf("Client disconnesso con successo.\n");

}

int main(int argc, char* argv[]){

	int porta, timer;
	int ret, sd, len, new_sd, lunghezza;
	struct sockaddr_in my_addr, cl_addr;
	pid_t pid, p_estrazioni;
	uint16_t lmsg;

	if(argc < 2 || argc > 3){
		porta = 4243;
		timer = 70;
	}
	else {
		porta = atoi(argv[1]);
		if (argc == 3)
			timer = atoi(argv[2]); /* controllo sul formato */
		else 
			timer = 70; /* secondi */
	}

	/* Creazione socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd < 0)
		perror("Errore in fase di creazione del socket\n");
	else
		printf("Socket creato con successo\n");

	/* Creazione indirizzo di bind */
	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(porta);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	/* Aggancio del socket */
	ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr));
	if(ret < 0){
		perror("Errore in fase di bind: \n");
		exit(-1);
	}
	else
		printf("Bind superata\n");

	ret = listen(sd, 10);
	if(ret < 0){
		perror("Errore in fase di bind: \n");
		exit(-1);
	}
	else
		printf("Listen superata\n");

	p_estrazioni = fork();
	if(p_estrazioni < 0) 
		printf("Errore nella creazione del processo per le estrazioni.\n");
	else {
		if(p_estrazioni == 0) {
			while(1) {
				sleep(timer);
				char timestamp[36];
				estrazione_lotto(timestamp);
				printf("Nuova estrazione!\n");
				modifica_estrazioni(timestamp);
			}
		}
	}

	while(1) {
	
		lunghezza = sizeof(cl_addr);
		len = ntohs(lunghezza);

		/* Accetto nuove connessioni */
		/* printf("Sono arrivato alla accept()\n"); */
		new_sd = accept(sd, (struct sockaddr*)&cl_addr, &len);
		if(new_sd < 0) {
			perror("Errore in fase di accept:\n");
			exit(-1);
		}
		else {
			printf("Accept superata. L'indirizzo del client e': %s\n", inet_ntoa(cl_addr.sin_addr));
		}

		/* printf("%d\n", new_sd); */
		/* Creazione processo figlio */
		pid = fork();
		/* printf("%d\n", pid); */
		if(pid == 0) {
			/* Siamo nel processo figlio */
			/* Chiusura del listening socket */
			close(sd);
			while(1) {
				char* comando;
				pulisci();
		
				/* Riceve la dimensione del comando */
				ret = recv(new_sd, (void*)&lmsg, sizeof(uint16_t),0);

				/* Conversione in formato 'host' */
				len = ntohs(lmsg);

				/* Ricezione del comando in buffer */
				ret = recv(new_sd, (void*)buffer, len, 0);

				/* 	Inserisco il comando */
				comando = (char*)malloc((strlen(buffer)+1)*sizeof(char));
				strcpy(comando, buffer);
			
	
				if(strcmp(comando, "signup") == 0) {
					char username[30];
					char password[30];
					/* printf("Ho ricevuto la signup.\n"); */
					pulisci();
					ret = ricevi_risposta(new_sd);
					strcpy(username, buffer);
					/* printf("Ho ricevuto l'username.\n"); */
					pulisci();
					ret = ricevi_risposta(new_sd);
					/* printf("Ho ricevuto la password.\n"); */
					strcpy(password, buffer);
					signup(new_sd, username, password);
				}
				
				if(strcmp(comando, "login") == 0) {
					/* printf("Sono entrato nel login.\n"); */
					char username[30];
					char password[30];
					bool ritorna = false;
				
					/* printf("Ho ricevuto il login.\n"); */

					while(1) {
						pulisci();
						ret = ricevi_risposta(new_sd);
						if(strcmp(buffer, "ritorna") == 0) {
							ritorna = true;
							break;
						}
					
						if(strcmp(buffer, "\n") == 0) {
							break;
						}
					}
					if(ritorna)
						continue;

					pulisci();
					ret = ricevi_risposta(new_sd);
					if(strcmp(buffer, "ko") == 0)
						continue;

					pulisci();
					ret = ricevi_risposta(new_sd);
					strcpy(username, buffer);
					/* printf("Ho ricevuto l'username.\n"); */

					pulisci();
					ret = ricevi_risposta(new_sd);
					strcpy(password, buffer);
					/* printf("Ho ricevuto la password.\n"); */
					
					login(new_sd, username, password, &cl_addr);
				}
						
				if(strcmp(comando, "invia_giocata") == 0) {
					/* printf("Sono entrato nella invia giocata.\n"); */
					char username[30];
					char session_id[11];

					pulisci();
					ret = ricevi_risposta(new_sd);
					strcpy(session_id, buffer);
					/* printf("Ho ricevuto il session_id.\n"); */
					
					pulisci();
					ret = ricevi_risposta(new_sd);
					strcpy(username, buffer);
					/* printf("Ho ricevuto l'username.\n"); */

					invia_giocata(new_sd, username, session_id);

				}

				if(strcmp(comando, "vedi_giocate") == 0) {
					/* printf("Sono entrato nella vedi giocate.\n"); */
					char username[30];
					char session_id[11];
					char tipo[2];

					pulisci();
					ret = ricevi_risposta(new_sd);
					strcpy(session_id, buffer);
					/* printf("Ho ricevuto il session_id.\n"); */
					
					pulisci();
					ret = ricevi_risposta(new_sd);
					strcpy(username, buffer);
					/* printf("Ho ricevuto l'username.\n"); */

					pulisci();
					ret = ricevi_risposta(new_sd);
					strcpy(tipo, buffer);
					/* printf("Ho ricevuto il tipo.\n"); */

					vedi_giocate(new_sd, username, session_id, tipo);

				}

				if(strcmp(comando, "vedi_estrazione") == 0) {
					/* printf("sono entrato nella vedi estrazione.\n"); */
					char n[4];
					char ruota[11];
					char session_id[11];

					pulisci();
					ret = ricevi_risposta(new_sd);
					strcpy(session_id, buffer);
					/* printf("Ho ricevuto il session_id.\n"); */

					pulisci();
					ret = ricevi_risposta(new_sd);
					strcpy(n, buffer);
					/* printf("Ho ricevuto la n.\n"); */

					pulisci();
					ret = ricevi_risposta(new_sd);
					strcpy(ruota, buffer);
					/* printf("Ho ricevuto la ruota.\n"); */
					
					vedi_estrazione(new_sd, session_id, n, ruota);
					
				}

				if(strcmp(comando, "vedi_vincite") == 0) {
					/* printf("Sono entrato nella vedi_vincite.\n"); */
					char username[30];
					char session_id[11];

					pulisci();
					ret = ricevi_risposta(new_sd);
					strcpy(username, buffer);
					/* printf("Ho ricevuto l'username.\n"); */

					pulisci();
					ret = ricevi_risposta(new_sd);
					strcpy(session_id, buffer);
					/* printf("Ho ricevuto il session_id.\n"); */

					vedi_vincite(new_sd, username, session_id);

				}
				
				if(strcmp(comando, "esci") == 0) {
					/* printf("Sono entrato nella esci.\n"); */
					char username[30];
					char session_id[11];

					pulisci();
					ret = ricevi_risposta(new_sd);
					strcpy(username, buffer);
					/* printf("Ho ricevuto l'username.\n"); */

					pulisci();
					ret = ricevi_risposta(new_sd);
					strcpy(session_id, buffer);
					/* printf("Ho ricevuto il session_id.\n"); */

					esci(new_sd, username, session_id);

				}

			}

			close(new_sd);
			exit(0);
		}
		else {
			/* Processo padre */
			/* Chiusura del socket connesso */
			close(new_sd);
		}
	}
}
