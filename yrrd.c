// yrrd.c - Word Frequency Utility
// When trying to run follow the steps:
// 1. type: make yrrd
// 2. type: ./yrrd <file> - replace the file with the name of the file you want to run ex: Makefile
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define HASH_SIZE 1024
#define MAX_TARGETS 64

// Structure to store unique words and their occurrence counts
typedef struct node {
    char *word;
    int count;
    struct node *next;
} Node;

Node *hash_table[HASH_SIZE] = {NULL};

// Frees all nodes in the hash table to prevent memory leaks
void free_table() {
    for (int i = 0; i < HASH_SIZE; i++) {
        Node *entry = hash_table[i];
        while (entry) {
            Node *temp = entry;
            entry = entry->next;
            free(temp->word);
            free(temp);
        }
    }
}

// DJB2 hash algorithm to distribute words across the hash table
unsigned int hash(char *str) {
    unsigned int h = 0;
    while (*str) h = (h << 5) + *str++;
    return h % HASH_SIZE;
}

// Adds a word to the hash table or increments its count if it already exists
void increment_word(char *word) {
    unsigned int index = hash(word);
    Node *entry = hash_table[index];
    while (entry != NULL) {
        if (strcmp(entry->word, word) == 0) {
            entry->count++;
            return;
        }
        entry = entry->next;
    }
    Node *new_node = malloc(sizeof(Node));
    new_node->word = strdup(word);
    new_node->count = 1;
    new_node->next = hash_table[index];
    hash_table[index] = new_node;
}

// Displays program usage documentation
void print_help() {
    printf("Usage: ./yrrd <file> [-w word1 word2 ...]\n\n");
    printf("Options:\n");
    printf("  <file>          The path to the text file.\n");
    printf("  -w <words>      Specify words to count.\n");
    printf("  -help           Display this documentation.\n");
}

// Logic for the child worker process (triggered via execl)
void run_worker(int argc, char *argv[]) {
    // argv[2] is the filename; subsequent arguments are the words to filter
    char *filename = argv[2];
    
    // Open the file using system call 'open'
    int fd = open(filename, O_RDONLY);
    if (fd < 0) exit(1);
    
    // Get file size using system call 'fstat'
    struct stat st;
    fstat(fd, &st);
    
    // Map the file into memory using 'mmap' for efficient reading
    char *mapped = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    
    // Create a mutable copy of the file content for tokenization
    char *text = strdup(mapped);
    char *word = strtok(text, " \n\t.,");
    
    while (word != NULL) {
        // If argc <= 3, no target words provided; count everything
        int should_count = (argc <= 3); 
        // Otherwise, check if current word exists in the provided target list
        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], word) == 0) {
                should_count = 1;
                break;
            }
        }
        
        if (should_count) increment_word(word);
        word = strtok(NULL, " \n\t.,");
    }
    
    // Write the results to a file for the parent to display
    FILE *f = fopen("results.txt", "w");
    for (int i = 0; i < HASH_SIZE; i++) {
        Node *entry = hash_table[i];
        while (entry) {
            fprintf(f, "%-10d %s\n", entry->count, entry->word);
            entry = entry->next;
        }
    }
    fclose(f);
    
    // Clean up memory and file descriptors using 'munmap' and 'close'
    munmap(mapped, st.st_size);
    close(fd);
    free(text);
    free_table();
    exit(0);
}

int main(int argc, char *argv[]) {
    // Handle the -help flag
    if (argc > 1 && strcmp(argv[1], "-help") == 0) {
        print_help();
        return 0;
    }

    // Check if running in worker mode (invoked by execl)
    if (argc > 1 && strcmp(argv[1], "--worker") == 0) {
        run_worker(argc, argv);
        return 0;
    }

    if (argc < 2) {
        printf("Error: Missing filename. Use -help.\n");
        return 1;
    }

    // Create a new process
    pid_t pid = fork();
    if (pid == 0) {
        // --- CHILD PROCESS ---
        // Prepare argument list for execl/execvp
        char *exec_args[MAX_TARGETS + 5];
        exec_args[0] = "yrrd";
        exec_args[1] = "--worker";
        int j = 2;
        // Copy arguments, skipping the "-w" flag
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-w") != 0) {
                exec_args[j++] = argv[i];
            }
        }
        exec_args[j] = NULL;
        
        // Execute the program in worker mode
        execvp("./yrrd", exec_args);
        exit(1);
    } else {
        // --- PARENT PROCESS ---
        // Wait for child to complete file processing
        wait(NULL);
        
        printf("%-10s %s\n", "COUNT", "WORD");
        printf("--------------------\n");
        
        // Open results written by the child and display them to the user
        FILE *f = fopen("results.txt", "r");
        if (f) {
            char line[256];
            while (fgets(line, sizeof(line), f)) printf("%s", line);
            fclose(f);
        }
    }
    return 0;
}
