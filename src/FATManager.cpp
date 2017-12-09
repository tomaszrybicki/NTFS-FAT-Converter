/*
 * FATManager.cpp
 *
 *  Created on: Dec 9, 2017
 *  Author: Tomasz Rybicki
 */

#include "FATManager.h"

FATManager::FATManager(string drive)
	: m_drive(drive)
{

}

FATManager::~FATManager() {

}

void FATManager::readPartitionTable() {
	for (int i = 0; i < 4; i++){
		/* Read state */
		read(
			(PARTITION_ENTRY1_OFFSET + 16*i + PARTITION_STATE_OFFSET)
			, PARTITION_STATE_LEN
			, &m_partitionTable[i].state);

		/* Read in CHS beginning of partition */
		read(
			(PARTITION_ENTRY1_OFFSET + 16*i + PARTITION_BEG_HEAD_OFFSET)
			, PARTITION_BEG_HEAD_LEN
			, &m_partitionTable[i].beginHead);
		read(
			(PARTITION_ENTRY1_OFFSET + 16*i + PARTITION_BEG_CYL_SECTOR_OFFSET)
			, PARTITION_BEG_CYL_SECTOR_LEN
			, m_partitionTable[i].beginCylHead);

		/* Read type of partition */
		read(
			(PARTITION_ENTRY1_OFFSET + 16*i + PARTITION_TYPE_OFFSET)
			, PARTITION_TYPE_LEN
			, &m_partitionTable[i].type);

		/* Read in CHS end of partition */
		read(
			(PARTITION_ENTRY1_OFFSET + 16*i + PARTITION_END_HEAD_OFFSET)
			, PARTITION_END_HEAD_LEN
			, &m_partitionTable[i].endHead);
		read(
			(PARTITION_ENTRY1_OFFSET + 16*i + PARTITION_END_CYL_SECTOR_OFFSET)
			, PARTITION_END_CYL_SECTOR_LEN
			, m_partitionTable[i].endCylHead);

		/* Read number of sectors between MBR and partition */
		read(
			(PARTITION_ENTRY1_OFFSET + 16*i + SECTORS_BETWEEN_MBR_PART_OFFSET)
			, SECTORS_BETWEEN_MBR_PART_LEN
			, m_partitionTable[i].sectorsBetweenMBR);

		/* Read size of partition in sectors */
		read(
			(PARTITION_ENTRY1_OFFSET + 16*i + PARTITION_SECTOR_SIZE_OFFSET)
			, PARTITION_SECTOR_SIZE_LEN
			, m_partitionTable[i].sizeInSectors);
	}
}

void FATManager::writeBPB() {

	m_bpb.bootCode[0]
	write(
		MBC_OFFSET
		,MBC_LEN
		,)

}



void FATManager::read(streamoff offset,
		unsigned long long length,
		char* dest) {

	ifstream drive;
	drive.open(m_drive.c_str(), ios::binary|ios::in);

	if (!drive.good()){
		cout << "Could not open drive" << endl;
		return;
	}

	drive.seekg(offset);

	for (unsigned int i = 0; i < length; i++){
		drive.read(&(dest[i]), 1);
	}

	drive.close();
}

void FATManager::write(streamoff offset, unsigned long long length, char* src) {
	fstream drive;
	drive.open(m_drive.c_str(), ios::binary|ios::in|ios::out);

	if (!drive.good()){
		cout << "Could not open drive" << endl;
		return;
	}

	drive.seekp(offset);

	for (unsigned int i = 0; i < length; i++){
		drive.write(&(src[i]), 1);
	}

	drive.close();
}
