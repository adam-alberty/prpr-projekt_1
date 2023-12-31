#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ID_MODULU 0
#define POZICIA_MODULU 1
#define TYP_VELICINY 2
#define HODNOTA 3
#define CAS_MERANIA 4
#define DATUM_MERANIA 5

// Prints items from dynamic array
void print_items(char **items_global, int item_count_global) {
    for (int i = 0; i < item_count_global * 6; i++) {
        printf("%s\n", items_global[i]);
    }
}

// Prints items from file
void print_file(FILE **fp) {
    char line[20];

    fseek(*fp, 0, SEEK_SET);
    while (fgets(line, sizeof(line) - 1, *fp) != NULL) {
        printf("%s", line);
    }
}

// Opens file and reads it (from dynamic array or raw file)
void open_file(FILE **fp, char **items_global, int item_count_global) {
    if (*fp != NULL) {
        if (items_global != NULL) {
            print_items(items_global, item_count_global);
        } else {
            print_file(fp);
        } 
        return;
    }

    *fp = fopen("dataloger.txt", "r");
    if (*fp == NULL) {
        printf("Neotvoreny subor\n");
        return;
    }
    print_file(fp);
}

// Count items from file
int count_items(FILE *fp) {
    fseek(fp, 0, SEEK_SET);

    int line_count = 1;
    char c;
    while((c = fgetc(fp)) != EOF) {
        if (c == '\n') {
            line_count++;
        }
    }
    return line_count / 7;
}

// Deallocates array containing measurements
void deallocate_arrays(char ***items_global, int *item_count_global) {
    for (int i = 0; i < *item_count_global * 6; i++) {
        free((*items_global)[i]);
    }
    free(*items_global);

    *items_global = NULL;
    *item_count_global = 0;
}

// Allocates arrays dynamically   
void allocate_arrays(FILE **fp, char ***items_global, int *item_count_global) {
    if (*fp == NULL) {
        printf("Neotvoreny subor\n");
        return;
    }

    if (*items_global != NULL) {
        deallocate_arrays(items_global, item_count_global);
    }

    int item_count = count_items(*fp);
    fseek(*fp, 0, SEEK_SET);

    // Allocate main array - measurements (without \n)
    char **items = (char**)malloc(sizeof(char*) * (item_count * 6));

    if (items == NULL) {
        printf("Nepodarilo sa alokovat");
        exit(1);
    }

    int item_idx = 0;
    for (int i = 0; i < item_count * 7; i++) {
        // Newline every 6th line
        if ((i + 1) % 7 == 0) {
            char throwaway[30];
            fgets(throwaway, sizeof(throwaway), *fp);
            continue;
        }

        char buff[30];
        fgets(buff, sizeof(buff), *fp);

        // Allocate memory for measurement attribute
        items[item_idx] = malloc(sizeof(char) * strlen(buff) + 1);
        if (items[item_idx] == NULL) {
            printf("Nepodarilo sa alokovat");
            exit(1);
        }
        strcpy(items[ item_idx], buff);
        items[item_idx][strlen(items[ item_idx]) - 1] = '\0';
        item_idx++;
    }

    *items_global = items;
    *item_count_global = item_count;
}

// Calculate difference in months
int month_difference(char *measurement_date, char *ciache_date) {
    int measurement_year, measurement_month, measurement_day;
    int ciache_year, ciache_month, ciache_day;

    sscanf(measurement_date, "%4d%2d%2d", &measurement_year, &measurement_month, &measurement_day);
    sscanf(ciache_date, "%4d%2d%2d", &ciache_year, &ciache_month, &ciache_day);
    return (measurement_year - ciache_year) * 12 + (measurement_month - ciache_month) + ((measurement_day - ciache_day) < 0 ? -1 : 0);
}

// Get message about modules ciached more than Y months ago.
void ciachovanie(char **items_global, int item_count_global) {
    int Y;

    if (items_global == NULL) {
        printf("Polia nealokovane\n");
        return;
    }

    printf("Pocet mesiacov: ");
    scanf("%d", &Y);
    FILE *fp = fopen("ciachovanie.txt", "r");
    if (fp == NULL) {
        printf("Subor ciachovanie.txt sa nepodarilo otvorit\n");
        return;
    };

    char id_ciached[20];
    char date_ciached[20];
    char blank_line[20];

    for (int i = 0; i < item_count_global; i++) {
        fseek(fp, 0, SEEK_SET);

        int is_ciached = 0;
        while (fgets(id_ciached, sizeof(id_ciached) - 1, fp) != NULL) {
            fgets(date_ciached, sizeof(date_ciached) - 1, fp);
            fgets(blank_line, sizeof(blank_line) - 1, fp);
            id_ciached[strlen(id_ciached) - 1] = '\0';


            // // Ak sa ID rovnaju
            if (strcmp(items_global[i * 6], id_ciached) == 0) {
                is_ciached = 1;
                int month_diff = month_difference(items_global[i * 6 + 5], date_ciached);
                if (month_diff > Y) {
                    printf("ID. mer. modulu [%s] ma %d mesiacov po ciachovani\n", items_global[i * 6], month_diff);
                }
            }
        }

        if (!is_ciached) {
            printf("ID. mer. modulu [%s] nie je ciachovany.\n", items_global[i * 6]);
        }
    }
    fclose(fp);
}

