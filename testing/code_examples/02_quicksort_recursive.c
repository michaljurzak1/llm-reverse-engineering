#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function to swap two elements
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Function to partition the array
int partition(int arr[], int low, int high) {
    // Choose the rightmost element as pivot
    int pivot = arr[high];
    
    // Index of smaller element
    int i = (low - 1);
    
    // Compare each element with pivot
    for (int j = low; j <= high - 1; j++) {
        // If current element is smaller than or equal to pivot
        if (arr[j] <= pivot) {
            i++;    // Increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

// Function to implement QuickSort
void quickSort(int arr[], int low, int high) {
    if (low < high) {
        // Find the partition index
        int pi = partition(arr, low, high);
        
        // Sort elements before and after partition
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

// Function to print array
void printArray(int arr[], int size) {
    for (int i = 0; i < size; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

// Function to generate random array
void generateRandomArray(int arr[], int size) {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        arr[i] = rand() % 1000;  // Random numbers between 0 and 999
    }
}

// Function to check if array is sorted
int isSorted(int arr[], int size) {
    for (int i = 0; i < size - 1; i++) {
        if (arr[i] > arr[i + 1])
            return 0;
    }
    return 1;
}

int main() {
    const int SIZE = 20;
    int arr[SIZE];
    
    // Generate random array
    generateRandomArray(arr, SIZE);
    
    printf("Original array: ");
    printArray(arr, SIZE);
    
    // Perform QuickSort
    quickSort(arr, 0, SIZE - 1);
    
    printf("Sorted array: ");
    printArray(arr, SIZE);
    
    // Verify sorting
    if (isSorted(arr, SIZE))
        printf("Array is correctly sorted!\n");
    else
        printf("Error: Array is not sorted correctly!\n");
    
    return 0;
} 