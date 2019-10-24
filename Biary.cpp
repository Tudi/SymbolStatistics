#include "Biary.h"
#include "BitReader.h"
#include "BitWriter.h"
#include <Windows.h>
#include <conio.h>
#include "SymStats.h"

void InitBiary(BiaryStore *bs, int Ones, int Zeros)
{
	bs->ReadCount1 = Ones;
	bs->ReadCount0 = Zeros;
	bs->WrittenCount1 = 0;
	bs->WrittenCount0 = 0;
}

void WriteHeaderToFile(FILE *out, BiaryStore *bs)
{

}

int GuessNextSymbol(BiaryStore *bs, int InputSymbol)
{
	int ret;
//	if (bs->ReadCount1 * bs->WrittenCount0 >= bs->ReadCount0 * bs->WrittenCount1)
	if (bs->ReadCount1 - bs->WrittenCount1 <= bs->ReadCount0 - bs->WrittenCount0)
		ret = 1;
	else
		ret = 0;
	if(InputSymbol == 1)
		bs->WrittenCount1++;
	else
		bs->WrittenCount0++;
	return ret;
}

int ShouldSkipProcessing(int CurBitIndex, int InitialSkips, int SkipCount)
{
	if (CurBitIndex < InitialSkips)
		return 1;
	CurBitIndex -= InitialSkips;
	if (SkipCount == 0 || CurBitIndex % SkipCount == 0)
		return 0;
	return 1;
}

void ProcessFile(const char *Input, const char *Output, int InitialSkips = 0, int SkipCount = 0)
{
	bitreader br;
	BitwriterStore bw;
	BiaryStore bs;
	bitreader_open(br, Input, 1);
	printf("Input File size %d MB = %d\n", br.FileSize / 1024 / 1024, br.FileSize);
	//init bit writer
	InitBitwriter(&bw, br.FileSize + 8);
	//start reading the input and meantime process it
	do {
		int CountSymbol1 = 0;
		int CountSymbol0 = 0;
		int OutCountSymbol1 = 0;
		while (br.readcount && br.readcount > br.processed_count + (br.SymbolBitCount + 7) / 8)
		{
			//count number of symbols in input buffer
			int KeepNth = SkipCount + 1;
			for (int BitIndex = InitialSkips; BitIndex < br.readcount * 8; BitIndex++)
			{
				if (ShouldSkipProcessing(BitIndex, InitialSkips, KeepNth) == 1)
					continue;
				int Symbol = GetBitFromBuff(br.cbuffer, BitIndex);
				if (Symbol == 1)
					CountSymbol1++;
				else
					CountSymbol0++;
			}

			//init biary with symbol statistics. This should help us make the output have equal amount of symbols
			InitBiary(&bs, (int)CountSymbol1, CountSymbol0);

			//start guessing symbols
			for (int BitIndex = InitialSkips; BitIndex < br.readcount * 8; BitIndex++)
			{
				int StreamInputSymbol = GetBitFromBuff(br.cbuffer, BitIndex);
				int OutputSymbol;
				if (ShouldSkipProcessing(BitIndex, InitialSkips, KeepNth) == 0)
				{
					int GuessedSymbol = GuessNextSymbol(&bs, StreamInputSymbol);
					OutputSymbol = (StreamInputSymbol == GuessedSymbol);
					if (OutputSymbol == 1)
						OutCountSymbol1++;
				}
				else
					OutputSymbol = StreamInputSymbol;
				WriteBit(&bw, OutputSymbol);
			}

			// done with this chunk we just read
			br.processed_count += br.readcount;
		}

		printf("Started with %d ones, ended with %d ones, Diff %d, diff %f\n", (int)CountSymbol1, OutCountSymbol1, (int)CountSymbol1 - OutCountSymbol1, (float)CountSymbol1 / (float)OutCountSymbol1);
		br.readcount = (unsigned int)fread(br.cbuffer, 1, BITREADER_FILE_SEGMENT_BUFFER_SIZE, br.inf);
		if (br.readcount > 0)
		{
			printf("Reading next buffer %u Kbytes\n", br.readcount / 1024);
			br.TotalReadCount += br.readcount;
			// no blocks processed yet 
			br.processed_count = 0;
		}
	} while (br.readcount);
	//we are done processing input
	//dump the output buffer into a file
	DumptToFile(&bw, Output);

	bitreader_close(br);
}

void DoBiaryTest()
{
	PrintStatisticsf("data.rar", 2);
	ProcessFile("data.rar", "out.dat");
//	for(int i=0;i<10;i++)
//		ProcessFile("out.dat", "out.dat");
	ProcessFile("out.dat", "out.dat", 0, 1);
	PrintStatisticsf("out.dat", 2);
	ProcessFile("out.dat", "out.dat", 1, 1);
	PrintStatisticsf("out.dat", 2);
	ProcessFile("out.dat", "out.dat", 0, 2);
	PrintStatisticsf("out.dat", 2);
	ProcessFile("out.dat", "out.dat", 1, 2);
	PrintStatisticsf("out.dat", 2);
	ProcessFile("out.dat", "out.dat", 2, 2);
	PrintStatisticsf("out.dat", 2);
	_getch();
	exit(0);
}