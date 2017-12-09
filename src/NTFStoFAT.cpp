//============================================================================
// Name        : NTFStoFAT.cpp
// Author      : Tomasz Rybicki
// Version     : 1.0
// Copyright   : 
// Description : NTFS to FAT32 converter. The application
//					takes as input the NTFS partition and an empty partition with
//					enough space for conversion.
//					The FAT32 partition will be created
//					containing files from NTFS partition
//
//					Application assumes a little endian format is used on the machine
//					Bootable partitions are not supported
//============================================================================

#include <iostream>
#include "DefinesFAT.h"
#include "FATManager.h"

using namespace std;


/* TODO:
 *
 * add fats to mount fat
 * why arent fat clusters in fat?
 * what is fat third entry
 *
 * optional:
 * 	specify only a disk and partition will be created for FAT
 */
int main() {
	FATManager m2("/dev/sdb1");
	char zero[0x1000]={0};
	m2.write(0x4000, 0x1000, zero);
	m2.write(0x83800, 0x1000, zero);


	FATManager manager("/dev/sdc1");
	manager.writeBPB();
	manager.writeFSInfo();
	return 0;
}
