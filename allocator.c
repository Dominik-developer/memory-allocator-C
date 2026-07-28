#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define HEAP_SIZE 4096 // pamiec RAM, statyczna tablica 4 KB
#define ALIGNMENT 8 

static uint8_t heap[HEAP_SIZE];

typedef struct Header {
    size_t size;
    bool is_free;
    struct Header* next;
} Header;

static Header *head;

size_t align(size_t size) {

    if(size == 0 ) {
        return 0;
    }
    
    if (size % ALIGNMENT == 0) {
        return size;
    } else {
        return ((size / ALIGNMENT) + 1 ) * ALIGNMENT;
    }
}

void heap_init(void) {

    head = (Header *)heap;
    head->size = HEAP_SIZE - sizeof(Header);
    head->is_free = true;
    head->next = NULL;    
}

void dump_heap(void) {

    Header *current = head;

    size_t user_mem = 0;
    size_t free_mem = 0;
    size_t meta_mem = 0;
    size_t blocks = 0;

    while(current != NULL) {
        if(current->is_free) {
            free_mem += current->size;
        } else {
            user_mem += current->size;
        }
        blocks ++;
        meta_mem += sizeof(Header);

        current = current->next;
    }

    printf("\n========================= HEAP STATUS =========================\n\n");

    printf("Total memory : %d B \n", HEAP_SIZE);
    printf("User payload : %zu B \n", user_mem);
    printf("Metadata     : %zu B \n", meta_mem);
    printf("Free memory  : %zu B \n", free_mem);
    printf("Blocks       : %zu \n", blocks);

    printf("\n===============================================================\n\n");

    printf("+--------------+--------+--------+--------------+--------------+\n");
    printf("| Address      | Size   | Free   | Payload      | Next         |\n");
    printf("+--------------+--------+--------+--------------+--------------+\n");

    current = head;

    while(current != NULL) {

        char *s_yn; // pointer do tekstu 
        if(current->is_free) {
            s_yn = "YES";
        } else {
            s_yn = "NO";
        }

        uint8_t *payload_adr = (uint8_t*)current + sizeof(Header); 

        printf("| %-12p | %-6zu | %-6s | %-12p | %-12p |\n", (void *)current, current->size, s_yn, (void *)payload_adr, (void *)current->next);

        current = current->next;
    }
    printf("+--------------+--------+--------+--------------+--------------+\n");

    printf("\n===============================================================\n");
}

void *heap_malloc(size_t size) {

    if(size > 0) {
        size_t real_needed_size = align(size);
        Header *current = head;

        while(current != NULL) {
            if(current->is_free && current->size >= real_needed_size) {

                size_t free_size = current->size;

                void *payload = (uint8_t *)current + sizeof(Header);

                if(free_size > (real_needed_size + (sizeof(Header)) + ALIGNMENT)) {
                    current->size = real_needed_size;

                    Header *new_header = (Header *)((uint8_t *)payload + real_needed_size); // adress nowego Header
                    // (Header *) od tego miejsca traktuj to jako strukturę Header
                    // uint8_t ma rozmiar bajt
                    // (uint8_t *)payload + NUMBER -> Przesuń adres o NUMBER elementw typu uint8_t

                    new_header->is_free = true;
                    new_header->size = free_size - real_needed_size - sizeof(Header);
                    new_header->next = current->next;
                    current->next = new_header;
                }
                current->is_free = false;

                return payload;
            }
        current = current->next;
        }
    }
    return NULL;
}

void heap_free(void *ptr) {

    if(ptr == NULL) {
        return;
    }

    uintptr_t address = (uintptr_t)ptr; // rzutowanie zmiennej: int x; float y = (float)x;

    if(address < (uintptr_t)heap || address >= (uintptr_t)(heap + HEAP_SIZE)) {
        printf("Invalid pointer\n");
        return;
    }

    Header *current = head;

    while(current != NULL) {

        void *payload = (uint8_t *)current + sizeof(Header);

        if(ptr == payload) {
            if(current->is_free) {
                printf("Block already free\n");
                return;
            }
            current->is_free = true;

            Header *next = current->next;

            if(current->next != NULL && current->next->is_free) { // sprawdzenie czy nastepny blok pamieci jest wolny i scalenie jelsi jest
                current->size += next->size + sizeof(Header);
                current->next = next->next;
            } 

            Header *prev = NULL;
            Header *iter = head;

            while(iter != current) {
                prev = iter;
                iter = iter->next;
            }

            if(prev != NULL && prev->is_free) { // sprawdzenie czy poprzedni blok pamieci jest wolny i scalenie jelsi jest
                prev->size += current->size + sizeof(Header);
                prev->next = current->next;
            }
            return;
        }
        current = current->next;
    }
    printf("Address does not belong to allocated block\n");
}

void *heap_calloc(size_t count, size_t size) {

    if(count == 0 || size == 0) {
        return NULL;
    }

    if(count != 0 && size > SIZE_MAX / count) {
        return NULL;
    }

    size_t total_size = count * size;

    void *ptr = heap_malloc(total_size);

    if(ptr == NULL) {
        return NULL;
    }

    memset(ptr, 0, total_size);

    return ptr;
}

void *heap_realloc(void *payload_ptr, size_t new_size) {

    size_t real_needed_size = align(new_size);
    
    if(payload_ptr == NULL) {
        void *return_ptr = heap_malloc(real_needed_size);
        if(return_ptr == NULL) {
            return NULL;
        }
        return return_ptr;
    }

    if(real_needed_size == 0){
        heap_free(payload_ptr);
        return NULL;
    }

    Header *current_old = (Header *)((uint8_t *)payload_ptr - sizeof(Header));
    Header *old_next = current_old->next;

    if(real_needed_size == current_old->size) {
        return payload_ptr;
    }

    if(real_needed_size < current_old->size) {
        return payload_ptr;
    }

    if(old_next != NULL && old_next->is_free && old_next->size >= (real_needed_size - current_old->size)) {
        if(old_next->size >= (real_needed_size - current_old->size) + sizeof(Header) + ALIGNMENT) { // jesli jest zapas na nowy blok 

            Header *new_header = (Header *)((uint8_t *)payload_ptr + real_needed_size);

            new_header->size = old_next->size - (real_needed_size - current_old->size) - sizeof(Header);
            new_header->next = old_next->next;
            new_header->is_free = true;

            current_old->size = real_needed_size;
            current_old->next = new_header;
            current_old->is_free = false;

        } else {
            current_old->size += old_next->size + sizeof(Header);
            current_old->next = old_next->next;
            current_old->is_free = false;
        }
        return payload_ptr;
    } else {
        void *new_ptr = heap_malloc(real_needed_size);

        if(new_ptr == NULL) {
            return NULL;
        }

        memcpy(new_ptr, payload_ptr, current_old->size);

        heap_free(payload_ptr);

        return new_ptr;
    }
}


int main(void) {


    return 0;
}
