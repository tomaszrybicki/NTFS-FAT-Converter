/*
 * NTFSManager.cpp
 *
 *  Created on: Dec 17, 2017
 *      Author: Tomasz Rybicki
 */

#include "NTFSManager.h"

using namespace std;

NTFSManager::NTFSManager(string part, bool readDeleted)
	: m_partition(part)
	, m_sectorsPerCluster(0)
	, m_fatManager(0)
	, m_readDeleted(readDeleted)
{

}

NTFSManager::~NTFSManager() {
}

void NTFSManager::readBPB() {
	/* Read bytes per sector */
	read(BYTES_PER_SECTOR_OFFSET, BYTES_PER_SECTOR_LEN, m_bytesPerSector);

	/* Read Sectors per cluster */
	read(SECTORS_PER_CLUSTER_OFFSET, SECTORS_PER_CLUSTER_LEN, &m_sectorsPerCluster);

	m_bytesPerCluster = string2int(m_bytesPerSector, 2) * m_sectorsPerCluster;

	/* Read reserved sectors */
	read(RESERVED_SECTORS_OFFSET, RESERVED_SECTORS_LEN, m_reservedSectors);

	/* Read total sectors of partition */
	read(0x28, 8, m_totalSectors);

	/* Read $MFT Logical Cluster Number */
	uint64_t MftLcn = 0;
	read(0x30, 8, (byte_t*)&MftLcn);
	m_MftLcns.push_back(MftLcn);

	/* Read clusters per file record segment */
	read(0x40, 4, m_clustersPerRecord);
}

void NTFSManager::readMFT() {

	/* Iterate over MFT entries */
	int i = 0;
	streamoff entryOffset = m_MftLcns[0] * m_sectorsPerCluster * string2int(m_bytesPerSector,2);
	streamoff fieldOffset = 0;
	streamoff firstAttributeOff = 0;
	uint32_t entrySize = 0;
	uint32_t FILEsignature = 1;
	uint32_t attrType = 0;
	uint32_t attrLen = 0;
	bool deleted = false;

	File file = {};

	/* Temporary bytes for reading */
	byte_t bytes2[2];

	/* For every $MFT cluster */
	int VCN = 0;
	for (uint32_t j = 0; j < m_MftLcns.size(); j++){

		VCN++;
		entryOffset = m_MftLcns[j] * m_bytesPerCluster;

		/* While not end of MFT cluster or there are entries */
		while(FILEsignature && ((entryOffset + fieldOffset)
				< ((m_MftLcns[j] * m_bytesPerCluster) + m_bytesPerCluster))){

			/* Read entry header */
			read(entryOffset + fieldOffset, 4, (byte_t*)&FILEsignature);

			/* If no signature - no more files in this cluster */
			if (!FILEsignature){
				break;
			}

			/* Read first attribute offset */
			fieldOffset = 0x14;
			read(entryOffset + fieldOffset, 2, bytes2);
			firstAttributeOff = string2int(bytes2, 2);

			/* Read MFT entry size (allocated) */
			fieldOffset = 0x1C;
			read(entryOffset + fieldOffset, 4, (byte_t*)&entrySize);

			/* See if file is deleted - bit in flags field*/
			deleted = false;
			fieldOffset = 0x16;
			read(entryOffset + fieldOffset, 2, bytes2);
			if (! (bytes2[0] & 0b00000001)){
				deleted = true;
			}


			/* Look for interesting attributes */
			fieldOffset = firstAttributeOff;

			while(true){

				/* Get attribute type */
				read(entryOffset + fieldOffset, 4, (byte_t*)&attrType);

				/* End of record - 0xFFFFFFFF, some files have bad 0xb0 attribute length
				 * hence the 0x00 check */
				if (attrType == 0xFFFFFFFF || attrType == 0x00000000){
					break;
				}

				/* Get attribute length */
				read(entryOffset + fieldOffset + 4, 4, (byte_t*)&attrLen);


				/* See if attribute is interesting - handle if necessary */
				handleAttribute(attrType, entryOffset + fieldOffset, file);


				fieldOffset += attrLen;
			}

			/* If file is not system file or empty file then copy it */
			cout << std::dec << "\n[NTFS] $MFT's VCN: " << VCN << " Found file: \'"
					<< file.name << "\'\tSize: " << std::dec << file.length << endl;

			if (file.name[0] != '$' && file.name[0] != '\0' && file.length != 0){
				if (deleted && !m_readDeleted){
					cout << "\t[NTFS] File is flagged as deleted. Use --deleted flag for copying it..." << endl;
				} else {
					m_fatManager->writeFile(file.data, file.name, file.extension, file.length);
				}

			}else{
				cout << "\t[FAT32] Empty or system file, omitting..." << endl;
			}

			fieldOffset = 0;
			entryOffset += entrySize;
			i++;
			file.data.clear();
			file.length = 0;
			strncpy((char*)file.name, "\0\0\0\0\0\0\0\0\0\0\0", 11);
			strncpy((char*)file.extension, "\0\0\0", 3);
		}
	}
}

