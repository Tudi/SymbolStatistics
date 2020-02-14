#pragma once

struct BitwriterStore
{

};

void InitBitwriter(BitwriterStore* bw, int PreallocSize);
void WriteBit(BitwriterStore* bw, int Symbol);
void DumptToFile(BitwriterStore* bw, const char *FileName);