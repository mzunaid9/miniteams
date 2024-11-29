#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define TAILLE_MAX 1024
#define BITS_PAR_CARACTERE 8
#define MAX_MESSAGES 100 // Nombre maximum de messages à stocker

char buffer_phrase[TAILLE_MAX];
size_t index_phrase = 0;
char caractere_courant = 0;
int bit_index = 0;

// Tableau pour stocker les messages reçus
char messages[MAX_MESSAGES][TAILLE_MAX];
size_t total_messages = 0;
int fin_reception = 0; // Indicateur pour terminer

// Gestionnaire pour recevoir chaque bit
void reception_bit(int sig) {
    if (sig == SIGUSR1) {
        caractere_courant &= ~(1 << bit_index); // Ajouter un 0 au bit courant
    } else if (sig == SIGUSR2) {
        caractere_courant |= (1 << bit_index); // Ajouter un 1 au bit courant
    }

    bit_index++;

    if (bit_index == BITS_PAR_CARACTERE) { // Un caractère complet a été reçu
        if (caractere_courant == '\0') { // Fin d'un message
            buffer_phrase[index_phrase] = '\0';

            if (strcmp(buffer_phrase, "exit") == 0) {
                fin_reception = 1; // Marque la fin de réception
            } else if (total_messages < MAX_MESSAGES) {
                // Stocker le message dans le tableau
                strcpy(messages[total_messages], buffer_phrase);
                total_messages++;
                //printf("Message reçu : %s\n", buffer_phrase);
            } else {
                fprintf(stderr, "Erreur : nombre maximal de messages atteint.\n");
            }

            index_phrase = 0; // Réinitialiser pour le prochain message
        } else {
            if (index_phrase < TAILLE_MAX - 1) {
                buffer_phrase[index_phrase++] = caractere_courant;
            } else {
                fprintf(stderr, "Erreur : message trop long.\n");
            }
        }

        caractere_courant = 0;
        bit_index = 0;
    }
}

int main() {
  // La structure sigaction configure la gestion des signaux dans un processus.
// Elle permet de définir une fonction (sa_handler) qui sera appelée à chaque réception d'un signal spécifique.
// Dans ce programme, sigaction est utilisée pour recevoir les bits via les signaux SIGUSR1 (0) et SIGUSR2 (1).
// Le gestionnaire de signal (reception_bit) traite ces signaux pour reconstituer les caractères et, à terme, le message complet.

//on a pas pu faire autrement , les autres tentatives sans structure n'ont rien donné 

//Pourquoi tout ce fractionnage ? Eh bien parce que dans les versions précédentes, les messages étaient coupés , mal convertis, ou carrément envoyés à moitié.. Il y a même eu des messages combinés entre eux gratuitement !

//la solution qu'on a trouvée ( après moultes essais, et multiples décompositions du projet à zéro  et tests unitaires étapes par étapes avec des petis cas simple , vu qu on a recommencé 
//5 ou 6 fois ) , on s'est dit que fractionner les messages en caractère , recomposer à la réception était une bonne //idée , et ca a fonctionné !! , ojn tuilise même les deux signaux pour ne pas surcharger un seul signal 
// ca a coûté deux machines virtuelles ( merci ouali t'est fort), mais voila//

//tout ca pour dire , chaque message est bien envoyé et traité séparémment//

  
    struct sigaction sa;
    
    // Gestion des signaux pour recevoir les bits
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = reception_bit;

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Erreur sigaction SIGUSR1");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("Erreur sigaction SIGUSR2");
        exit(EXIT_FAILURE);
    }

    printf("PID du récepteur : %d\n", getpid());
    printf("En attente de messages...\n");

    while (!fin_reception) {
        pause(); // Attendre les signaux
    }

    // Afficher tous les messages reçus à la fin
    printf("\n--- Tous les messages reçus ---\n");
    for (size_t i = 0; i < total_messages; i++) {
        printf("%zu : %s\n", i + 1, messages[i]);
    }
    printf("-------------------------------\n");

    return 0;
}