void NTFSManager::handleAttribute(uint32_t attrType, streamoff attrOffset, File &file) {

	uint32_t attrLen = 0;
	byte_t nonResidentFlag;
	byte_t nameLen;

	streamoff attrContentOff;
	byte_t bytes2[2];
	bool extensionPart = false;
	int extWritten = 0;
	uint64_t fileLen = 0;


	switch(attrType){

		/* Filename attribute */
		case 0x30:

			/*******************		ATTRIBUTE HEADER		*******************/
			/* See if resident */
			read(attrOffset + 0x08, 1, &nonResidentFlag);

			/* TODO: handle non-resident attribute */
			if(nonResidentFlag){
				cout << "\t [WARNING] Filename attribute is non resident, reading functionality not implemented!" << endl;
				return;
			}

			/* Get attribute header name length - 0 if not named */
			read(attrOffset + 0x09, 1, &nameLen);

			/* Get attributes actual content offset */
			read(attrOffset + 0x14, 2,  bytes2);
			attrContentOff = 2 * nameLen + string2int(bytes2, 2);
			attrOffset += attrContentOff;


			/*******************		ATTRIBUTE CONTENT		*******************/
			/* Read file length - now attrOffset points to begging of attribute content */
			fileLen = 0;
			read(attrOffset + 0x30, 8, (byte_t*)&fileLen);

			/* File above 4GB */
			if (fileLen >= 0xFFFFFFFF){
				cout << "\t[WARNING] File is above 4GB, cannot copy due to FAT32 limitations" << endl;
			}else{
				file.length = fileLen;
			}

			/* Read file name length in characters*/
			byte_t nameLen;
			read(attrOffset + 0x40, 1, &nameLen);

			/* Read extension and file name and convert it to
			 * eleven asci chars (FAT) from unicode */

			byte_t ntfsName[256];
			read(attrOffset + 0x42, nameLen*2, ntfsName);

			extensionPart = false;
			extWritten = 0;
			for (int i = 0; i < nameLen; i++){

				if (ntfsName[2*i] == '.'){
					extensionPart = true;
				}

				/* If we still have place and arent looking at extension */
				if (i < 11 && !extensionPart){
					file.name[i] = ntfsName[2*i];
				}

				/* After dot in filename */
				if (extensionPart){
					if(ntfsName[2*i] == '.'){
						continue;
					}

					file.extension[extWritten] = ntfsName[2*i];
					extWritten++;

					/* Filled extension */
					if(extWritten >=3){
						break;
					}
				}

			}

			break;



		/* Data attribute */
		case 0x80:

			/*******************		ATTRIBUTE HEADER		*******************/
			/* Get attribute length */
			read(attrOffset + 0x04, 4, (byte_t*)&attrLen);

			/* See if resident */
			read(attrOffset + 0x08, 1, &nonResidentFlag);

			/* Handle non-resident attribute */
			if(nonResidentFlag){

				/* Get offset to data runs - actual attribute content */
				read(attrOffset + 0x20, 2, bytes2);
				attrContentOff = string2int(bytes2, 2);

				/* Get first data run header - a byte where
				 * bits 4-7 are number of bytes for starting LCN number of data run
				 * bits 0-3 are number of bytes for cluster count of run*/
				byte_t header;
				byte_t bytesForLCN = 0;
				byte_t bytesForClusterCount = 0;
				byte_t startingLCN[8];
				byte_t clusterCount[8];
				uint32_t runsCount = 0;
				uint32_t clustersSum = 0;

				read(attrContentOff + attrOffset, 1, &header);

				/* While there are data runs */
				while (header != 0x00){
					bytesForClusterCount = header & 0b00001111;
					bytesForLCN = (header & 0b11110000) >> 4;

					/* Undocumented pattern */
					if(bytesForClusterCount == 0 || bytesForLCN == 0){
						break;
					}

					/* Read cluster count */
					attrContentOff += 1;
					read(attrOffset + attrContentOff, bytesForClusterCount, clusterCount);

					/* Read starting LCN */
					attrContentOff += bytesForClusterCount;
					read(attrOffset + attrContentOff, bytesForLCN, startingLCN);

					/* Read header */
					attrContentOff += bytesForLCN;
					read(attrOffset + attrContentOff, 1, &header);

					/* If it's MFT file, add it's LCNs to m_mftLcns */
					if (!m_mftLcnsRead &&
							file.name[0] == '$' &&
							file.name[1] == 'M' &&
							file.name[2] == 'F' &&
							file.name[3] == 'T' &&
							file.name[4] == 0 ){

						for (uint32_t i = 0; i < string2long(clusterCount, bytesForClusterCount); i++){
							uint64_t mftLcn = string2long(startingLCN, bytesForLCN) + i;
							m_MftLcns.push_back(mftLcn);
						}


					}else{
						/* Read data from each cluster of  data run */
						Cluster tmp(m_sectorsPerCluster * string2int(m_bytesPerSector, 2));
						for (uint32_t i = 0; i < string2long(clusterCount, bytesForClusterCount); i++){
							read((string2long(startingLCN, bytesForLCN) + i) * string2int(m_bytesPerSector,2) * m_sectorsPerCluster
									, string2int(m_bytesPerSector, 2) * m_sectorsPerCluster
									, tmp.m_data);

							file.data.push_back(tmp);
						}
					}

					runsCount++;
					clustersSum += string2long(clusterCount, bytesForClusterCount);
				}

				if (file.name[0] == '$' &&
					file.name[1] == 'M' &&
					file.name[2] == 'F' &&
					file.name[3] == 'T' &&
					file.name[4] == 0 ){
					m_mftLcnsRead = true;
				}

			/* Handle resident attribute */
			}else{

			/* Get attribute header name length - 0 if not named */
			read(attrOffset + 0x09, 1, &nameLen);

			/* Get attributes actual content offset */
			read(attrOffset + 0x14, 2,  bytes2);
			attrContentOff = 2 * nameLen + string2int(bytes2, 2);


			/*******************		ATTRIBUTE CONTENT		*******************/
			/* Read data */
			Cluster data(string2int(m_bytesPerSector, 2) * m_sectorsPerCluster);
			uint32_t bytesToRead = attrLen - attrContentOff;
			read(attrOffset + attrContentOff, bytesToRead, data.m_data);

			file.data.push_back(data);

			}

		break;
	}
}

void NTFSManager::read(streamoff offset,
		uint32_t length,
		byte_t* dest) {

	ifstream drive;
	drive.open(m_partition.c_str(), ios::binary|ios::in);

	if (!drive.good()){
		cout << "Could not open drive" << endl;
		return;
	}

	drive.seekg(offset);

	for (unsigned int i = 0; i < length; i++){
		drive.read((char*)&(dest[i]), 1);
	}

	drive.close();
}

void NTFSManager::getFatManager(FATManager* fatManager) {
	m_fatManager = fatManager;

	/* Create FAT32 partition structures */
	fatManager->writeBPB();
	fatManager->writeFSInfo();
}

uint32_t NTFSManager::string2int(byte_t* src, uint32_t n) {
	uint32_t result = 0;

	for (int i = n-1; i >= 0; i--){
		result = result << 8;
		result += src[i];
	}

	return result;
}


uint64_t NTFSManager::string2long(byte_t* src, uint32_t n) {
	uint64_t result = 0;

	for (int i = n-1; i >= 0; i--){
		result = result << 8;
		result += src[i];
	}

	return result;
}

