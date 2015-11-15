/*
Szyfrowanie AES z wykorzystaniem trybu CTR 128

Tryb CTR pozwala na odszyfrowanie dowolnego fragmentu danych, bez koniecznoœci poznania wczeœniejszych bloków. Z tego powodu wybra³am
ten rodzaj szyfrowania do tego zadania.

C:\privatekey. keystore -- sciezka zawierajaca klucz, ktorym szyfrowane sa pliki

Po uruchomieniu funkcji run() po raz pierwszy, program prosi o podanie PINu, zapisuje go w zmiennej ckey, szyfruje, a nastepnie zapisuje w pliku passEnc.txt
Nastepnie prosi o podanie sciezki do pliku zawierajacego klucz. Klucz ten zostal wygenerowany przez opcjê keygen w konsoli.
Program odczytuje zawartosc pliku privatekey.keystore, szyfruje go kluczem k2 (zaszytym w tym programie, linia 54), a nastepnie zapisuje w pliku configEnc.txt
Wszystkie posrednio tworzone pliki s¹ automatycznie kasowane.
Gdy program uruchamiany jest po raz drugi, program prosi o podanie has³a. Deszyfruje plik pinEnc.txt i porównuje wynik z podanym has³em. Jeœli has³a siê zgadzaj¹,
nastêpuje odszyfrowanie pliku configEnc i zapisanie jego zawartosci w zmiennej ckey. Dziêki temu, odszyfrowany klucz nie jest zapisywany z powrotem na dysku.
Nastêpnie u¿ytkownik mo¿e szyfrowaæ/deszyfrowaæ wskazane przez siebie pliki na dysku.

FUnkcja decrypt() ma mo¿liwoœæ odszyfrowywania wskazanego pliku i zapisania jego zawartoœci na dysku LUB zapisania zawartoœci w zmiennej char[]. Dziêki temu
odszyfrowane pliki poœrednie (jak configEnc.txt) nie s¹ z powrotem zapisywane na dysku.
Funkcja encrypt() nie ma takiej mo¿liwoœci (zabrak³o mi czasu).
*/

#define _CRT_SECURE_NO_DEPRECATE
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <openssl/rand.h>
#include <sys/stat.h>
#include <conio.h>   

/*
ivec - licznik
ecount_buf - zmienna, dziêki której mo¿na zaszyfrowaæ ivec
num - indeks
*/


struct ctr_state {
	unsigned char ivec[16];
	unsigned char ecount[16];
	unsigned int num;
};

FILE *fp, *encrfp, *decrfp;
size_t count;
AES_KEY key;

int bytes_read, bytes_written;
unsigned char message_in[AES_BLOCK_SIZE], message_out[AES_BLOCK_SIZE];
char *ckey;
unsigned char *k2 = "ahbsihbvfieyv27323g27328";  //klucz szyfruj¹cy config.txt oraz pass.txt
unsigned char iv[8] = "alamakot";
struct ctr_state state;
char pin[100] = "";
char ckeypath[1500] = "";

void init_ctr(struct ctr_state *state, const unsigned char iv[8]) {
	state->num = 0;
	memset(state->ecount, 0, 16);
	memset(state->ivec + 8, 0, 8);
	memcpy(state->ivec, iv, 8);
}

