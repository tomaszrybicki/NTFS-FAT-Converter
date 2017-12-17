/*
 * FATManager.h
 *
 *  Created on: Dec 9, 2017
 *  Author: Tomasz Rybicki
 */

#ifndef FATMANAGER_H_
#define FATMANAGER_H_

#include "DefinesFAT.h"
#include "FileAllocationTable.h"
#include "DirectoryTable.h"
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

class FATManager {
public:
	FATManager(string drive
			, uint32_t bytesPerSector
			, uint32_t sectorsPerCluster
			, uint32_t reservedSectors
			, uint64_t neededSectors);
	virtual ~FATManager();

	/* This function writes given file to disk */
	void writeFile(list<Cluster>
					, byte_t* filename
					, byte_t* extension
					, uint32_t fileLength);

	void readPartitionTable();
	void writeBPB();
	void writeFSInfo();
	void writeFATs();
	static uint32_t string2int(byte_t* src, uint32_t n);
	void read(streamoff offset, uint32_t length, byte_t* dest);
	void write(streamoff offset, uint32_t length, byte_t* src);
	LBA convertCHStoLBA(CHS source);
	CHS convertLBAtoCHS(LBA source);
	uint32_t clusterCeil(uint32_t offset);

private:
	/* Partition parameters */
	string m_drive;
	uint32_t m_bytesPerSector;
	uint32_t m_sectorsPerCluster;
	uint32_t m_reservedSectors;
	uint32_t m_fatSize;
	streamoff m_fat1Offset;
	streamoff m_fat2Offset;
	streamoff m_dataClustersOffset;

	/* Structures */
	Partition m_partitionTable[4];
	BPB m_bpb;
	FSInfo m_fsInfo;
	FileAllocationTable m_fat1;
	FileAllocationTable m_fat2;
	/* Directory can be on multiple clusters */
	std::vector<DirectoryTable> m_rootDirectory;

};

#endif /* FATMANAGER_H_ */
