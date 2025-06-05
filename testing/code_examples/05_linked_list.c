#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Node structure
typedef struct Node {
    int data;
    struct Node* next;
    struct Node* prev;  // For doubly linked list
} Node;

// List structure
typedef struct {
    Node* head;
    Node* tail;
    size_t size;
} LinkedList;

// Function to create a new node
Node* create_node(int data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    new_node->data = data;
    new_node->next = NULL;
    new_node->prev = NULL;
    return new_node;
}

// Function to initialize a linked list
LinkedList* create_list() {
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    if (!list) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

// Function to insert at the beginning
void insert_front(LinkedList* list, int data) {
    Node* new_node = create_node(data);
    
    if (list->head == NULL) {
        list->head = list->tail = new_node;
    } else {
        new_node->next = list->head;
        list->head->prev = new_node;
        list->head = new_node;
    }
    list->size++;
}

// Function to insert at the end
void insert_back(LinkedList* list, int data) {
    Node* new_node = create_node(data);
    
    if (list->tail == NULL) {
        list->head = list->tail = new_node;
    } else {
        new_node->prev = list->tail;
        list->tail->next = new_node;
        list->tail = new_node;
    }
    list->size++;
}

// Function to insert at a specific position
void insert_at(LinkedList* list, int data, size_t position) {
    if (position > list->size) {
        printf("Position out of bounds\n");
        return;
    }
    
    if (position == 0) {
        insert_front(list, data);
        return;
    }
    
    if (position == list->size) {
        insert_back(list, data);
        return;
    }
    
    Node* current = list->head;
    for (size_t i = 0; i < position; i++) {
        current = current->next;
    }
    
    Node* new_node = create_node(data);
    new_node->next = current;
    new_node->prev = current->prev;
    current->prev->next = new_node;
    current->prev = new_node;
    list->size++;
}

// Function to delete a node
void delete_node(LinkedList* list, int data) {
    Node* current = list->head;
    
    while (current != NULL) {
        if (current->data == data) {
            if (current->prev) {
                current->prev->next = current->next;
            } else {
                list->head = current->next;
            }
            
            if (current->next) {
                current->next->prev = current->prev;
            } else {
                list->tail = current->prev;
            }
            
            free(current);
            list->size--;
            return;
        }
        current = current->next;
    }
    printf("Data not found in list\n");
}

// Function to reverse the list
void reverse_list(LinkedList* list) {
    Node* current = list->head;
    Node* temp = NULL;
    
    while (current != NULL) {
        temp = current->prev;
        current->prev = current->next;
        current->next = temp;
        current = current->prev;
    }
    
    if (temp != NULL) {
        list->head = temp->prev;
    }
}

// Function to print the list
void print_list(LinkedList* list) {
    Node* current = list->head;
    printf("List: ");
    while (current != NULL) {
        printf("%d ", current->data);
        current = current->next;
    }
    printf("\n");
}

// Function to free the list
void free_list(LinkedList* list) {
    Node* current = list->head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
    free(list);
}

int main() {
    LinkedList* list = create_list();
    
    // Test various operations
    insert_front(list, 10);
    insert_back(list, 20);
    insert_at(list, 15, 1);
    insert_back(list, 30);
    
    printf("Initial list:\n");
    print_list(list);
    
    printf("\nReversing list:\n");
    reverse_list(list);
    print_list(list);
    
    printf("\nDeleting node with data 15:\n");
    delete_node(list, 15);
    print_list(list);
    
    printf("\nInserting 25 at position 1:\n");
    insert_at(list, 25, 1);
    print_list(list);
    
    // Clean up
    free_list(list);
    
    return 0;
} 