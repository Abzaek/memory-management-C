#include <stdio.h>
#include <stdlib.h>
int fact(unsigned int n);
int itFact(unsigned int n);
int* pSum(int arr[], int n);

int main()
{
    int arr[] = {1, 2, 3, 4, 5, 6};
    int second = pSum(arr, 6);

    int i;

    for (i = 0; i < 6; i++) {
        printf("%d ",second[i]);
    }
    printf("\n");
    printf("%d, %d", fact(5), itFact(5));
}


int* pSum(int arr[], int n) {
    int acc = 0;
    int index = 0;

    while (index < n) {
        acc = acc + arr[index];
        arr[index] = acc;
        index++;
    };
    return &arr;
}
















