#include "BitReader.h"
#include <Windows.h>
#include <conio.h>
#include "SymStats.h"

/*
Read symbols from file. At each step, replace the last read symbol with one that costs more bits
Chances are that each symbol will appear the same amount of time. If we keep updating our symbol library, there is a chance that high cost symbols will cost less
*/

#define Cube_Rows 16
#define Cube_Colls 16
#define CubeCordToIndex(x,y) (y * Cube_Colls + x)
#define SymbolBitCount 8
#define SymbolByteCount 1

void InitRubicCube(unsigned short* Cube)
{
	unsigned short* Cube_ = &Cube[CubeCordToIndex(-1, -1)];
	memset(Cube_, 0xFFFFFFFF, sizeof(unsigned short) * (Cube_Rows + 2) * (Cube_Colls + 2));
	for (int y = 0; y < Cube_Rows; y++)
		for (int x = 0; x < Cube_Colls; x++)
			Cube[CubeCordToIndex(x, y)] = CubeCordToIndex(x, y);
}

int GetBitCost(int Symbol)
{
	int BitCount = 0;
	while (Symbol != 0)
	{
		BitCount += (Symbol & 1);
		Symbol = Symbol >> 1;
	}
	return BitCount;
}

void EvolveCube(unsigned short* Cube, int atx, int aty)
{
	int WorstX, WorstY;
	int WorstCost = -1;
	for (int y = aty - 1; y < aty + 1; y++)
		for (int x = atx - 1; x < atx + 1; x++)
		{
			int CostNow = GetBitCost(Cube[CubeCordToIndex(x, y)]);
			if (CostNow > WorstCost)
			{
				WorstCost = CostNow;
				WorstX = x;
				WorstY = y;
			}
		}
	//swap current cost with worst cost. Should have a smaller chance to trigger than the one we use now
	if (WorstCost != -1)
	{
		unsigned short OldValue = Cube[CubeCordToIndex(atx, aty)];
		unsigned short NewValue = Cube[CubeCordToIndex(WorstX, WorstY)];
		Cube[CubeCordToIndex(atx, aty)] = NewValue;
		Cube[CubeCordToIndex(WorstX, WorstY)] = OldValue;
	}
}

void SplitSymbolToXY(unsigned char* ReadBuff, int* x, int* y)
{
	*x = (ReadBuff[0]) & 0x0F;
	*y = ((int)ReadBuff[0] >> 4) & 0x0F;
}
/*
void AssambleSymbolToXY(unsigned char* ReadBuff, int* x, int* y)
{
	ReadBuff[0] = *x & 0x0F;
	ReadBuff[0] |= (*y & 0x0F) << 4;
}*/

void ProcessFile_Rubic(const char* Input, const char* Output, int InitialSkips = 0, int SkipCount = 0)
{
	FILE *inf, *outf;
	unsigned short* Cube_ = (unsigned short*)malloc(sizeof(unsigned short) * (Cube_Rows + 2) * (Cube_Colls + 2));
	unsigned short* Cube = &Cube_[CubeCordToIndex(1, 1)];
	InitRubicCube(Cube);
	errno_t er = fopen_s(&inf, Input, "rb");
	outf = fopen(Output, "wb");
	int x, y;
	do {
		size_t ReadC;
		unsigned char ReadBuff[16];
		ReadC = fread_s(ReadBuff, sizeof(ReadBuff), 1, SymbolByteCount, inf);
		if (ReadC != SymbolByteCount)
			break;
		SplitSymbolToXY(ReadBuff, &x, &y);
		int OldValue = Cube[CubeCordToIndex(x, y)];
		EvolveCube(Cube, x, y);
		fwrite(&OldValue, 1, SymbolByteCount, outf);
	} while (1);
	fclose(inf);
	fclose(outf);
	free(Cube_);
}

void DoRubicTest()
{
	PrintStatisticsf("data.rar", 1);
	ProcessFile_Rubic("data.rar", "out2.dat");
	PrintStatisticsf("out2.dat", 1);
	ProcessFile_Rubic("out2.dat", "out3.dat");
	PrintStatisticsf("out3.dat", 1);
	ProcessFile_Rubic("out3.dat", "out4.dat");
	PrintStatisticsf("out4.dat", 1);

	printf("Program will exit on keypress");
	_getch();
	exit(0);
}