#ifndef _TOOL_H_
#define _TOOL_H_
#include <iostream>
#include <string>

using namespace std;

void swap(string *words, int src, int dest);
void heapify(string *words, int n, int i);
void heapSort(string *words, int n);

void postlistMergeSort(string *arr, int low, int high);
void postlistMerge(string *arr, int low, int middle, int high);


#endif