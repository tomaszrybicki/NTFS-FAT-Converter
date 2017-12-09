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
	// first get from ntfs number of sectors, count fat size, root directory location

	/* Create MBC (jmp and nop) */
	m_bpb.bootCode[0] = 0xEB;
	m_bpb.bootCode[1] = 0x58;
	m_bpb.bootCode[2] = 0x90;
	write(MBC_OFFSET, MBC_LEN, m_bpb.bootCode);

	/* OEM = 160405 */
	write(OEM_NAME_OFFSET, OEM_NAME_LEN, m_bpb.OEMName);

	/* 512B per sector */
	m_bpb.bytesPerSector[0] = 0x00;
	m_bpb.bytesPerSector[1] = 0x02;
	write(BYTES_PER_SECTOR_OFFSET, BYTES_PER_SECTOR_LEN, m_bpb.bytesPerSector);

	/* 4kiB clusters */
	m_bpb.sectorsPerCluster = 0x08;
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
	// TMP!!
	m_bpb.numberOfSectors[0] = 0x00; m_bpb.numberOfSectors[1] = 0xF8;
	m_bpb.numberOfSectors[2] = 0x0F; m_bpb.numberOfSectors[3] = 0x00;
	write(NUMBER_OF_SECTORS_OFFSET, NUMBER_OF_SECTORS_LEN, m_bpb.numberOfSectors);

	/* FAT size in sectors - depending on source partition */
	// TMP!!
	m_bpb.fatSectorSize[0] = 0xFC; m_bpb.fatSectorSize[1] = 0x03;
	m_bpb.fatSectorSize[2] = 0x00; m_bpb.fatSectorSize[3] = 0x00;
	write(FAT_SECTOR_SIZE_OFFSET, FAT_SECTOR_SIZE_LEN, m_bpb.fatSectorSize);

	/* Flags: - disable mirroring, set first FAT as active */
	m_bpb.flags[0] = 0b10000000;
	m_bpb.flags[1] = 0x00;
	write(FLAGS_OFFSET, FLAGS_LEN, m_bpb.flags);

	/* Drive version - zeroed */
	m_bpb.driveVersion[0] = 0x0;
	m_bpb.driveVersion[1] = 0x0;
	write(DRIVE_VERSION_OFFSET, DRIVE_VERSION_LEN, m_bpb.driveVersion);

	/* Root directory cluster number - depending on FATs size */
	//TMP!!
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

	/* Logical drive number - may cause errors if duplicated */
	m_bpb.logicalDriveNumber = 0x90;
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

void FATManager::writeFSInfo() {
	streamoff FSinfoOffset = m_bpb.fsinfoSector[1];
	FSinfoOffset = FSinfoOffset << BYTE;
	FSinfoOffset += m_bpb.fsinfoSector[0];
	FSinfoOffset *= SECTOR_SIZE;


	write(FSinfoOffset + FSINFO_SIGNATURE1_OFFSET, FSINFO_SIGNATURE1_LEN, m_fsInfo.signature1);

	write(FSinfoOffset + FSINFO_SIGNATURE2_OFFSET, FSINFO_SIGNATURE2_LEN, m_fsInfo.signature2);

	write(FSinfoOffset + FSINFO_FREE_CLUSTERS_OFFSET, FSINFO_FREE_CLUSTERS_LEN, (char*)m_fsInfo.freeClusters);

	write(FSinfoOffset + FSINFO_RECENT_CLUSTER_OFFSET, FSINFO_RECENT_CLUSTER_LEN, (char*)m_fsInfo.recentCluster);

	write(FSinfoOffset + FSINFO_SIGNATURE3_OFFSET, FSINFO_SIGNATURE3_LEN, (char*)m_fsInfo.signature3);
}

void FATManager::writeFATs() {
	streamoff FAToffset;
	unsigned int reservedSectors = m_bpb.reservedSectors[1];
	reservedSectors = reservedSectors << BYTE;
	reservedSectors += m_bpb.reservedSectors[0];

	FAToffset = reservedSectors * SECTOR_SIZE;




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
