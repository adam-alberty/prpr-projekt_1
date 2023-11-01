#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ID_MODULU 0
#define POZICIA_MODULU 1
#define TYP_VELICINY 2
#define HODNOTA 3
#define CAS_MERANIA 4
#define DATUM_MERANIA 5

// Prints items from dynamic array
void print_items(char ***items_global, int item_count_global) {
    for (int i = 0; i < item_count_global; i++) {
        for(int j = 0; j < 6; j++) {
            printf("%s\n", items_global[i][j]);
        }
        printf("\n");
    }
}

// Prints items from file
void print_file(FILE **fp) {
    char line[20];

    fseek(*fp, 0, SEEK_SET);
    while (fgets(line, 20, *fp) != NULL) {
        printf("%s", line);
    }
}

// Opens file and reads it
void open_file(FILE **fp, char ***items_global, int item_count_global) {
    if (*fp != NULL) {
        if (items_global != NULL) {
            print_items(items_global, item_count_global);
        } else {
            print_file(fp);
        } 
        return;
    }

    *fp = fopen("./dataloger.txt", "r");
    if (*fp == NULL) {
        printf("Neotvoreny subor\n");
        return;
    }

    print_file(fp);
}

// Count items from file
int count_items(FILE *fp) {
    fseek(fp, 0, SEEK_SET);

    int item_count = 0;
    char c;
    while((c = fgetc(fp)) != EOF) {
        if (c == '\n') {
            item_count++;
        }
    }
    return item_count / 6;
}

void deallocate_arrays(char ****items_global, int *item_count_global) {
    for (int i = 0; i < *item_count_global; i++) {
        for (int j = 0; j < 6; j++) {
            free((*items_global)[i][j]);
        }
        free((*items_global)[i]);
    }
    free(*items_global);

    *items_global = NULL;
    *item_count_global = 0;
}

// Allocate arrays dynamically   
void allocate_arrays(FILE **fp, char ****items_global, int *item_count_global) {
    if (*fp == NULL) {
        printf("Neotvoreny subor\n");
        return;
    }

    if (*items_global != NULL) {
        deallocate_arrays(items_global, item_count_global);
    }

    int item_count = count_items(*fp);
    fseek(*fp, 0, SEEK_SET);

    // Allocate main array - items
    char ***items = (char***)malloc(sizeof(char**) * item_count);

    if (items == NULL) {
        printf("Nepodarilo sa alokovat");
        exit(1);
    }

    for (int i = 0; i < item_count; i++) {
        // Allocate array of strings - item
        char **item = (char**)malloc(sizeof(char*) * 6);
        if (item == NULL) {
            printf("Nepodarilo sa alokovat");
            exit(1);
        }

        for (int j = 0; j < 6; j++) {
            // Allocate memory for string - line;
            char *string = malloc(sizeof(char) * 20);
            if (string == NULL) {
                printf("Nepodarilo sa alokovat");
                exit(1);
            }

            fgets(string, 20, *fp);
            string[strlen(string) - 1] = '\0';
            item[j] = string;
        }
        char throwaway[10];
        fgets(throwaway, 10, *fp);
        items[i] = item;
    }

    *items_global = items;
    *item_count_global = item_count;
}

void ciachovanie(char ***items_global, int item_count_global) {
    if (items_global == NULL) {
        printf("Polia nealokovane\n");
        return;
    }
    
    int Y;

    printf("Zadaj pocet mesiacov: ");
    scanf("%d", &Y);
    FILE *fp = fopen("./ciachovanie.txt", "r");
    if (fp == NULL) {
        return;
    };


    for (int i = 0; i < item_count_global; i++) {
        fseek(fp, 0, SEEK_SET);
        char line[20];

        int k = -1;
        while (fgets(line, 20, fp) != NULL) {
            k = (k + 1) % 3;

            if (k == 2) {
                fgets(line, 20, fp);
                continue;
            }

            if (k == 0) {
                printf("Name: %s\n", items_global[i][ID_MODULU]);
                printf("LINE ID MODULU: %s", line);
            }

            if (k == 1) {
                printf("ARRAY DATUM: %s\n", items_global[i][DATUM_MERANIA]);
                printf("LINE DATUM: %s", line);
            }
        }
    }
}

int count_items_with_id(char *module_id, char *unit, char ***items_global, int item_count_global) {
    int count = 0;
    for (int i = 0; i < item_count_global; i++) {
        if (strcmp(items_global[i][ID_MODULU], module_id) == 0 && strcmp(items_global[i][TYP_VELICINY], unit) == 0)
            count++;
    }

    return count;
}

void vystup(char ***items_global, int item_count_global) {
    if (items_global == NULL) {
        printf("Polia nie su vytvorene\n");
        return;
    }

    char module_id[6];
    char unit[3];

    scanf("%s %s", module_id, unit);
    int count = count_items_with_id(module_id, unit, items_global, item_count_global);
    if (count == 0) {
        printf("Pre dany vstup neexistuju zaznamy");
    }

    char **arr = malloc(sizeof(char *) * count);

    int arr_i = 0; 
    for (int i = 0; i < item_count_global; i++) {
        if (strcmp(items_global[i][ID_MODULU], module_id) == 0 && strcmp(items_global[i][TYP_VELICINY], unit) == 0) {
            char *value = malloc(sizeof(char) * 50);
            strcpy(items_global[], value);
            arr[arr_i] = value; 
            arr_i++;
        }
    }
}

int main(void) {
    char command;
    FILE *fp = NULL;
    char ***items = NULL;
    int item_count = 0;

    // Hlavny cyklus na zadavanie prikazov
    while (command = getchar()) {
        switch(command) {
            case 'v':
                open_file(&fp, items, item_count);
                break;
            case 'n':
                allocate_arrays(&fp, &items, &item_count);
                break;
            case 'c':
                // TODO
                ciachovanie(items, item_count);
                break;
            case 's':
                // TODO
                vystup(items, item_count);
                break;
        }
    }    
    return 0;
}