/*
 * NTFSManager.h
 *
 *  Created on: Dec 17, 2017
 *      Author: root
 */

#ifndef NTFSMANAGER_H_
#define NTFSMANAGER_H_

#include <string>
#include <fstream>
#include <iostream>
#include "DefinesFAT.h"
#include "FATManager.h"

class NTFSManager {
public:
	NTFSManager(std::string part);
	virtual ~NTFSManager();

	/* Gets info about basic parameters such as sector size */
	void readBPB();

	/* Sets a pointer to FATManager, used for writing data to FAT */
	void getFatManager(FATManager* fatManager);

	/* Reads MFT and copies files to FAT partition */
	void readMFT();

	void handleAttribute(uint32_t attr, std::streamoff attrStart, File &file);

	void read(std::streamoff offset, uint32_t length, byte_t* dest);

	static uint32_t string2int(byte_t* src, uint32_t n);
	static uint64_t string2long(byte_t* src, uint32_t n);



	byte_t* getBytesPerSector() {
		return m_bytesPerSector;
	}

	byte_t* getReservedSectors(){
		return m_reservedSectors;
	}

	byte_t getSectorsPerCluster()  {
		return m_sectorsPerCluster;
	}

	byte_t* getTotalSectors() {
		return m_totalSectors;
	}

private:
	std::string m_partition;
	byte_t m_bytesPerSector[2] = {};
	byte_t m_sectorsPerCluster;
	byte_t m_reservedSectors[2] = {};
	byte_t m_MftLcn[8];
	byte_t m_clustersPerRecord[4];
	byte_t m_totalSectors[8];

	FATManager* m_fatManager;

};

#endif /* NTFSMANAGER_H_ */
