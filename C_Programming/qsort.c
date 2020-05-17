
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>

int partition(int arr[], int low, int high);
void quicksort(int arr[], int start, int end); 
void swap(int *a, int *b);
void print(int arr[], int size);
void printReverse(int arr[], int size); 

int main(int argc, char *argv[]){ 
	int a[] = {50, 23, 9, 18, 61, 32}; 
	int n = sizeof(a) / sizeof(a[0]); 
	quicksort(a, 0, n-1); 

	int option; 
	while ((option = getopt(argc, argv, "ru")) != -1) {
		switch (option) { 
			case 'r': 
				printReverse(a, n);
				exit(0);
			case '?':
				printf("Unknown option\n");
				exit(1);
		}
	}
	print(a, n);
	return 0; 
}

void swap(int *a, int *b) {
	int t = *a; 	// store the value of a here 
	*a = *b; 	// swap values of a and b 
	*b = t;		// set b to a's value from temp  
}

int partition(int arr[], int low, int high) { 
	int piv = arr[high];  		// pivot is the last elem in list 
	int i = low-1;			// set the index counting the elements less than pivot to be 1 less than starting index. In the first case, it'll be -1 because start will be 0 
	for (int j = low; j < high-1; j++) {
		if (arr[j] <= piv) {
			i++; 
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[i+1], &arr[high]); 	// swap where counter pointing to place where the pivot element should be 
	return i+1;
}

void quicksort(int arr[], int start, int end) {
	if (start < end) { 
		int q; 
		q = partition(arr, start, end); 	// q will be the returned index where we then call the Divide and Conquer recursively for the LHS and RHS of array
		quicksort(arr, start, q-1);		// recursively solving left hand side of array (LHS)
		quicksort(arr, q+1, end);		// recursively solving right hand side of array (RHS)
	}	
}

void print(int arr[], int size){
	for (int i = 0 ; i < size; i++ ) {
		printf("%d ", arr[i]);
	}
	printf("\n");
}

void printReverse(int arr[], int size) { 
	for (int i = size-1; i >= 0; i--){
		printf("%d ", arr[i]);
	}
	printf("\n");
}
