/*
 * FATManager.cpp
 *
 *  Created on: Dec 9, 2017
 *  Author: Tomasz Rybicki
 */

#include "FATManager.h"


FATManager::FATManager(string drive
		, uint32_t bytesPerSector
		, uint32_t sectorsPerCluster
		, uint64_t neededSectors)
	: m_drive(drive)
	, m_bytesPerSector(bytesPerSector)
	, m_sectorsPerCluster(sectorsPerCluster)
	, m_reservedSectors(32)
	, m_fatSize((((neededSectors / sectorsPerCluster)*4) / bytesPerSector) - 1)
	, m_fat1Offset(m_bytesPerSector * m_reservedSectors)
	, m_fat2Offset(m_fat1Offset + m_fatSize * m_bytesPerSector)
	, m_dataClustersOffset(m_fat2Offset + m_fatSize * m_bytesPerSector)
	, m_fat1(m_fat1Offset, m_sectorsPerCluster, m_bytesPerSector, m_fatSize, this)
	, m_fat2(m_fat2Offset, m_sectorsPerCluster, m_bytesPerSector, m_fatSize, this)
{
	DirectoryTable tmp(m_fat2Offset + (m_fatSize * m_bytesPerSector), this);
	m_rootDirectory.push_back(tmp);
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

	/* Create MBC (jmp and nop) */
	m_bpb.bootCode[0] = 0xEB;
	m_bpb.bootCode[1] = 0x58;
	m_bpb.bootCode[2] = 0x90;
	write(MBC_OFFSET, MBC_LEN, m_bpb.bootCode);

	/* OEM = 160405 */
	write(OEM_NAME_OFFSET, OEM_NAME_LEN, m_bpb.OEMName);

	/* 512B per sector */
	m_bpb.bytesPerSector[0] = ((byte_t*)(&m_bytesPerSector))[0];
	m_bpb.bytesPerSector[1] = ((byte_t*)(&m_bytesPerSector))[1];
	write(BYTES_PER_SECTOR_OFFSET, BYTES_PER_SECTOR_LEN, m_bpb.bytesPerSector);

	/* 4kiB clusters */
	m_bpb.sectorsPerCluster = m_sectorsPerCluster;
	write(SECTORS_PER_CLUSTER_OFFSET, SECTORS_PER_CLUSTER_LEN, &m_bpb.sectorsPerCluster);

	/* 32 reserved sectors */
	m_bpb.reservedSectors[0] = 0x20;
	m_bpb.reservedSectors[1] = 0x00;
	write(RESERVED_SECTORS_OFFSET, RESERVED_SECTORS_LEN, m_bpb.reservedSectors);

	/* 2 FAT copies */
	m_bpb.fatCopiesCount = 0x02;
	write(FAT_COPIES_COUNT_OFFSET, FAT_COPIES_COUNT_LEN, &m_bpb.fatCopiesCount);

	write(MAX_ROOT_ENTRIES_OFFSET, MAX_ROOT_ENTRIES_LEN, m_bpb.maxRootEntries);

	write(SECTORS_SMALL_PART_OFFSET, SECTORS_SMALL_PART_LEN, m_bpb.sectorsSmallPart);

	/* Media descriptor - hard drive */
	m_bpb.mediaDescriptor = 0xF8;
	write(MEDIA_DESCRIPTOR_OFFSET, MEDIA_DESCRIPTOR_LEN, &m_bpb.mediaDescriptor);

	write(UNUSED1_OFFSET, UNUSED1_LEN, m_bpb.unused1);

	/* 63 sectors per track */
	m_bpb.sectorsPerTrack[0] = 0x3F;
	m_bpb.sectorsPerTrack[1] = 0x00;
	write(SECTORS_PER_TRACK_OFFSET, SECTORS_PER_TRACK_LEN, m_bpb.sectorsPerTrack);

	/* 255 heads */
	m_bpb.numberOfHeads[0] = 0xFF;
	m_bpb.numberOfHeads[1] = 0x00;
	write(NUMBER_OF_HEADS_OFFSET, NUMBER_OF_HEADS_LEN, m_bpb.numberOfHeads);

	/* 8 hidden sectors */
	m_bpb.numberOfHiddenSectors[0] = 0x00;
	m_bpb.numberOfHiddenSectors[1] = 0x08;
	m_bpb.numberOfHiddenSectors[2] = 0x00;
	m_bpb.numberOfHiddenSectors[3] = 0x00;
	write(NUMBER_OF_HIDDEN_SECTORS_OFFSET, NUMBER_OF_SECTORS_LEN, m_bpb.numberOfHiddenSectors);

	/* Number of sectors - depending on source partition */
	uint32_t numberOfSectors = ((m_fatSize * m_bytesPerSector) / 4) * m_sectorsPerCluster;
	strncpy((char*)m_bpb.numberOfSectors, (char*)&numberOfSectors, 4);
	write(NUMBER_OF_SECTORS_OFFSET, NUMBER_OF_SECTORS_LEN, m_bpb.numberOfSectors);

	/* FAT size in sectors - depending on source partition */
	strncpy((char*)m_bpb.fatSectorSize, (char*)&m_fatSize, 4);
	write(FAT_SECTOR_SIZE_OFFSET, FAT_SECTOR_SIZE_LEN, m_bpb.fatSectorSize);

	/* Flags: - disable mirroring, set first FAT as active */
	m_bpb.flags[0] = 0b10000000;
	m_bpb.flags[1] = 0x00;
	write(FLAGS_OFFSET, FLAGS_LEN, m_bpb.flags);

	/* Drive version - zeroed */
	m_bpb.driveVersion[0] = 0x00;
	m_bpb.driveVersion[1] = 0x00;
	write(DRIVE_VERSION_OFFSET, DRIVE_VERSION_LEN, m_bpb.driveVersion);

	/* Root directory cluster number */
	m_bpb.rootDirectoryCluster[0] = 0x02;
	write(ROOT_DIRECTORY_CLUSTER_OFFSET, ROOT_DIRECTORY_CLUSTER_LEN, m_bpb.rootDirectoryCluster);

	/* FS info in sector 1. */
	m_bpb.fsinfoSector[0] = 0x01;
	m_bpb.fsinfoSector[1] = 0x00;
	write(FSINFO_SECTOR_OFFSET, FSINFO_SECTOR_LEN, m_bpb.fsinfoSector);

	/* Backup VBR sector location - none */
	m_bpb.backupVBRSector[0] = 0x00;
	m_bpb.backupVBRSector[1] = 0x00;
	write(BACKUP_VBR_SECTOR_OFFSET, BACKUP_VBR_SECTOR_LEN, m_bpb.backupVBRSector);

	/* Reserved bytes */
	write(RESERVED_OFFSET, RESERVED_LEN, m_bpb.reserved);

	/* Logical drive number */
	m_bpb.logicalDriveNumber = 0x80;
	write(LOGICAL_DRIVE_NUM_OFFSET, LOGICAL_DRIVE_NUM_LEN, &m_bpb.logicalDriveNumber);

	/* Unused */
	write(UNUSED2_OFFSET, UNUSED1_LEN, &m_bpb.unused2);

	/* Extended signature */
	m_bpb.extendedSignature = 0x29;
	write(EXTENDED_SIGNATURE_OFFSET, EXTENDED_SIGNATURE_LEN, &m_bpb.extendedSignature);

	/* Serial number */
	m_bpb.serialNumber[0] = 0x13;
	m_bpb.serialNumber[1] = 0x37;
	m_bpb.serialNumber[2] = 0x13;
	m_bpb.serialNumber[3] = 0x37;
	write(SERIAL_NUMBER_OFFSET, SERIAL_NUMBER_LEN, m_bpb.serialNumber);

	/* Volume name */
	write(VOLUME_NAME_OFFSET, VOLUME_NAME_LEN, m_bpb.volumeName);

	/* FAT name */
	write(FAT_NAME_OFFSET, FAT_NAME_LEN, m_bpb.fatName);

	/* Rest of boot code */
	write(CODE_OFFSET, CODE_LEN, m_bpb.code);

	/* VBR signature  */
	m_bpb.VBRsignature[0] = 0x55;
	m_bpb.VBRsignature[1] = 0xAA;
	write(VBR_SIGNATURE_OFFSET, VBR_SIGNATURE_LEN, m_bpb.VBRsignature);
}




