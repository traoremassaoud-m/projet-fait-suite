#include <stdlib.h>
#include <stdio.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/types.h>

int main (int argc, char* argv[])
{
    int ordreMatrice = atoi(argv[1]), nbElement;
    int i = atoi(argv[2]);
    int j = atoi(argv[3]);
    int a, b;
    int *tableauI = NULL, *tableauJ = NULL;
    int *matrice = NULL;
    int cIJ = 0;

    int shmidMatrice;
    key_t keyMatrice;

    nbElementMatrice = ordreMatrice * ordreMatrice;


    printf("Processus %d,%d: %d\n", i, j, getpid());

    tableauI = malloc(ordreMatrice * sizeof(int));

    tableauJ = malloc(ordreMatrice * sizeof(int));

    if (tableauJ == NULL) {
        printf("L'allocation de mémoire est impossible\n");
        free(tableauI);
        exit(EXIT_FAILURE);
    }

    keyMatrice = ftok("shmMatrice", 19);
    shmidMatrice = shmget(keyMatrice, 2 * nbElement * sizeof(int), 0666);
    matrice = (int*) shmat(shmidMatrice,(void*)0,0);

    if (matrice == NULL) {
        printf("Processus %d,%d: La lecture de la mémoire partagée est impossible\n", i, j);
        free(tabI);
        free(tabJ);
        exit(EXIT_FAILURE);
    }


    for (a = 0; a < ordreMatrice; a++) {
        tableauI[a] = matrice[i*ordreMatrice + a];
        tableauJ[a] = matrice[j+ ordreMatrice * a];
    }


    printf("[i]: ");
    for (a = 0; a < ordreMatrice-1; a++) printf("%d, ", tableauI[a]);
    printf("%d", tableauI[a]);
    putchar('\n');
    printf("[j]: ");
    for (a = 0; a < ordreMatrice-1; a++) printf("%d, ", tableauJ[a]);
    printf("%d", tableauJ[a]);
    printf("\n\n");

    matrice[i*ordreMatrice + j + nbElement] = 0;
    for (a = 0; a < ordreMatrice; a++) {
        matrice[i*ordreMatrice + j + nbElement] += tabI[a] * tabJ[a];
    }
    printf("%d %d\n", i*ordreMatrice+j, matrice[i*ordreMatrice + j + nbElement]);

    shmdt(matrice);

    free(tableauI);
    free(tableauJ);

    return 0;
}
