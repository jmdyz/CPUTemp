#pragma once

#include <iostream>
#include <fstream>
#include <bitset>
#include <Windows.h>
#include <stdio.h>

#include "resource.h"

void WriteFromStream(unsigned char* buf, const char* filename, unsigned int size);
bool resource_release(const char* filename);