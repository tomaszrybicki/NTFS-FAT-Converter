/*
 * DirectoryTable.cpp
 *
 *  Created on: Dec 15, 2017
 *      Author: root
 */

#include "DirectoryTable.h"
#include <cstring>
#include "FATManager.h"

DirectoryTable::DirectoryTable(std::streamoff location
		, FATManager* manager)
	: m_location(location)
	, m_manager(manager)
{
}

DirectoryTable::~DirectoryTable() {
}

Entry::Entry(byte_t* filename
		, byte_t* extension
		, uint32_t firstCluster
		, uint32_t fileLength){
	strncpy((char*)m_shortFilename, (char*)filename, 8);
	strncpy((char*)m_extension, (char*)extension, 3);
	m_fileLength = fileLength;

	/* Set first cluster of file in separate fields */
	byte_t* src = (byte_t*)&firstCluster;
	m_firstClusterHigh[1] = src[3];
	m_firstClusterHigh[0] = src[2];
	m_firstClusterLow[1] = src[1];
	m_firstClusterLow[0] = src[0];
}

Entry::~Entry() {
}

bool DirectoryTable::createEntry(byte_t* filename
		, byte_t* extension
		, uint32_t firstCluster
		, uint32_t fileLength){

	/* If directory table is full, return false */
	if ((m_entries.size() * 4) >= (m_manager->m_sectorsPerCluster*m_manager->m_bytesPerSector)){
		return false;
	}

	Entry tmp(filename, extension, firstCluster, fileLength);
	m_entries.push_back(tmp);
	return true;
}

void DirectoryTable::sync() {

	cout << "\t[FAT32] Writing directory table at offset: " << std::hex << m_location << " to disk..." << endl;

	byte_t zero[30] ={0};
	byte_t date[2] = { 0b00100001, 0};

	for (unsigned int i = 0; i < m_entries.size(); i++){

		/* Write filename */
		streamoff offset = m_location + (i * 32);
		m_manager->write(offset, 8, m_entries[i].m_shortFilename);

		/* Write extension */
		offset += 8;
		m_manager->write(offset, 3, m_entries[i].m_extension);

		/* Write attributes */
		offset += 3;
		m_manager->write(offset, 1, zero);

		/* Write user attributes */
		offset += 1;
		m_manager->write(offset, 1, zero);

		/* Write create time in miliseconds */
		offset += 1;
		m_manager->write(offset, 1, zero);

		/* Write create time hour */
		offset += 1;
		m_manager->write(offset, 2, zero);

		/* Write create date */
		offset += 2;
		m_manager->write(offset, 2, date);

		/* Write access date */
		offset += 2;
		m_manager->write(offset, 2, date);

		/* Write high 2 bytes of first cluster of file */
		offset += 2;
		m_manager->write(offset, 2, m_entries[i].m_firstClusterHigh);

		/* Write last modified hour */
		offset += 2;
		m_manager->write(offset, 2, zero);

		/* Write last modified date */
		offset += 2;
		m_manager->write(offset, 2, date);

		/* Write low 2 bytes of first cluster of file */
		offset += 2;
		m_manager->write(offset, 2, m_entries[i].m_firstClusterLow);

		/* Write file length (size in bytes) */
		offset += 2;
		m_manager->write(offset, 4, (byte_t*)&m_entries[i].m_fileLength);

	}
}
