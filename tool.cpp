#include <iostream>
#include <string>
#include "tool.h"

using namespace std;

/*
	This tool.cpp consist of two functions:
		1. heap sort the filename
			like: 
				input: file_B, file_C, file_A
				output: file_A->file_B->file_C...
		2. merge sort the postlist based on the count
			like: 	<docId#count>
				input: 3#4.45, 20#1.2, 30#4.5, 35#0.5
				output: 30#4.5->3#4.45->20#1.2->35#0.5
*/

// Function_1: heap sort the filename
void swap(string *words, int src, int dest)
{
	string temp = words[src];
	words[src] = words[dest];
	words[dest] = temp;
}
void heapify(string *words, int n, int i)
{
	int largest = i;
	int left = i * 2 + 1;
	int right = i * 2 + 2;

	if(left < n && words[left].compare(words[largest]) >= 0){
		largest = left;
	}

	if(right < n && words[right].compare(words[largest]) >= 0){
		largest = right;
	}

	if(largest != i){
		swap(words, i, largest);
		heapify(words, n, largest);
	}
}
void heapSort(string *words, int n)
{
	int i;
	for(i = n / 2 - 1; i >= 0; i--){
		heapify(words, n, i);
	}

	for(i = n - 1; i >= 0; i--){
		swap(words, 0, i);
		heapify(words, i, 0);
	}
}

// Funtion_2: merge sort the postlist based on the count
//		like: 	<docId#count>
//				input: 3#4.45, 20#1.2, 30#4.5, 35#0.5
//				output: 30#4.5->3#4.45->20#1.2->35#0.5
//
void postlistMerge(string *arr, int low, int middle, int high)
{
	string* temp = new string[high - low + 1];
	int i = low;
	int j = middle + 1;
	int index = 0;
	int k;

	while(i <= middle && j <= high){
		double value_i = stod(arr[i].substr(arr[i].find('#') + 1));
		double value_j = stod(arr[j].substr(arr[j].find('#') + 1));
		if(value_i < value_j){
			temp[index++] = arr[j++];
		}else{
			temp[index++] = arr[i++];
		}
	}

	while(i <= middle){
		temp[index++] = arr[i++];
	}

	while(j <= high){
		temp[index++] = arr[j++];
	}

	for(k = 0; k < high - low + 1; k++){
		arr[k + low] = temp[k];
	}
	delete []temp;
}

void postlistMergeSort(string *arr, int low, int high)
{
	int middle;
	if(low < high){
		middle = low + ((high-low) >> 1);
		postlistMergeSort(arr, low, middle);
		postlistMergeSort(arr, middle + 1, high);
		postlistMerge(arr, low, middle, high);
	}
}
