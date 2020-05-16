
#include <stdio.h>
#include <stdlib.h>

void partition(int arr[], int low, int high);
void quicksort(int arr, int start, int end); 
void swap(int *a, int *b);

int main(int argc, char *argv[]){ 
	int a[] = {50, 23, 9, 18, 61, 32}; 
	int n = sizeof(a) / sizeof(a[0]); 
	quicksort(a, 0, n-1); 
	return 0; 
}

void swap(int *a, int *b) {
	int t = *a; 	// store the value of a here 
	*a = *b; 	// swap values of a and b 
	*b = t;		// set b to a's value from temp  
}

void partition(int arr[], int low, int high) { 
	int piv = arr[high];  		// pivot is the last elem in list 
	int i = low-1;			// set the index counting the elements less than pivot to be 1 less than starting index. In the first case, it'll be -1 because start will be 0 
	for (int j = low; j < high-1; j++) {
		if (arr[j] <= piv) {
			i++; 
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[i], &arr[piv]); 	// swap where counter pointing to place where the pivot element should be 
}


