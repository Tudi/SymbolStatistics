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
    printf("Input File size %d MB = %d B for %s\n", br.FileSize / 1024 / 1024, br.FileSize, Input);
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
    bitreader_close(br);
    //dump the output buffer into a file
    DumptToFile(&bw, Output);
    //free memory as we tend to run low on it :P
    BitwriterFree(&bw);
}

void DoBiaryTest()
{
	PrintStatisticsf("data.rar", 1);
	ProcessFile("data.rar", "out.dat2");
	for(int i=0;i<10;i++)
		ProcessFile("out.dat2", "out.dat2", i & 1);
/*    PrintStatisticsf("out.dat2", 1);
	ProcessFile("out.dat2", "out.dat3");
	PrintStatisticsf("out.dat3", 1);
	ProcessFile("out.dat3", "out.dat4");
	PrintStatisticsf("out.dat4", 1);
	ProcessFile("out.dat4", "out.dat5");
	PrintStatisticsf("out.dat5", 1);
	ProcessFile("out.dat5", "out.dat6");
	PrintStatisticsf("out.dat6", 1);*/
	_getch();
	exit(0);

    /*
    count 0   : 1339439
    count 1   : 1398345
    Sum        : 2737784
    pct 0 / 1 : 0.957874 1.043978
    pct 0 / 1 : 0.489242 0.510758
    Input File size 0 MB = 342224 for data.rar
    Started with 1398345 ones, ended with 1339433 ones, Diff 58912, diff 1.04
    count 0   : 1398351
    count 1   : 1339433
    Sum        : 2737784
    pct 0 / 1 : 1.043987 0.957866
    pct 0 / 1 : 0.510760 0.489240
    Input File size 0 MB = 342224 for out.dat2
    Started with 1339433 ones, ended with 1339433 ones, Diff 0, diff 1.000000
    count 0   : 1398351
    count 1   : 1339433
    Sum        : 2737784
    pct 0 / 1 : 1.043987 0.957866
    pct 0 / 1 : 0.510760 0.489240
    Input File size 0 MB = 342224 for out.dat3
    Started with 1339433 ones, ended with 1339433 ones, Diff 0, diff 1.000000
    count 0   : 1398351
    count 1   : 1339433
    Sum        : 2737784
    pct 0 / 1 : 1.043987 0.957866
    pct 0 / 1 : 0.510760 0.489240
    Input File size 0 MB = 342224 for out.dat4
    Started with 1339433 ones, ended with 1339433 ones, Diff 0, diff 1.000000
    count 0   : 1398351
    count 1   : 1339433
    Sum        : 2737784
    pct 0 / 1 : 1.043987 0.957866
    pct 0 / 1 : 0.510760 0.489240
    Input File size 0 MB = 342224 for out.dat5
    Started with 1339433 ones, ended with 1339433 ones, Diff 0, diff 1.000000
    count 0   : 1398351
    count 1   : 1339433
    Sum        : 2737784
    pct 0 / 1 : 1.043987 0.957866
    pct 0 / 1 : 0.510760 0.489240
    */
}