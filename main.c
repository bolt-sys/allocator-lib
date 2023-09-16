//
// Created by valo on 9/15/23.
//
#include <stdio.h>
#include <Kernel.h>
#include <stdlib.h>
#include <time.h>


MEMORY_DESCRIPTOR** generateMemoryDescriptor()
{
	MEMORY_DESCRIPTOR** descriptor = malloc(sizeof(MEMORY_DESCRIPTOR*) * 20);
	for (int i = 0; i < 20; ++i)
	{
		int size = 8192 + (((float )rand() / RAND_MAX) * 32768);
		descriptor[i] = malloc(sizeof(MEMORY_DESCRIPTOR));
		descriptor[i]->Base = (UINT64) malloc(size);
		descriptor[i]->Length = size;
		descriptor[i]->Type = (((float) rand() / RAND_MAX) * 10) < 6.5 ? MEMMAP_USABLE : MEMMAP_RESERVED;
	}
	return descriptor;
}

KERNEL_PARAMETERS* GenerateParameters(void)
{
	KERNEL_PARAMETERS*  Parameters;
	CHAR8*              CommandLine;
	MEMORY_DESCRIPTOR** Descriptor;

	srand(time(0));

	Parameters = (KERNEL_PARAMETERS*)malloc(sizeof(KERNEL_PARAMETERS));
	if (Parameters == NULL) {
		return NULL;
	}

	CommandLine = (CHAR8*)malloc(256);
	if (CommandLine == NULL) {
		free(Parameters);
		return NULL;
	}

	Descriptor = generateMemoryDescriptor();

	Parameters->CommandLineSize = 256;
	Parameters->CommandLine = &CommandLine;
	Parameters->MemoryMapSize = 20;
	Parameters->MemoryMap = Descriptor;

	return Parameters;
}

int main(void)
{
	KERNEL_PARAMETERS* Parameters = GenerateParameters();
	if (Parameters == NULL) {
		printf("Failed to generate parameters!");
		return 1;
	}
	// print memory descriptors
	for (UINTN i = 0; i < Parameters->MemoryMapSize; ++i)
	{
		MEMORY_DESCRIPTOR* Descriptor = Parameters->MemoryMap[i];
		printf("Memory Descriptor %llu: Base: %llu, Length: %llu, Type: %llu\n", i, Parameters->MemoryMap[i]->Base, Parameters->MemoryMap[i]->Length, Parameters->MemoryMap[i]->Type);
	}
	return (0);
}