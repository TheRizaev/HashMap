#include "Table.h"
#include "GroupContainer.h"
#include "MemoryManager.h"
#include "Mem.h"
#include <iostream>
#include <locale.h>
#include <string.h>
#include "Test.h"
using namespace std;


int main()
{
	setlocale(LC_ALL, "Russian");
	ContainerTester::testTableContainer();
	return 0;
}