/* Funkcja set_key pobiera sciezke do klucza, czyta dany plik i zapisuje jego zawartosc do zmiennej ckey */
void set_key(char *keystorePath) {

	//char *source = NULL;
	FILE *keystoreFp = fopen(keystorePath, "rb");
	if (keystoreFp != NULL) {
		if (fseek(keystoreFp, 0L, SEEK_END) == 0) {
			long bufsize = ftell(keystoreFp);
			if (bufsize == -1) {}

			ckey = (char*)malloc(bufsize + 1);
			printf("%d\n", bufsize);
			if (fseek(keystoreFp, 0L, SEEK_SET) != 0) {}

			size_t newLen = fread(ckey, sizeof(char), bufsize, keystoreFp);
			if (newLen == 0) {
				fputs("Error reading file", stderr);
			}
			else {
				ckey[newLen++] = '\0';
			}
		}
		fclose(keystoreFp);
	}
}
void encrypt(char *startFile, char *enrcyptedFile, char *k2key) {

	/*
	otwieranie plikow
	plik1.txt zawiera tekst do zaszyfrowania
	plik2.txt zawiera tekst zaszyfrowany
	k2key - klucz, ktorym ma byc szyfrowany plik (tu ckey lub k2)
	*/
	fp = fopen(startFile, "a+");
	encrfp = fopen(enrcyptedFile, "a+");
	if (fp == NULL) { fputs("File error", stderr); exit(1); }
	if (encrfp == NULL) { fputs("File error", stderr); exit(1); }

	//inicializacja zmiennej KEY (klucz szyfruj¹cy)
	AES_set_encrypt_key(k2key, 128, &key);

	while (1) {

		init_ctr(&state, iv); //Counter call
		bytes_read = fread(message_in, 1, AES_BLOCK_SIZE, fp);
		AES_ctr128_encrypt(message_in, message_out, bytes_read, &key, state.ivec, state.ecount, &state.num);
		bytes_written = fwrite(message_out, 1, bytes_read, encrfp);

		//	bytes_written = fwrite(message_out, 1, bytes_read, odszyfrowane);
		if (bytes_read < AES_BLOCK_SIZE)
			break;
	}
	fclose(fp);
	fclose(encrfp);
}
/*
param: encryptedFile - nazwa pliku zaszyfrowanego ; dercyptedFile - nazwa pliku odszyfrowanego ; sekunda - od której sekundy rozpocz¹æ odtwarzanie
file_or_char - odszyfrowac do pliku (1) lub do zmiennej char[] (0) ; k2key - klucz, ktorym ma byc odszyfrowany plik
*/
void decrypt(char *encryptedFile, char *dercyptedFile, int sekunda, int file_or_char, char *k2key) {
	/*
	odszyfrowywamoe odbywa siê dok³adnie w ten sam sposób co szyfrowanie, tylko tym razem
	plikiem odczytywanym jest ten zaszyfrowany
	*/
	encrfp = fopen(encryptedFile, "a+");
	decrfp = fopen(dercyptedFile, "a+");
	if (encrfp == NULL) { fputs("File error", stderr); exit(1); }
	if (decrfp == NULL) { fputs("File error", stderr); exit(1); }
	int bytes_written22;
	AES_set_encrypt_key(k2key, 128, &key);
	int licznik = 0, przesuniecie = 0;
	char *message = (char *)malloc(sizeof(char) * 3);
	while (1) {

		init_ctr(&state, iv);
		bytes_read = fread(message_in, 1, AES_BLOCK_SIZE, encrfp);
		if (licznik >= 1000 * sekunda) { // licznik pozwala na zdeszyfrowanie pliku od pewnego momentu
			AES_ctr128_encrypt(message_in, message_out, bytes_read, &key, state.ivec, state.ecount, &state.num);
			if (file_or_char == 1) {
				memcpy(pin + przesuniecie*sizeof(message_out), message_out, AES_BLOCK_SIZE);
				przesuniecie = przesuniecie + 1;
			}
			else {
				bytes_written = fwrite(message_out, 1, bytes_read, decrfp);
			}
		}
		if (bytes_read < AES_BLOCK_SIZE)
			break;
		licznik++;

	}


	fclose(encrfp);
	fclose(decrfp);

	//return (char *)message;
}

int isEmpty(FILE *file)
{
	long savedOffset = ftell(file);
	fseek(file, 0, SEEK_END);

	if (ftell(file) == 0)
	{
		return 1;
	}

	fseek(file, savedOffset, SEEK_SET);
	return 0;
}

void copy_string(char *target, char *source) {
	while (*source) {
		*target = *source;
		source++;
		target++;
	}
	*target = '\0';
}

void displayMenu() {
	printf("\nWybierz opcje\n");
	printf("1 -szyfruj \n2 - deszyfruj \n3 - koniec\n");
}

