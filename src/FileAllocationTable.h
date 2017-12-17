/*
 * FileAllocationTable.h
 *
 *  Created on: Dec 15, 2017
 *      Author: root
 */

#ifndef FILEALLOCATIONTABLE_H_
#define FILEALLOCATIONTABLE_H_

#include <iostream>
#include <vector>
#include <list>
#include <fstream>
#include <cstring>
#include "DefinesFAT.h"

class FATManager;


class FileAllocationTable {
public:
	FileAllocationTable(std::streamoff offset
			, uint32_t clusterSize
			, uint32_t bytesPerSector
			, uint32_t sizeInSectors
			, FATManager* fatManager);
	virtual ~FileAllocationTable();

	/* Writes objects contents to disk */
	void sync();

	/* Looks for free clusters, writes data, and updates FAT table
	 * returns number of files first cluster */
	uint32_t writeFile(std::list<Cluster> file);

	/* Writes data to cluster of given number
	 * where cluster number two is first cluster after FATs */
	void writeToCluster(byte_t* data, uint32_t clusterNumber);

	/* Allocates another cluster for given file, returns its number */
	uint32_t extendFile(uint32_t lastClusterNumber);

private:
	/* Offset of the table from partition start */
	std::streamoff m_FAToffset;
	uint32_t m_bytesPerSector;
	uint32_t m_clusterSize;
	uint32_t m_sizeInSectors;

	/* Map[cluster number] = fat index/code  */
	std::vector <uint32_t> m_entries;

	/* FAT table codes - written in big endian! */
	uint32_t m_endOfChain = 0x0FFFFFFF;
	uint32_t m_notUsed = 0x00;
	uint32_t m_fatId = 0xF8FFFF0F;

	FATManager* m_manager;

};

#endif /* FILEALLOCATIONTABLE_H_ */
