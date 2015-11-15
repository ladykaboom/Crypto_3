/*==============================================================================
Do odtwarzania muzyki wykorzystana zosta³a biblioteka FMOD. U¿y³am przyk³adu z dokumentacji programu, mianowicie playsteram.cpp
Przenios³am do programu funkcjê run() z pliku szyfrowanie.c
Funkcja run() jest opisana w pliku z zadania 1
==============================================================================*/
#include "fmod.hpp"
#include "common.h"
#include "szyfrowanie.c"
#include <stdlib.h>
#include <iostream>


int FMOD_Main()
{
	FMOD::System     *system;
	FMOD::Sound      *sound, *sound_to_play;
	FMOD::Channel    *channel = 0;
	FMOD_RESULT       result;
	unsigned int      version;
	void             *extradriverdata = 0;
	int               numsubsounds;



	int sekunda = 0; // od której sekundy rozpocz¹æ odwtarzanie. Wartoœæ 1 oznacza pierwsz¹ sekundê
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
			char c;
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
							strncat(path2, "enc.mp3", 7);
							encrypt(path, path2, k2);
							printf("Zaszyfrowano plik %s do pliku %s\n", path, path2);
							displayMenu();
						}
					}
					else if (opcja == 2) {
						printf("ODTWARZANIE MUZYKI. Podaj sciezke do zaszyfrowanego pliku: \n");
						if (scanf_s("%s", path, sizeof(path)) == 1) {
							printf("Od ktorej sekundy odtworzyc muzyke? (1 = 1 sekunda etc...) \n");
							std::cin >> sekunda;
							memcpy(path2, &path[0], (strlen(path) - 4));
							path2[(strlen(path) - 4)] = '\0';
							strncat(path2, "dec.mp3", 7);
							decrypt(path, path2,sekunda,0, k2);

							/* START MP3*/
							Common_Init(&extradriverdata);

							/*
							Create a System object and initialize.
							*/
							result = FMOD::System_Create(&system);
							ERRCHECK(result);

							result = system->getVersion(&version);
							ERRCHECK(result);

							if (version < FMOD_VERSION)
							{
								Common_Fatal("FMOD lib version %08x doesn't match header version %08x", version, FMOD_VERSION);
							}

							result = system->init(32, FMOD_INIT_NORMAL, extradriverdata);
							ERRCHECK(result);

							/*
							This example uses an FSB file, which is a preferred pack format for fmod containing multiple sounds.
							This could just as easily be exchanged with a wav/mp3/ogg file for example, but in this case you wouldnt need to call getSubSound.
							Because getNumSubSounds is called here the example would work with both types of sound file (packed vs single).
							*/

							/*start my */

							/*end my*/
							result = system->createStream(Common_MediaPath(path2), FMOD_LOOP_NORMAL | FMOD_2D, 0, &sound);
							//	result = system->createStream(source, FMOD_LOOP_NORMAL | FMOD_2D, 0, &sound);
							ERRCHECK(result);

							result = sound->getNumSubSounds(&numsubsounds);
							ERRCHECK(result);

							if (numsubsounds)
							{
								sound->getSubSound(0, &sound_to_play);
								ERRCHECK(result);
							}
							else
							{
								sound_to_play = sound;
							}
							/*
							Play the sound.
							*/
							result = system->playSound(sound_to_play, 0, false, &channel);
							ERRCHECK(result);

							/*
							Main loop.
							*/
							do
							{
								//	printf("Uwaga: %s \n", sound_to_play);

								Common_Update();

								if (Common_BtnPress(BTN_ACTION1))
								{
									bool paused;
									result = channel->getPaused(&paused);
									ERRCHECK(result);
									result = channel->setPaused(!paused);
									ERRCHECK(result);
								}

								result = system->update();
								ERRCHECK(result);

								{
									unsigned int ms = 0;
									unsigned int lenms = 0;
									bool         playing = false;
									bool         paused = false;

									if (channel)
									{
										result = channel->isPlaying(&playing);
										if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE))
										{
											ERRCHECK(result);
										}

										result = channel->getPaused(&paused);
										if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE))
										{
											ERRCHECK(result);
										}

										result = channel->getPosition(&ms, FMOD_TIMEUNIT_MS);
										if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE))
										{
											ERRCHECK(result);
										}

										result = sound_to_play->getLength(&lenms, FMOD_TIMEUNIT_MS);
										if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE))
										{
											ERRCHECK(result);
										}
									}

									Common_Draw("Play");
									Common_Draw("Copyright (c) Firelight Technologies 2004-2015.");
									Common_Draw("==================================================");
									Common_Draw("");
									Common_Draw("Press %s to toggle pause", Common_BtnStr(BTN_ACTION1));
									Common_Draw("Press %s to quit", Common_BtnStr(BTN_QUIT));
									Common_Draw("");
									Common_Draw("Time %02d:%02d:%02d/%02d:%02d:%02d : %s", ms / 1000 / 60, ms / 1000 % 60, ms / 10 % 100, lenms / 1000 / 60, lenms / 1000 % 60, lenms / 10 % 100, paused ? "Paused " : playing ? "Playing" : "Stopped");
								}

								Common_Sleep(50);
							} while (!Common_BtnPress(BTN_QUIT));

							/*
							Shut down
							*/
							result = sound->release();  /* Release the parent, not the sound that was retrieved with getSubSound. */
							ERRCHECK(result);
							result = system->close();
							ERRCHECK(result);
							result = system->release();
							ERRCHECK(result);

							Common_Close();

							/* END MP3*/

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




//	if (remove("Media/newmusicfile.mp3") != 0)
	//	perror("Error deleting file");
//	else
//		puts("File successfully deleted");
//	return 0;
	return 0;
}
