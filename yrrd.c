// yrrd.c - Word Frequency Utility
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define HASH_SIZE 1024

// Data structure for the Hash Table (Separate Chaining)
typedef struct node {
    char *word;
    int count;
    struct node *next;
} Node;

Node *hash_table[HASH_SIZE] = {NULL};

// Memory management: Iterate through all buckets and free linked list nodes
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

// Tokenizes a comma-separated string to check if a word matches any target words
int is_in_list(char *list, char *word) {
    char *list_copy = strdup(list);
    char *token = strtok(list_copy, ", ");
    while (token != NULL) {
        if (strcmp(token, word) == 0) {
            free(list_copy);
            return 1;
        }
        token = strtok(NULL, ", ");
    }
    free(list_copy);
    return 0;
}

// Simple hash function using bitwise shifting to distribute words across the table
unsigned int hash(char *str) {
    unsigned int h = 0;
    while (*str) h = (h << 5) + *str++;
    return h % HASH_SIZE;
}

// Updates word counts in the hash table or adds a new node if the word is new
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

void print_help() {
    printf("YRRD Word Frequency Utility\nUsage: ./yrrd <file.txt> [-w word1, word2, ...]\n");
}

int main(int argc, char *argv[]) {
    // Basic argument validation
    if (argc < 2) {
        printf("Error: Missing arguments. Use -help for usage.\n");
        return 1;
    }
    if (strcmp(argv[1], "-help") == 0) {
        print_help();
        return 0;
    }

    char *filename = NULL;
    char *target_words = NULL;

    // Handle optional word filtering argument
    if (argc >= 4 && strcmp(argv[1], "-w") == 0) {
        target_words = argv[2];
        filename = argv[3];
    } else {
        filename = argv[1];
    }

    // Open file and map it into memory for high-performance read access
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return 1;
    }
    
    struct stat st;
    fstat(fd, &st);
    char *mapped = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        return 1;
    }

    // Fork: Separate the counting/file processing from the sorting/output logic
    pid_t pid = fork();
    if (pid == 0) {
        // Child Process: Count tokens from the memory-mapped file
        char *text = strdup(mapped);
        char *word = strtok(text, " \n\t.,");
        
        while (word != NULL) {
            if (target_words == NULL || is_in_list(target_words, word)) {
                increment_word(word);
            }
            word = strtok(NULL, " \n\t.,");
        }
        
        // Write the resulting frequency map to a file for the parent to process
        FILE *f = fopen("results.txt", "w");
        for (int i = 0; i < HASH_SIZE; i++) {
            Node *entry = hash_table[i];
            while (entry) {
                fprintf(f, "%-10d %s\n", entry->count, entry->word);
                entry = entry->next;
            }
        }
        fclose(f); 
        
        free(text);
        free_table();
        exit(1);
    } else {
        // Parent Process: Wait for child, then sort and display output
        wait(NULL);
        // Use system sort utility to sort results by frequency descending
        system("sort -rn results.txt -o sorted_results.txt");
        
        printf("%-10s %s\n", "COUNT", "WORD");
        printf("--------------------\n");
        
        // Output the final results from the sorted file
        FILE *f = fopen("results.txt", "r");
        if (f) {
            char line[256];
            while (fgets(line, sizeof(line), f)) {
                printf("%s", line);
            }
            fclose(f);
        }

        // Cleanup resources
        munmap(mapped, st.st_size);
        close(fd);
    }
    return 0;
}
