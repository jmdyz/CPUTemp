#pragma once

#include <iostream>
#include <fstream>
#include <bitset>
#include <stdio.h>

unsigned int len(const char sys[]);
unsigned int HexToDec(const char c);
void WriteFromStream(unsigned char* buf, char* filename, unsigned int size);
void HexToChar(const char sys[], char* filename);