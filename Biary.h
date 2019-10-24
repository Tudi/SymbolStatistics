#pragma once
#include <stdio.h>

struct BiaryStore
{
	//number of symbols we will have at the end
	int ReadCount1;
	int ReadCount0;
	//number of symbols we guessed so far
	int WrittenCount1;
	int WrittenCount0;
};

void InitBiary(BiaryStore *bs, int Ones, int Zeros);
void WriteHeaderToFile(FILE *out, BiaryStore *bs);
int GuessNextSymbol(BiaryStore *bs);
void DoBiaryTest();