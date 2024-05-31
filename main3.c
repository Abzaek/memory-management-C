#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 65536  // Define the total memory size

// Define structures for free blocks, processes, and allocated blocks
typedef struct FreeBlock {
    int start;                 // Start address of the free block
    int size;                  // Size of the free block
    struct FreeBlock* next;    // Pointer to the next free block
} FreeBlock;

typedef struct Process {
    int process_id;            // ID of the process
    struct AllocatedBlock* allocated_blocks;  // Pointer to the list of allocated blocks
    struct Process* next;      // Pointer to the next process
} Process;

typedef struct AllocatedBlock {
    int start;                 // Start address of the allocated block
    int size;                  // Size of the allocated block
    struct AllocatedBlock* next;  // Pointer to the next allocated block
} AllocatedBlock;

// Global pointers to the free list and process list
FreeBlock* free_list = NULL;
Process* process_list = NULL;

// Initialize the free list with a single large block// Function to initialize the free list with a single large block
FreeBlock* initialize_free_list() {
    // Allocate memory for the head of the free list
    FreeBlock* head = (FreeBlock*)malloc(sizeof(FreeBlock));
    // Set the start address of the head block to 0
    head->start = 0;
    // Set the size of the head block to the total memory size
    head->size = MEMORY_SIZE;
    // Set the next pointer of the head block to NULL since it's the only block in the list
    head->next = NULL;
    // Return a pointer to the head of the free list
    return head;
}


// Insert a new free block into the free list in sorted order by address
void insert_free_block_sorted(FreeBlock* new_block) {
    // Check if the free list is empty or if the new block should be inserted before the current head
    if (!free_list || new_block->start < free_list->start) {
        // Insert the new block at the beginning of the list
        new_block->next = free_list;
        free_list = new_block;
    } else {
        // Traverse the list to find the correct position to insert the new block
        FreeBlock* current = free_list;
        while (current->next && current->next->start < new_block->start) {
            current = current->next;
        }
        // Insert the new block after the current block
        new_block->next = current->next;
        current->next = new_block;
    }
}

// Function to merge adjacent free blocks in the free list
void merge_free_blocks() {
    // Start from the beginning of the free list
    FreeBlock* current = free_list;
    // Traverse the list while there are at least two consecutive free blocks
    while (current && current->next) {
        // Check if the current free block and its next block are adjacent
        if (current->start + current->size == current->next->start) {
            // Merge the current and next free blocks into a single block
            FreeBlock* temp = current->next;
            current->size += temp->size;  // Increase the size of the current block
            current->next = temp->next;   // Update the next pointer of the current block
            free(temp);                   // Free the memory occupied by the next block
        } else {
            // Move to the next free block
            current = current->next;
        }
    }
}


// Allocate memory to a process
int allocate_memory(int process_id, int size) {

    FreeBlock* current = free_list;
    FreeBlock* prev = NULL;


    Process* curr = process_list;

    while (curr) {
        if (curr->process_id  == process_id) {
            break;
        }
        curr = curr->next;
    }

    //checks wether process is found
    if (!curr) {
        return -1;
    }

    // Find a free block that can satisfy the allocation request
    while (current != NULL) {
        if (current->size >= size) {
            int start_address = current->start;
            current->start += size;
            current->size -= size;

            // Remove the free block if it's completely used
            if (current->size == 0) {
                if (prev == NULL) {
                    free_list = current->next;
                } else {
                    prev->next = current->next;
                }
                free(current);
            }

            // Find or create the process in the process list
            Process* process = process_list;
            while (process && process->process_id != process_id) {
                process = process->next;
            }
            if (!process) {
                process = (Process*)malloc(sizeof(Process));
                process->process_id = process_id;
                process->allocated_blocks = NULL;
                process->next = process_list;
                process_list = process;
            }

            // Add the allocated block to the process's allocated blocks list
            AllocatedBlock* new_block = (AllocatedBlock*)malloc(sizeof(AllocatedBlock));
            new_block->start = start_address;
            new_block->size = size;
            new_block->next = process->allocated_blocks;
            process->allocated_blocks = new_block;

            return start_address;  // Return the start address of the allocated block
        }

        prev = current;
        current = current->next;
    }

    return -1;  // Allocation failed
}

