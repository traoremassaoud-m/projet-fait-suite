#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

#define SPACE ' '

int printcol(const int colWidth, const int nb);


int nbChiffre(int nb);

int main(int argc, char* argv[])
{
    FILE* fichierMatrice = NULL;
    int* matrice = NULL;
    int ordreMatrice, nbElement, tailleColonne = 0, status;
    pid_t** pid_tTab = NULL;
    int i, j, a, b;
    char* argp[] = {"calcul_cij", NULL, NULL, NULL};

    char* chO, *chI, *chJ = NULL;

    int shmidMatrice;
    key_t keyMatrice;

    if (argc != 2 ) {
        printf("USAGE: <fichier_matrice>\n");
        exit(EXIT_FAILURE);
    }

    printf("\n");
    fichierMatrice = fopen(argv[1], "r");

    if (fichierMatrice == NULL) {
        printf("\nLe fichier %s n'exite pas!\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    fscanf(fichierMatrice, "%d ", &ordreMatrice);

    if (ordreMatrice < 2) {
        printf("\nL'ordre de la matrice doit être forcément supérieur à 2 !\n");
        fclose(fichierMatrice);
        exit(EXIT_FAILURE);
    }

    nbElementMatrice = ordreMatrice * ordreMatrice;

    keyMatrice = ftok("shmMatrice", 19);
    shmidMatrice = shmget(keyMatrice, 2 * nbElement * sizeof(int), 0666|IPC_CREAT);
    matrice = (int*) shmat(shmidMatrice,(void*)0,0);

    if (matrice == NULL) {
        printf("Allocation de mémoire impossible !\n");
        fclose(fichierMatrice);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < ordreMatrice; i++) {
        for (j = 0; j< ordreMatrice; j++) {
            fscanf(fichierMatrice, "%d ", &matrice[i*ordreMatrice + j]);

            if (tailleColonne < nbChiffre(matrice[i*ordreMatrice + j]))
                tailleColonne = nbChiffre(matrice[i*ordreMatrice + j]);

            if (feof(fichierMatrice) && j != ordreMatrice-1) {
                fclose(fichierMatrice);

                shmdt(matrice);
                shmctl(shmidMatrice, IPC_RMID, NULL);

                printf("Matrice carrée en entrée incomplète !\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(fichierMatrice);
    printf("Fin lecture de la matrice en fichier !\n\nOrdre de la matrice: %d\n\n", ordreMatrice);


    for (i = 0; i < ordreMatrice; i++) {

        putchar('+');

        for (j = 0; j < ordreMatrice; j++) {
            for (a = 0; a < tailleColonne; a++)
                putchar('-');
            putchar('+');
        }
        printf("\n|");

        for (j = 0; j < ordreMatrice; j++) {
            printcol(tailleColonne, matrice[i*ordreMatrice + j]);
            putchar('|');
        }
        printf("\n");
    }
    putchar('+');
    for (i = 0; i < ordreMatrice; i++) {
        for (j = 0; j < tailleColonne; j++)
            putchar('-');
        putchar('+');
    }
    printf("\n\n");

    pid_tTab = malloc(ordreMatrice * sizeof(pid_t*));

    if (pid_tTab == NULL) {
        printf("Allocation de mémoire impossible !\n");

        shmdt(matrice);
        shmctl(shmidMatrice, IPC_RMID, NULL);

        exit(EXIT_FAILURE);
    }

    for (i = 0; i < ordreMatrice; i++) {
        pid_tTab[i] = malloc(ordreMatrice * sizeof(pid_t));

        if (pid_tTab[i] == NULL) {
            printf("Allocation de mémoire impossible !\n");

            for (j = 0; j < i; j++)
                free(pid_tTab[j]);

            free(pid_tTab);
            shmdt(matrice);
            shmctl(shmidMatrice, IPC_RMID, NULL);

            exit(EXIT_FAILURE);
        }
    }

    chO = malloc((1 + nbChiffre(ordreMatrice)) * sizeof(char));

    if (chO == NULL) {
        printf("Allocation de mémoire impossible !\n");
            for (i = 0; i < ordreMatrice; i++)
                free(pid_tTab[i]);

        free(pid_tTab);

        shmdt(matrice);
        shmctl(shmidMatrice, IPC_RMID, NULL);

        exit(EXIT_FAILURE);
    }

    sprintf(chO, "%d", ordreMatrice);
    argv[1] = chO;

    chI = malloc((1 + nbChiffre(ordreMatrice)) * sizeof(char));

    if (chI == NULL) {
        printf("Allocation de mémoire impossible !\n");
            for (i = 0; i < ordreMatrice; i++)
                free(pid_tTab[i]);

        free(chO);

        free(pid_tTab);

        shmdt(matrice);
        shmctl(shmidMatrice, IPC_RMID, NULL);

        exit(EXIT_FAILURE);
    }

    chJ = malloc((1 + nbChiffre(ordreMatrice)) * sizeof(char));

    if (chJ == NULL) {
        printf("Allocation de mémoire impossible !\n");
            for (i = 0; i < ordreMatrice; i++)
                free(pid_tTab[i]);

        free(chO);
        free(chI);

        free(pid_tTab);

        shmdt(matrice);
        shmctl(shmidMatrice, IPC_RMID, NULL);

        exit(EXIT_FAILURE);
    }

    for (i = 0; i < ordreMatrice; i++) {
        for (j = 0; j < ordreMatrice; j++) {

            sprintf(chI, "%d", i);
            argv[2] = chI;
            sprintf(chJ, "%d", j);
            argv[3] = chJ;

            pid_tTab[i][j] = fork();

            if (pid_tTab[i][j] < 0) {
                printf("Création du processus pour le calcul du c[%d,%d] impossible \n", i, j);

                for (a = 0; a <= i; a++) {
                    for (b = 0; b < j; b++)
                        wait(&status);
                }

                for (a = 0; a < ordreMatrice; a++)
                    free(pid_tTab[i]);

                free(chO);
                free(chI);
                free(chJ);
                free(pid_tTab);

                shmdt(matrice);

                shmctl(shmidMatrice, IPC_RMID, NULL);

                exit(EXIT_FAILURE);
            }

            if (pid_tTab[i][j] == 0) {
                execv("./calcul_cij", argv);
            }
            sleep(2);
        }
    }


    if (getpid() > 0) {

        tailleColonne = 0;

        for (i = 0; i < nbElement; i++) {
            if (tailleColonne < nbChiffre(matrice[i+nbElement]))
                tailleColonne = nbChiffre(matrice[i+nbElement]);
        }

    for (i = 0; i < ordreMatrice; i++) {

        putchar('+');

        for (j = 0; j < ordreMatrice; j++) {
            for (a = 0; a < tailleColonne; a++)
                putchar('-');
            putchar('+');
        }
        printf("\n|");

        for (j = 0; j < ordreMatrice; j++) {
            printcol(tailleColonne, matrice[i*ordreMatrice + j + nbElement]);
            putchar('|');
        }
        printf("\n");
    }
    putchar('+');
    for (i = 0; i < ordreMatrice; i++) {
        for (j = 0; j < tailleColonne; j++)
            putchar('-');
        putchar('+');
    }
    printf("\n\n");

//FArrêt du programme

        free(chO);
        free(chI);
        free(chJ);

        for (i = 0; i < ordreMatrice; i++) {
            for(j = 0; j < ordreMatrice; j++)
                wait(&status);

            for(j = 0; j < ordreMatrice; j++) printf("Arrêt du processus %d %d\n", status, pid_tTab[i][j]);

            free(pid_tTab[i]);
        }

        free(pid_tTab);
        shmdt(matrice);
        shmctl(shmidMatrice, IPC_RMID, NULL);
        putchar('\n');
    }
}

int nbChiffre(int nb)
{
    int i;
    for (i = (nb <= 0) ? 1 : 0; nb != 0; i++) nb /= 10;
    return i;
}

int printcol(const int colWidth, const int nb)
{
    const int strWidth = nbChiffre(nb);
    int nbrSpace;

    if (colWidth > 0 && strWidth <= colWidth){
        for (nbrSpace = colWidth - strWidth; nbrSpace > 0; nbrSpace--)
            putchar(SPACE);
        printf("%d", nb);
        return 1;
    }
    else return 0;
}
