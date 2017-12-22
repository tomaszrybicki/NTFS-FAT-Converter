/*
 * DirectoryTable.h
 *
 *  Created on: Dec 15, 2017
 *      Author: Tomasz Rybicki
 */

#ifndef DIRECTORYTABLE_H_
#define DIRECTORYTABLE_H_

#include <vector>
#include <fstream>
#include "DefinesFAT.h"

class FATManager;

class Entry{
public:
	Entry(byte_t* filename
			, byte_t* extension
			, uint32_t firstCluster
			, uint32_t fileLength);
	~Entry();

	byte_t m_shortFilename[8] = {};
	byte_t m_extension[3] = {};
	byte_t m_attributes = 0;
	byte_t m_userAttributes = 0;
	byte_t m_createTimeMilisec = 0;
	byte_t m_createTimeHour[2] = {};
	byte_t m_createDate[2] = {};
	byte_t m_accessDate[2] = {};
	byte_t m_firstClusterHigh[2] = {};
	byte_t m_lastModifiedHour[2] = {};
	byte_t m_lastModifiedDate[2] = {};
	byte_t m_firstClusterLow[2] = {};
	uint32_t m_fileLength = 0;
};


class DirectoryTable {
public:
	DirectoryTable(std::streamoff location, FATManager* manager);
	virtual ~DirectoryTable();

	/* Adds new entry */
	bool createEntry(byte_t* filename, byte_t* extension, uint32_t firstCluster, uint32_t fileLength);

	/* Writes directory table content to disk */
	void sync();

	std::vector<Entry> m_entries;
	std::streamoff m_location;
	FATManager* m_manager;
};

#endif /* DIRECTORYTABLE_H_ */
