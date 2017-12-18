/*
 * FileAllocationTable.cpp
 *
 *  Created on: Dec 15, 2017
 *      Author: root
 */


#include "FileAllocationTable.h"
#include "DefinesFAT.h"
#include "FATManager.h"

using namespace std;

FileAllocationTable::FileAllocationTable(streamoff offset
		, uint32_t clusterSize
		, uint32_t bytesPerSector
		, uint32_t sizeInSectors
		, FATManager* fatManager)
	: m_FAToffset(offset)
	, m_bytesPerSector(bytesPerSector)
	, m_clusterSize(clusterSize)
	, m_sizeInSectors(sizeInSectors)
	, m_entries(m_sizeInSectors*m_bytesPerSector/4)
	, m_manager(fatManager)
{
	/* Create FAT ID (1. entry) */
	m_entries[0] = m_fatId;

	/* Create end of chain marker (2. entry) */
	m_entries[1] = m_endOfChain;

	/* Create entry for root directory (3. entry) */
	m_entries[2] = m_endOfChain;
}



FileAllocationTable::~FileAllocationTable() {
}

void FileAllocationTable::sync() {
	cout << "\t[FAT32] Writing File Allocation Table(0x" << std::hex << m_FAToffset << ") to disk..." << endl;

	for (uint32_t i = 0; i < m_entries.size(); i++){
		m_manager->write(m_FAToffset + (i * 4), 4, (byte_t*)&m_entries[i]);
	}
}

uint32_t FileAllocationTable::writeFile(list<Cluster> file) {
	/* First run to see if there is space on the filesystem */
	uint32_t neededClusters = file.size();
	uint32_t freeClusters = 0;
	for (uint32_t i = 0; i < m_entries.size(); i++){
		if (m_entries[i] == m_notUsed){
			freeClusters++;

			if (freeClusters >= neededClusters){
				break;
			}
		}
	}

	if(neededClusters < freeClusters){
		cout << "Not enough space on FAT partition for the file of size ";
		cout << neededClusters << " clusters" << endl;
		return 0;
	}

	/* Write data clusters and update FAT accordingly */
	Cluster clusterToWrite(0);
	uint32_t previousEntry = 0;
	uint32_t firstCluster = 0;

	for (uint32_t i = 0; i < m_entries.size(); i++){

		/* If no more clusters to write - file has been copied */
		if (file.empty()){

			/* Set end of chain in fat */
			if (previousEntry)
				m_entries[previousEntry] = m_endOfChain;
			return firstCluster;
		}


		/* Found free cluster */
		if (m_entries[i] == m_notUsed){
			if (!firstCluster){
				firstCluster = i;
			}

			/* Get next cluster to write */
			clusterToWrite = *(file.begin());
			file.pop_front();

			/* Save data */
			writeToCluster(clusterToWrite.m_data, i);

			/* Update FAT entries - link file clusters */
			if (previousEntry){
				m_entries[previousEntry] = i;
			}

			previousEntry = i;
		}
	}

	if (file.empty()){
		/* Set end of chain in fat */
		if (previousEntry)
			m_entries[previousEntry] = m_endOfChain;
		return firstCluster;
	}

	return 0;
}


void FileAllocationTable::writeToCluster(byte_t* data, uint32_t clusterNumber) {
	if (clusterNumber < 2){
		cout << "ERROR: Writing to one of first two FAT entries!!!" << endl;
		return;
	}

	/* We need to subtract 2, because third FAT entry (which has number 2,
	 * because of 0-based indexing) is the first cluster in data part */
	streamoff offset = (clusterNumber - 2) * m_clusterSize * m_bytesPerSector;
	uint32_t length = m_clusterSize * m_bytesPerSector;

	m_manager->write(m_manager->getDataClustersOffset() + offset, length, data);
}

uint32_t FileAllocationTable::extendFile(uint32_t lastClusterNumber) {
	/* Find free cluster */
	for (uint32_t i = 0; i < m_entries.size(); i++){

			/* Found free cluster */
			if (m_entries[i] == m_notUsed){

				/* Update previously last cluster */
				m_entries[lastClusterNumber] = i;

				/* Update current last cluster */
				m_entries[i] = m_endOfChain;

				return i;
			}
		}

	return 0;
}
