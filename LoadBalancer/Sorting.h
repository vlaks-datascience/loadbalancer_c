#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "Data.h"

struct SortedList {
	Data* data;
	struct SortedList* next;
};

void insert(Data* data);

void sort();

SortedList* Current();

void AddToCurrent(int len);

void Display();