void FATManager::read(streamoff offset,
		uint32_t length,
		byte_t* dest) {

	ifstream drive;
	drive.open(m_drive.c_str(), ios::binary|ios::in);

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

void FATManager::writeFSInfo() {
	streamoff FSinfoOffset = m_bpb.fsinfoSector[1];
	FSinfoOffset = FSinfoOffset << BYTE;
	FSinfoOffset += m_bpb.fsinfoSector[0];
	FSinfoOffset *= m_bytesPerSector;


	write(FSinfoOffset + FSINFO_SIGNATURE1_OFFSET, FSINFO_SIGNATURE1_LEN, m_fsInfo.signature1);

	write(FSinfoOffset + FSINFO_SIGNATURE2_OFFSET, FSINFO_SIGNATURE2_LEN, m_fsInfo.signature2);

	write(FSinfoOffset + FSINFO_FREE_CLUSTERS_OFFSET, FSINFO_FREE_CLUSTERS_LEN, (byte_t*)m_fsInfo.freeClusters);

	write(FSinfoOffset + FSINFO_RECENT_CLUSTER_OFFSET, FSINFO_RECENT_CLUSTER_LEN, (byte_t*)m_fsInfo.recentCluster);

	write(FSinfoOffset + FSINFO_SIGNATURE3_OFFSET, FSINFO_SIGNATURE3_LEN, (byte_t*)m_fsInfo.signature3);
}


void FATManager::write(streamoff offset, uint32_t length, byte_t* src) {
	fstream drive;
	drive.open(m_drive.c_str(), ios::binary|ios::in|ios::out);

	if (!drive.good()){
		cout << "Could not open drive" << endl;
		return;
	}

	drive.seekp(offset);

	for (unsigned int i = 0; i < length; i++){
		drive.write((char*)&(src[i]), 1);
	}

	drive.close();
}

uint32_t FATManager::string2int(byte_t* src, uint32_t n) {
	uint32_t result = 0;

	for (int i = n-1; i >= 0; i--){
		result = result << 8;
		result += src[i];
	}

	return result;
}

uint32_t FATManager::clusterCeil(
		uint32_t offset) {
	int modulo = offset % (m_bpb.sectorsPerCluster * string2int(m_bpb.bytesPerSector,2));

	/* If offset is beginning of cluster then return offset */
	if(!modulo){
		return offset;
	}

	/* Otherwise return next cluster offset */
	return (offset / (m_bpb.sectorsPerCluster * string2int(m_bpb.bytesPerSector,2)))
			+ (m_bpb.sectorsPerCluster * string2int(m_bpb.bytesPerSector,2));
}

void FATManager::writeFile(list<Cluster> data
		, byte_t* filename
		, byte_t* extension
		, uint32_t fileLength)
{
	/* Write to FATs file contents */
	uint32_t firstCluster;

	cout << "\t[FAT32] Copying file contents..." << endl;
	firstCluster = m_fat1.writeFile(data);
	firstCluster = m_fat2.writeFile(data);
	m_fat1.sync();
	m_fat2.sync();

	if(!firstCluster){
		cout << "Couldn't write file: " << filename << " Couldnt  find free cluster " << endl;
	}

	/* Create directory entry for file */
	for (uint32_t i = 0; i < m_rootDirectory.size(); i++){

		if (m_rootDirectory[i].createEntry(filename, extension, firstCluster, fileLength)){
			m_rootDirectory[i].sync();
			return;
		}

	}

	/* No more place for directory table entry - allocating new cluster */
	streamoff lastDirectoryOffset = m_rootDirectory.back().m_location;
	uint32_t lastDirectoryClusterNumber = ((lastDirectoryOffset - m_dataClustersOffset)
			/ (m_sectorsPerCluster * m_bytesPerSector) + 2);

	m_fat1.extendFile(lastDirectoryClusterNumber);
	uint32_t newCluster = m_fat2.extendFile(lastDirectoryClusterNumber);

	DirectoryTable tmp(m_dataClustersOffset + ((newCluster-2)*m_bytesPerSector*m_sectorsPerCluster), this);
	m_rootDirectory.push_back(tmp);
}
