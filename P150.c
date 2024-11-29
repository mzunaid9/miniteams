#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define BITS_PAR_CARACTERE 8
//on extrait chaque caractère du message pour convertir et evnoyer séparement//
//SIGUSR ne permet d'envoyer que des valeurs via la structure sigaction (variable système utilisée avec sigqueue
//fonction d'envoi de valeur ), on a donc choisi de fractionner, convertir, envoyer, puis recomposer les messages
//pour ne pas avoir de perte, comme dans les verions précedentes( oui c'est la 6ème version)//

void envoyer_caractere(char caractere, pid_t pid_cible) {
    for (int i = 0; i < BITS_PAR_CARACTERE; i++) {
        int bit = (caractere >> i) & 1; // Extraire le i-ème bit
        if (bit == 0) {
            if (kill(pid_cible, SIGUSR1) == -1) {
                perror("Erreur SIGUSR1");
                exit(EXIT_FAILURE);
            }
        } else {
            if (kill(pid_cible, SIGUSR2) == -1) {
                perror("Erreur SIGUSR2");
                exit(EXIT_FAILURE);
            }
        }
        usleep(10000); // Pause pour permettre la réception
    }
}

//Chaque caractère est envoyé séparament

void envoyer_phrase(const char *phrase, pid_t pid_cible) {
    while (*phrase) {
        envoyer_caractere(*phrase, pid_cible);
        phrase++;
    }
    envoyer_caractere('\0', pid_cible); // Envoyer le caractère de fin
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <PID_CIBLE> <PHRASE>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    //atoi permet de convertir l'argument passé en chaine de caractères en entier , le pid étant un nombre//
    pid_t pid_cible = atoi(argv[1]);
    const char *phrase = argv[2];
  
    // appel de la fonction d'envoi( et donc de fractionnage//  
    envoyer_phrase(phrase, pid_cible);

    printf("Phrase envoyée au PID %d : %s\n", pid_cible, phrase);

    return 0;
}

