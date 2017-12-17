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
#include "NTFSManager.h"

using namespace std;


/* TODO:
 * clean up files, private/public methods
 * nonresident data
 *
 * optional:
 * attributes copy
 * 	specify only a disk and partition will be created for FAT
 * 	create partition instead of fs on drive
 */
int main() {

	/* Create and initialize NTFS partition manager */
	NTFSManager ntfsManager("/dev/sdb1");
	ntfsManager.readBPB();

	/* Create and initialize FAT32 partition manager */
	FATManager fatManager("/dev/sdc1"
			, ntfsManager.string2int(ntfsManager.getBytesPerSector(), 2)
			, ntfsManager.getSectorsPerCluster()
			, 32
			, ntfsManager.string2long(ntfsManager.getTotalSectors(), 8));

	/* Start conversion */
	ntfsManager.getFatManager(&fatManager);
	ntfsManager.readMFT();




	return 0;
}
