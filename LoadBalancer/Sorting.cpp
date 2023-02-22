#pragma comment(lib, "Ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include "Sorting.h"

struct SortedList* head, * tail = NULL;

void insert(Data* data)
{
    //Create a new node  
    struct SortedList* newNode = (struct SortedList*)malloc(sizeof(struct SortedList));
    newNode->data = data;
    newNode->next = NULL;

    //Checks if the list is empty  
    if (head == NULL) {
        //If list is empty, both head and tail will point to new node  
        head = newNode;
        tail = newNode;
    }
    else {
        //newNode will be added after tail such that tail's next will point to newNode  
        tail->next = newNode;
        //newNode will become new tail of the list  
        tail = newNode;
    }
}

void sort() {
    //Node current will point to head  
    struct SortedList* current = head, * index = NULL;
    Data* temp = (Data*)malloc(sizeof(struct Data));

    if (head == NULL) {
        return;
    }
    else {
        while (current != NULL) {
            //Node index will point to node next to current  
            index = current->next;

            while (index != NULL) {
                //If current node's data is greater than index's node data, swap the data between them  
                if (current->data->DataCount > index->data->DataCount) {
                    temp->DataCount = current->data->DataCount;

                    temp->acceptedSocket = current->data->acceptedSocket;

                    current->data->DataCount = index->data->DataCount;

                    current->data->acceptedSocket = index->data->acceptedSocket;

                    index->data->DataCount = temp->DataCount;

                    index->data->acceptedSocket = temp->acceptedSocket;
                }
                index = index->next;
            }
            current = current->next;
        }
    }
}

SortedList* Current() {
    return head;
}

void AddToCurrent(int len) {
    head->data->DataCount = head->data->DataCount + len;
}

void Display()
{
    printf("Current state in the list:\n");
    //Node current will point to head  
    struct SortedList* current = head;
    if (head == NULL) {
        printf("List is empty!\n");
        return;
    }
    while (current != NULL) {
        //Prints each node by incrementing pointer  
        printf("%d ", current -> data -> DataCount);
        current = current -> next;
    }
    printf("\n");
}