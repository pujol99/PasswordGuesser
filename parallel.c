#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NTHREADS 4

struct args_struct{
    char** words1;
    char** words2;
    char sep[1];
    char* password;
    clock_t begin;
};

void end(clock_t begin)
{
    /*Prints total time given start*/
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Time: %.2f\n", time_spent);
    exit(0);
}

char** loadInfo(char* path){
    /*loads content from file into list of words*/
    int index = 0;
    char** info = (char **)malloc(sizeof(char *));
    char line[50];
    
    FILE *f = fopen(path, "r");
	while(fgets(line, sizeof(line), f)){
        info[index] = malloc(sizeof(char)*50);
        strcpy(info[index], line);
		if(info[index][strlen(info[index])-1] == '\n'){
			info[index][strlen(info[index])-1] = '\0';
		}
        index++;
        info = (char **)realloc(info, sizeof(char *) * (index+1)); 
    }
    fclose(f);

	return info;
}

int compare(char* word, char* password){
    /*Compare our guess with the real password*/
    if (strcmp(word, password) == 0){
        printf("\nFound! (%s)\n", password);
        return 1;
    }
    return 0;
}

int compare_all(char** words, char* password){
    int i;
    for (i = 0; i < _msize(words) / 8 - 1; i++){
        if (compare(words[i], password)){
            return 1;
        }
    }return 0;
}

char** build_words(char** words, char** extras, char sep[1], char* password, clock_t begin){
    int i, j, k = 0;
    char **result = (char **)malloc(sizeof(char *));

    for (i = 0; i < _msize(words) / 8 - 1; i++){
        for (j = 0; j < _msize(extras) / 8 - 1; j++){
            result[k] = malloc(sizeof(char) * 50);
            strcpy(result[k], words[i]);
            strcat(result[k], sep);
            strcat(result[k], extras[j]);
            if(compare(result[k], password)) end(begin);
            k++;
            result = (char **)realloc(result, sizeof(char *) * (k + 1));
        }
    }return result;
}

char** expand(char** words){
    int i, c, size = _msize(words) / 8 - 1, k = size;
    char** result = (char **)realloc(words, sizeof(char *) * (k + 1));

    for (i = 0; i < size; i++){
        //Capitalize first
        result[k] = malloc(sizeof(char) * 50);
        strcpy(result[k], result[i]);
        result[k][0] = toupper(result[k][0]);
        k++;
        result = (char **)realloc(result, sizeof(char *) * (k + 1));
    }

    for (i = 0; i < size; i++){
        //Capitalize all
        result[k] = malloc(sizeof(char) * 50);
        strcpy(result[k], result[i]);
        strrev(result[k]);
        k++;
        result = (char **)realloc(result, sizeof(char *) * (k + 1));
    }

    return result;
}

void print_word_list(char** words){
    int i, size = _msize(words) / 8 - 1;
    for (i = 0; i < size; i++){
        printf("-> %s\n", words[i]);
    }
}

void* build_words_thread(void *arguments)
{
    struct args_struct *args = (struct args_struct *)arguments;
    char **all, **new_all;
    all = build_words((args)->words1, (args)->words2, (args)->sep, (args)->password, args->begin);

    new_all = build_words(all, (args)->words2, "-", (args)->password, args->begin);
    new_all = build_words(all, (args)->words2, ".", (args)->password, args->begin);
    new_all = build_words(all, (args)->words2, "_", (args)->password, args->begin);
    return NULL;
}

int main(int argc, char* argv[]){
    char **common, **extras, **victim, **all, **new_all;
    char* password;
    char* fast;
    unsigned int i;
    struct args_struct args1;
    struct args_struct args2;
    struct args_struct args3;
    struct args_struct args4;
    pthread_t threads[NTHREADS];
    clock_t begin;begin = clock();

    if(argc < 2){
        printf("\tinput password please");
        return 0;
    }else{
        password = malloc(sizeof(char) * strlen(argv[1]));
        strcpy(password, argv[1]);
    }
    begin = clock();

    common = loadInfo("files/mostCommon.txt");
    extras = loadInfo("files/extras.txt");
    victim = loadInfo("files/victim.txt");
    
    

    if(compare_all(common, password)) end(begin);
    if(compare_all(victim, password)) end(begin);

    victim = expand(victim);
    
    (args1).words1 = victim;
    (args1).words2 = extras;
    (args1).password = password;
    (args2).words1 = victim;
    (args2).words2 = extras;
    (args2).password = password;
    (args3).words1 = victim;
    (args3).words2 = extras;
    (args3).password = password;
    (args4).words1 = victim;
    (args4).words2 = extras;
    (args4).password = password;
    args1.begin = begin;
    args2.begin = begin;
    args3.begin = begin;
    args4.begin = begin;

    if(argc < 3){
        //Parallel mode
        strcpy(args1.sep, "");
        pthread_create(&threads[0], NULL, build_words_thread, (void *)&args1);

        strcpy(args2.sep, ".");
        pthread_create(&threads[1], NULL, build_words_thread, (void *)&args2);

        strcpy(args3.sep, "-");
        pthread_create(&threads[2], NULL, build_words_thread, (void *)&args3);

        strcpy(args4.sep, "_");
        pthread_create(&threads[3], NULL, build_words_thread, (void *)&args4); 

        for (i = 0; i < NTHREADS; i++)
            pthread_join(threads[i], NULL);
    }else{
        printf("Concurrent mode\n");

        all = build_words(victim, extras, "", password, begin);

        new_all = build_words(all, extras, "-", password, begin);
        new_all = build_words(all, extras, ".", password, begin);
        new_all = build_words(all, extras, "_", password, begin);

        all = build_words(victim, extras, ".", password, begin);

        new_all = build_words(all, extras, "-", password, begin);
        new_all = build_words(all, extras, ".", password, begin);
        new_all = build_words(all, extras, "_", password, begin);

        all = build_words(victim, extras, "-", password, begin);

        new_all = build_words(all, extras, "-", password, begin);
        new_all = build_words(all, extras, ".", password, begin);
        new_all = build_words(all, extras, "_", password, begin);

        all = build_words(victim, extras, "_", password, begin);

        new_all = build_words(all, extras, "-", password, begin);
        new_all = build_words(all, extras, ".", password, begin);
        new_all = build_words(all, extras, "_", password, begin);
    }

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Time: %.2f\n", time_spent);
    exit(0);
    
    free(all);
    free(common);
    free(victim);
    free(extras);

    

    return 0;
}
