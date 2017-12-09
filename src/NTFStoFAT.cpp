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
//============================================================================

#include <iostream>
#include "DefinesFAT.h"
#include "FATManager.h"

using namespace std;


/* TODO:
 *
 * optional:
 * 	specify only a disk and partition will be created for FAT
 */
int main() {

	FATManager manager("/dev/sdb1");

	return 0;
}