/* Tu uruchamiana jest instalacja programu lub sam program (jesli ten jest juz zainstalowany).*/
void run() {

	int sekunda = 50; // od której sekundy rozpocz¹æ odwtarzanie. Wartoœæ 1 oznacza pierwsz¹ sekundê
	FILE *config_pin, *config_key, *config_pin_temp, *config_temp_key;
	config_pin_temp = fopen("pass.txt", "a+");
	config_temp_key = fopen("config.txt", "a+");
	config_pin = fopen("passEnc.txt", "a+");
	config_key = fopen("configEnc.txt", "a+");

	int empty = isEmpty(config_pin);
	int file_or_char = 1;
	int opcja = 0;
	char *path;
	char path2[20] = "";
	path = (char*)malloc(30 + 1);

	if (config_pin != NULL && config_key != NULL) {
		if (empty == 0) {
			/*
			* Wczytywanie hasla PIN
			*/
			printf("Podaj PIN (na koncu wcisnij TAB a nie ENTER):\n");
			fclose(config_pin);
			char temp_pin[64], path[64];
			decrypt("passEnc.txt", "pass3.txt", 0, 1, k2);
			remove("pass3.txt");
			int i = 0;
			char *c;
			while ((c = _getch())) {
				if (c != '	') {
					temp_pin[i] = c;
					printf("");
					i++;
				}
				else break;
			}
			temp_pin[i] = '\0';
			//if (scanf_s("%s", temp_pin, sizeof(temp_pin)) == 1) {
			/*
			* Sprawdzanie, czy PIN sie zgadza
			*/
			if (strcmp(temp_pin, pin) == 0) {
				printf("Poprawny pin\n");
				decrypt("configEnc.txt", "config3.txt", 0, 0, k2);
				set_key("config3.txt");
				printf("\nDECRYPTED KEY = %s\n", ckey);
				remove("config3.txt");

				printf("Wybierz opcje\n");
				printf("1 -szyfruj \n2 - deszyfruj \n3 - koniec\n");
				while (1) {
					scanf("%d", &opcja);
					if (opcja == 1) {
						printf("SZYFROWANIE. Podaj sciezke do pliku: \n");
						if (scanf_s("%s", path, sizeof(path)) == 1) {
							memcpy(path2, &path[0], (strlen(path) - 4));
							path2[(strlen(path) - 4)] = '\0';
							strncat(path2, "enc.txt", 7);
							encrypt(path, path2, k2);
							printf("Zaszyfrowano plik %s do pliku %s\n", path, path2);
							displayMenu();
						}
					}
					else if (opcja == 2) {
						printf("DESZYFROWANIE. Podaj sciezke do pliku: \n");
						if (scanf_s("%s", path, sizeof(path)) == 1) {
							memcpy(path2, &path[0], (strlen(path) - 4));
							path2[(strlen(path) - 4)] = '\0';
							strncat(path2, "dec.txt", 7);
							encrypt(path, path2, k2);
							printf("Odszyfrowano plik %s do pliku %s\n", path, path2);
							displayMenu();
						}
					}
					else {
						break;
					}
				}

			}
			else {
				printf("Bledny pin! \n");
			}
			//}


		}
		else {

			printf("Instalacja programu.\n");
			printf("Podaj PIN (na koniec prosze wcisnac TAB) ");

			/*
			*Interaktywne wczytywanie hasla
			*/
			int i = 0;
			char c;
			while ((c = _getch())) {
				if (c != '	') {
					pin[i] = c;
					printf("");
					i++;
				}
				else break;
			}
			pin[i] = '\0';



			//	if (scanf_s("%s", pin, sizeof(pin)) == 1) {
			printf("Ustalony pin %s\n", pin);
			int results = fputs(pin, config_pin_temp);
			if (results == EOF) {
				printf("Failed\n");
			}
			fclose(config_pin_temp);
			encrypt("pass.txt", "passEnc.txt", k2);
			remove("pass.txt");

			printf("Haslo zostalo umieszczone w pliku passEnc.txt i zaszyfrowane\n\n");

			printf("Podaj sciezke do klucza: ");
			if (scanf_s("%s", ckeypath, sizeof(ckeypath)) == 1) {
				printf("Podana sciezka  %s\n", ckeypath);
				set_key((char*)ckeypath);
				//	printf("%s\n", ckey);
				int results2 = fputs(ckey, config_temp_key);
				if (results2 == EOF) {
					printf("Failed\n");
				}
				fclose(config_temp_key);
				fclose(config_key);

				printf("Utworzono config.txt \n\n");
				encrypt("config.txt", "configEnc.txt", k2);
				remove("config.txt");
				printf("Zaszyfrowano configEnc.txt \n\n");

			}
		}
	}


}
int main(int argc, char *argv[]) {

	run();

	//free(ckey);
	return 0;
}