// Get difference in 2 times
int measurement_time_diff(char *a, char *b) {
    return atof(a) - atof(b);
}

// Export sorted measurements for specified module and unit to file
void export_measurements(char **items_global, int items_count_global) {
    char module_id[6];
    char unit[3];

    if (items_global == NULL) {
        printf("Polia nie su vytvorene\n");
        return;
    }

    scanf("%s %s", module_id, unit);

    
    int size = 10;
    int idx = 0;
    // To store indexes to values
    int *measurements = malloc(size * sizeof(int));
    if (measurements == NULL) {
        printf("Couldn't allocate memory\n");
        return;
    }
    
    // Get measurements of specified module and unit
    for (int i = 0; i < items_count_global; i++) {
        if (strcmp(items_global[i * 6], module_id) == 0 && strcmp(items_global[i * 6 + 2], unit) == 0) {
            measurements[idx] = i;
            idx++;

            if (size == idx) {
                size += 10;
                measurements = realloc(measurements, size * sizeof(int));
                if (measurements == NULL) {
                    printf("Couldn't reallocate memory\n");
                    return;
                }
            }
        }
    }

    if (idx == 0) {
        printf("Pre dany vstup neexistuju zaznamy.\n");
        free(measurements);
        return;
    }

    // Sort the array - bubble sort
    for (int i = 0; i < idx; i++) {
        for (int j = i; j < idx - 1; j++) {
            char a_datetime[20];
            char *a_time = items_global[measurements[j] * 6 + 4];
            char *a_date = items_global[measurements[j] * 6 + 5];
            char b_datetime[20];
            char *b_time = items_global[measurements[j + 1] * 6 + 4];
            char *b_date = items_global[measurements[j + 1] * 6 + 5];

            strcpy(a_datetime, a_date);
            strcat(a_datetime, a_time);
            strcpy(b_datetime, b_date);
            strcat(b_datetime, b_time);

            if (measurement_time_diff(a_datetime, b_datetime) > 0) {
                int temp = measurements[j];
                measurements[j] = measurements[j + 1];
                measurements[j + 1] = temp;
            }
        }
    }

    FILE *fp = fopen("vystup_S.txt", "w");
    if (fp == NULL) {
        printf("Pre dany vstup nie je vytvoreny txt subor");
        free(measurements);
        return;
    }

    for (int i = 0; i < idx; i++) {
        char *date = items_global[measurements[i] * 6 + 5];
        char *time = items_global[measurements[i] * 6 + 4];
        double value = atof(items_global[measurements[i] * 6 + 3]);
        char *latitude_longitude = items_global[measurements[i] * 6 + 1];
        char latitude[10], longitude[10];
        strncpy(latitude, latitude_longitude, 7);
        strcpy(longitude, latitude_longitude + 7);
        double latitude_num = atof(latitude) / 10000;
        double longitude_num = atof(longitude) / 10000;

        char lat_sign = latitude_num < 0 ? '-' : '+';
        char lon_sign = longitude_num < 0 ? '-' : '+';

        fprintf(fp, "%s%s\t%.5lf\t%c%.4lf\t%c%.4lf\n", date, time, value, lat_sign, latitude_num, lon_sign, longitude_num);
    }

    fclose(fp);
    free(measurements);
}

