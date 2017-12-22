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
//
//					So far the application cannot convert directories
//					and copies also files deleted on NTFS partition
//============================================================================

#include <iostream>
#include "DefinesFAT.h"
#include "FATManager.h"
#include "NTFSManager.h"

using namespace std;


int main(int argc, char *argv[]) {
	bool readDeleted = false;

	/* Show help */
	if(argc <= 2){
		cout << "NTFStoFAT usage:  \t./NTFStoFAT <ntfs_partition>  <zero_filled_empty_partition> --deleted (optional)" << endl;
		cout << "\t\t\t./NTFStoFAT -H for help" << endl;

		cout << endl << "This tool converts an NTFS partition to a new FAT32 formatted partition." << endl;
		cout << "--deleted flag makes the tool copy also files flagged as deleted (not in-use)" << endl;
		cout << "So far directories are not copied, only files with data content." << endl;
		cout << "WARNING: This tool is for experimental use only!!!"<<endl;

	}else if (argc >= 3){
		/* Get arguments */
		std::string input(argv[1]);
		std::string output(argv[2]);

		if (argc == 4){
			std::string flag(argv[3]);

			if (flag == "--deleted"){
				readDeleted = true;
			}
		}

		/* Create and initialize NTFS partition manager */
		NTFSManager ntfsManager(input, readDeleted);
		ntfsManager.readBPB();

		/* Create and initialize FAT32 partition manager */
		FATManager fatManager(output
				, ntfsManager.string2int(ntfsManager.getBytesPerSector(), 2)
				, ntfsManager.getSectorsPerCluster()
				, ntfsManager.string2long(ntfsManager.getTotalSectors(), 8));

		/* Start conversion */
		ntfsManager.getFatManager(&fatManager);
		ntfsManager.readMFT();
	}

	return 0;
}