// Free memory allocated to a process
void free_memory(int process_id, int address) {
    Process* process = process_list;
    Process* prev_process = NULL;

    // Find the process in the process list
    while (process && process->process_id != process_id) {
        prev_process = process;
        process = process->next;
    }

    if (!process) {
        printf("Process %d not found.\n", process_id);
        return;
    }

    // Find the allocated block in the process's allocated blocks list
    AllocatedBlock* block = process->allocated_blocks;
    AllocatedBlock* prev_block = NULL;

    while (block && block->start != address) {
        prev_block = block;
        block = block->next;
    }

    if (!block) {
        printf("Address %d not allocated to process %d.\n", address, process_id);
        return;
    }

    // Remove the allocated block from the process's allocated blocks list
    if (prev_block) {
        prev_block->next = block->next;
    } else {
        process->allocated_blocks = block->next;
    }

    // Create a new free block and insert it into the free list
    FreeBlock* new_free_block = (FreeBlock*)malloc(sizeof(FreeBlock));
    new_free_block->start = address;
    new_free_block->size = block->size;
    insert_free_block_sorted(new_free_block);

    free(block);

    // Merge adjacent free blocks
    merge_free_blocks();

    // Remove the process from the process list if it has no more allocated blocks
    if (!process->allocated_blocks) {
        if (prev_process) {
            prev_process->next = process->next;
        } else {
            process_list = process->next;
        }
        free(process);
    }
}

// Display the current memory status
void show_memory() {
    printf("Memory Status:\n");
    printf("---------------------------------------------------\n");
    printf("| Start Address | End Address   | Status          |\n");
    printf("---------------------------------------------------\n");

    int current_address = 0;

    FreeBlock* free_current = free_list;
    Process* process_current = process_list;


    // Iterate through memory and display status of each block
    while (current_address < MEMORY_SIZE) {
        int found = 0;

        if (free_current && free_current->start == current_address) {
            printf("| %12d | %12d | Free             |\n", free_current->start, free_current->start + free_current->size - 1);
            current_address += free_current->size;
            free_current = free_current->next;
            found = 1;
        } else {
            Process* process = process_list;
            while (process) {
                AllocatedBlock* block = process->allocated_blocks;
                while (block) {
                    if (block->start == current_address) {
                        printf("| %12d | %12d | Process %5d    |\n", block->start, block->start + block->size - 1, process->process_id);
                        current_address += block->size;
                        block = NULL;
                        found = 1;
                    } else {
                        block = block->next;
                    }
                }
                process = process->next;
            }
        }

        if (!found) {
            current_address++;
        }
    }

    printf("---------------------------------------------------\n");
}


// Display the help message with available commands
void show_help() {
    printf("Available commands:\n");
    printf("create <process_id> - Creates a new process with the given ID\n");
    printf("terminate <process_id> - Terminates the process with the given ID and frees all its memory\n");
    printf("allocate <process_id> <size> - Allocates memory of the given size for the specified process\n");
    printf("free <process_id> <address> - Frees the memory block starting at the specified address for the given process\n");
    printf("show memory - Displays the current memory status\n");
    printf("exit - Exits the program\n");
    printf("help - Shows this help message\n");
}




// Parse and execute commands from the user
void parse_command(char* command) {
    char* token = strtok(command, " ");
    if (strcmp(token, "create") == 0) {
        int process_id = atoi(strtok(NULL, " "));
        Process* new_process = (Process*)malloc(sizeof(Process));
        new_process->process_id = process_id;
        new_process->allocated_blocks = NULL;
        new_process->next = process_list;
        process_list = new_process;
        printf("Process %d created.\n", process_id);
    } else if (strcmp(token, "terminate") == 0) {
        int process_id = atoi(strtok(NULL, " "));
        free_memory(process_id, 0);  // Free all memory allocated to the process
        printf("Process %d terminated.\n", process_id);
    } else if (strcmp(token, "allocate") == 0) {
        int process_id = atoi(strtok(NULL, " "));
        int size = atoi(strtok(NULL, " "));
        int address = allocate_memory(process_id, size);
        if (address != -1) {
            printf("Allocated %d bytes to process %d at address %d.\n", size, process_id, address);
        } else {
            printf("Allocation failed for process %d.\n", process_id);
        }
    } else if (strcmp(token, "free") == 0) {
        int process_id = atoi(strtok(NULL, " "));
        int address = atoi(strtok(NULL, " "));
        free_memory(process_id, address);
        printf("Freed memory at address %d for process %d.\n", address, process_id);
    } else if (strcmp(token, "show") == 0) {
        token = strtok(NULL, " ");
        if (strcmp(token, "memory") == 0) {
            show_memory();
        } else if (strcmp(token, "free") == 0) {
            // Implement show free list if needed
        }
    } else if (strcmp(token, "help") == 0) {
        show_help();
    } else {
        printf("Unknown command. Type 'help' for a list of available commands.\n");
    }
}

int main() {
    free_list = initialize_free_list();  // Initialize the free list
    char command[256];

    // Main loop to accept and execute commands from the user
    while (1) {
        printf("enter your command > ");
        if (fgets(command, sizeof(command), stdin) == NULL) break;
        command[strcspn(command, "\n")] = '\0';  // Remove newline character
        if (strcmp(command, "exit") == 0) break;
        parse_command(command);
    }

    return 0;
}