// Displays histogram of measured units
void show_histogram(char **items_global, int item_count_global) {
    if (items_global == NULL) {
        printf("Polia nie su vytvorene\n");
        return;
    }

    int size = 5;
    int idx = 0;

    int *values = malloc(5 * sizeof(int));
    if (values == NULL) {
        printf("Couldn't allocate array");
        return;
    }

    int *values_count = malloc(5 * sizeof(int));
    if (values_count == NULL) {
        free(values);
        printf("Couldn't allocate array");
        return;
    }

    double *values_min = malloc(5 * sizeof(double));
    if (values_min == NULL) {
        free(values);
        free(values_count);
        printf("Couldn't allocate array");
        return;
    }
    double *values_max = malloc(5 * sizeof(double));
    if (values_min == NULL) {
        free(values);
        free(values_count);
        free(values_min);
        printf("Couldn't allocate array");
        return;
    }

    for (int i = 0; i < item_count_global; i++) {
        char *value = items_global[i * 6 + 2];

        int tracked = 0;
        for (int j = 0; j < idx; j++) {
            if (strcmp(value, items_global[values[j] * 6 + 2]) == 0) {
                tracked = 1;
                values_count[j] += 1;
                double curr_value = atof(items_global[i * 6 + 3]);
                if (curr_value < values_min[j]) {
                    values_min[j] = curr_value;
                }
                if (curr_value > values_max[j]) {
                    values_max[j] = curr_value;
                }
            }
        }

        // If the value isn't tracked, start tracking it
        if (!tracked) {
            values[idx] = i;
            values_min[idx] = atof(items_global[i * 6 + 3]);
            values_max[idx] = atof(items_global[i * 6 + 3]);
            values_count[idx] = 1;

            idx++;
        }

        if (idx == size) {
            size = size + 5;

            values = realloc(values, size * sizeof(int));
            if (values == NULL) {
                printf("Couldn't reallocate array");
                return;
            }
            values_count = realloc(values_count, size * sizeof(int));
            if (values_count == NULL) {
                printf("Couldn't reallocate array");
                return;
            }
            values_min = realloc(values_min, size * sizeof(double));
            if (values_min == NULL) {
                printf("Couldn't reallocate array");
                return;
            }
            values_max = realloc(values_max, size * sizeof(double));
            if (values_max == NULL) {
                printf("Couldn't reallocate array");
                return;
            }
        }
    }

    for (int i = 0; i < idx; i++) {
        printf("%s\t%d\t%.2lf\t%.2lf\n", items_global[values[i] * 6 + 2], values_count[i], values_min[i], values_max[i]);
    }
}

// Delete measurements of specified module
void delete_measurements(char ***items_global, int *item_count_global) {
    if ((*items_global) == NULL) {
        printf("Polia nie su vytvorene\n");
        return;
    }

    char module_id[10];
    scanf("%s", module_id);

    int *items_to_delete = malloc((*item_count_global) * sizeof(int));
    if (items_to_delete == NULL) {
        printf("Couldn't allocate memory");
        return;
    }
    int new_item_count = 0;

    for (int i = 0; i < *item_count_global; i++) {
        if (strcmp((*items_global)[i * 6], module_id) == 0) {
            items_to_delete[i] = 0;
        } else {
            items_to_delete[i] = 1;
            new_item_count++;
        }
    }

    char **new_items_global = malloc(sizeof(char*) * 6 * new_item_count);
    if (new_items_global == NULL) {
        printf("Couldn't reallocate memory");
        return;
    }

    int idx = 0;
    for (int i = 0; i < (*item_count_global); i++) {
        if (!items_to_delete[i]) {
            continue;
        }

        for (int j = 0; j < 6; j++) {
            char *str = malloc(sizeof(char) * (strlen((*items_global)[i * 6 + j]) + 1));
            if (str == NULL) {
                printf("Couldn't allocate memory");
                return;
            }
            strcpy(str, (*items_global)[i * 6 + j]);
            new_items_global[idx * 6 + j] = str;
        }
        idx++;
    }

    int item_count_prev = *item_count_global;
    // Deallocate previous arrays
    deallocate_arrays(items_global, item_count_global);
    *items_global = new_items_global;
    *item_count_global = new_item_count;

    printf("Vymazalo sa: %d zaznamov!\n", item_count_prev - new_item_count);
}

// Close file and deallocate arrays
void clean_and_exit(FILE **fp, char ***items_global, int *item_count_global) {
    if (*fp != NULL) {
        fclose(*fp);
        *fp = NULL;
    }
    deallocate_arrays(items_global, item_count_global);
}


int main(void) {
    char command;
    FILE *fp = NULL;
    char **items = NULL;
    int item_count = 0;
    int running = 1;

    // Hlavny cyklus na zadavanie prikazov
    while ((command = getchar()) && running) {
        switch(command) {
            case 'v':
                open_file(&fp, items, item_count);
                break;
            case 'n':
                allocate_arrays(&fp, &items, &item_count);
                break;
            case 'c':
                ciachovanie(items, item_count);
                break;
            case 's':
                export_measurements(items, item_count);
                break;
            case 'h':
                show_histogram(items, item_count);
                break;
            case 'z':
                delete_measurements(&items, &item_count);
                break;
            case 'k':
                clean_and_exit(&fp, &items, &item_count);
                running = 0;
                break;
        }
    }    
    return 0;
}