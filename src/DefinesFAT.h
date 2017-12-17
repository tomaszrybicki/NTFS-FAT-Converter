/*
 * DefinesFAT.h
 *
 *  Created on: Dec 9, 2017
 *  Author: Tomasz Rybicki
 *
 *  Description: defines describing positions
 *  of fields in MBR and VBR.
 *  Source: https://www.easeus.com/resource/fat32-disk-structure.htm
 */

#ifndef DEFINESFAT_H_
#define DEFINESFAT_H_

#include <list>
#include <cstring>


#define DWORD 4
#define WORD 2
#define BYTE 1


/* FAT32 MBR
 * Offsets from beginning of disks's sector 0.
 */
#define MBC_OFFSET 	0x00
#define MBC_LEN		446

#define PARTITION_ENTRY1_OFFSET 	0x01BE
#define PARTITION_ENTRY1_LEN		16

#define PARTITION_ENTRY2_OFFSET 	0x01CE
#define PARTITION_ENTRY2_LEN		16

#define PARTITION_ENTRY3_OFFSET 	0x01DE
#define PARTITION_ENTRY3_LEN		16

#define PARTITION_ENTRY4_OFFSET 	0x01EE
#define PARTITION_ENTRY4_LEN		16

/* 0x55 0xAA - signature */
#define MBR_SIGNATURE_OFFSET	0x01FE
#define MBR_SIGNATURE_LEN		WORD



/* FAT32 Partition table entry
 * Offsets from beginning of partition entry
 */
/* Current State of Partition(00h=Inactive, 80h=Active) */
#define PARTITION_STATE_OFFSET 	0x00
#define PARTITION_STATE_LEN 	BYTE

#define PARTITION_BEG_HEAD_OFFSET 	0x01
#define PARTITION_BEG_HEAD_LEN		BYTE

/* Encoded */
#define PARTITION_BEG_CYL_SECTOR_OFFSET	0x02
#define PARTITION_BEG_CYL_SECTOR_LEN 	WORD

#define PARTITION_TYPE_OFFSET 	0x04
#define PARTITION_TYPE_LEN		BYTE

#define PARTITION_END_HEAD_OFFSET 	0x05
#define PARTITION_END_HEAD_LEN		BYTE

/* Encoded */
#define PARTITION_END_CYL_SECTOR_OFFSET	0x06
#define PARTITION_END_CYL_SECTOR_LEN 	WORD

#define SECTORS_BETWEEN_MBR_PART_OFFSET	0x08
#define	SECTORS_BETWEEN_MBR_PART_LEN DWORD

#define PARTITION_SECTOR_SIZE_OFFSET	0x0C
#define PARTITION_SECTOR_SIZE_LEN		DWORD

/* Code is for Linux native partition */
#define TYPE_FAT32 0x83




/* FAT32 VBR
 * Offsets from beginning of partition's sector 0.
 */
#define VBR_CODE_OFFSET	0x00
#define VBR_CODE_LEN	3

#define OEM_NAME_OFFSET 0x03
#define OEM_NAME_LEN	8

#define BYTES_PER_SECTOR_OFFSET 0x0B
#define BYTES_PER_SECTOR_LEN WORD

#define SECTORS_PER_CLUSTER_OFFSET 0x0D
#define SECTORS_PER_CLUSTER_LEN BYTE

#define RESERVED_SECTORS_OFFSET	0x00E
#define RESERVED_SECTORS_LEN	WORD

#define FAT_COPIES_COUNT_OFFSET 0x010
#define FAT_COPIES_COUNT_LEN	BYTE

/* Not relevant for FAT32 */
#define MAX_ROOT_ENTRIES_OFFSET 0x11
#define MAX_ROOT_ENTRIES_LEN 	WORD

/* Not relevant for FAT32 */
#define SECTORS_SMALL_PART_OFFSET 0x13
#define SECTORS_SMALL_PART_LEN WORD

#define MEDIA_DESCRIPTOR_OFFSET 0x15
#define MEDIA_DESCRIPTOR_LEN	BYTE

/* Not relevant for FAT32 */
#define UNUSED1_OFFSET	0x16
#define UNUSED1_LEN		WORD

#define SECTORS_PER_TRACK_OFFSET 0x18
#define SECTORS_PER_TRACK_LEN WORD

#define NUMBER_OF_HEADS_OFFSET 	0x1A
#define NUMBER_OF_HEADS_LEN		WORD

#define NUMBER_OF_HIDDEN_SECTORS_OFFSET	0x1C
#define NUMBER_OF_HIDDEN_SECTORS_LEN 	DWORD

#define NUMBER_OF_SECTORS_OFFSET	0x20
#define NUMBER_OF_SECTORS_LEN		DWORD

#define FAT_SECTOR_SIZE_OFFSET	0x024
#define FAT_SECTOR_SIZE_LEN		DWORD

#define FLAGS_OFFSET	0x28
#define FLAGS_LEN		WORD

/*  (HighByte = Major Version, Low Byte = Minor Version) */
#define DRIVE_VERSION_OFFSET 0x2A
#define DRIVE_VERSION_LEN	WORD

#define ROOT_DIRECTORY_CLUSTER_OFFSET 0x2C
#define ROOT_DIRECTORY_CLUSTER_LEN	DWORD

#define FSINFO_SECTOR_OFFSET	0x30
#define FSINFO_SECTOR_LEN		WORD

#define BACKUP_VBR_SECTOR_OFFSET 0x32
#define BACKUP_VBR_SECTOR_LEN	WORD

#define RESERVED_OFFSET	0x34
#define RESERVED_LEN	12

#define LOGICAL_DRIVE_NUM_OFFSET	0x40
#define LOGICAL_DRIVE_NUM_LEN		BYTE

#define UNUSED2_OFFSET	0x41
#define UNUSED2_LEN		BYTE

/* 29h */
#define EXTENDED_SIGNATURE_OFFSET 	0x42
#define EXTENDED_SIGNATURE_LEN		BYTE

#define SERIAL_NUMBER_OFFSET	0x43
#define SERIAL_NUMBER_LEN		DWORD

#define VOLUME_NAME_OFFSET	0x47
#define VOLUME_NAME_LEN		11

#define FAT_NAME_OFFSET	0x52
#define FAT_NAME_LEN	8

#define CODE_OFFSET 0x5A
#define CODE_LEN	420

#define VBR_SIGNATURE_OFFSET	0x01FE
#define VBR_SIGNATURE_LEN	 	2


/* FS information sector offsets */
#define FSINFO_SIGNATURE1_OFFSET 0x00
#define FSINFO_SIGNATURE1_LEN 4

#define FSINFO_SIGNATURE2_OFFSET 0x01E4
#define FSINFO_SIGNATURE2_LEN 4

#define FSINFO_FREE_CLUSTERS_OFFSET 0x01E8
#define FSINFO_FREE_CLUSTERS_LEN 4

#define FSINFO_RECENT_CLUSTER_OFFSET 0x01EC
#define FSINFO_RECENT_CLUSTER_LEN 4

#define FSINFO_SIGNATURE3_OFFSET 0x01FC
#define FSINFO_SIGNATURE3_LEN 4



/* Types and structures */
typedef unsigned int uint32_t;
//typedef unsigned long long uint64_t;
typedef unsigned char byte_t;

typedef struct{
	byte_t head;
	byte_t cylinderHead[2];
} CHS;

typedef unsigned long long LBA;

/* Complete partition table entry */
typedef struct{
	byte_t state;
	byte_t beginHead;
	byte_t beginCylHead[2];
	byte_t type;
	byte_t endHead;
	byte_t endCylHead[2];
	byte_t sectorsBetweenMBR[2];
	byte_t sizeInSectors[4];
} Partition;

typedef struct{
	byte_t bootCode[3];
	byte_t OEMName[8] = { '1', '6', '0', '4', '0', '5', 0, 0 };
	byte_t bytesPerSector[2];
	byte_t sectorsPerCluster;
	byte_t reservedSectors[2];
	byte_t fatCopiesCount;
	byte_t maxRootEntries[2] = { 0, 0 };
	byte_t sectorsSmallPart[2] = { 0, 0 };;
	byte_t mediaDescriptor;
	byte_t unused1[2]  = { 0, 0 };
	byte_t sectorsPerTrack[2];
	byte_t numberOfHeads[2];
	byte_t numberOfHiddenSectors[4];
	byte_t numberOfSectors[4];
	byte_t fatSectorSize[4];
	byte_t flags[2];
	byte_t driveVersion[2];
	byte_t rootDirectoryCluster[4] = {0,0,0,0};
	byte_t fsinfoSector[2];
	byte_t backupVBRSector[2];
	byte_t reserved[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	byte_t logicalDriveNumber;
	byte_t unused2 = 1;
	byte_t extendedSignature;
	byte_t serialNumber[4];
	byte_t volumeName[11] = { 'R', 'y', 'b', 'i', 'c', 'k','i','T','o','m','a' };
	byte_t fatName[8] = { 's', 'z', 0, 'F', 'A', 'T', '3', '2' };
	byte_t code[420] = {0};
	byte_t VBRsignature[2];
} BPB;

typedef struct {
	byte_t signature1[4] = {'R', 'R', 'a', 'A'};
	byte_t signature2[4] = {'r', 'r','A','a'};
	byte_t freeClusters[4] = {0xFF, 0xFF, 0xFF, 0xFF};
	byte_t recentCluster[4] = {0xFF, 0xFF, 0xFF, 0xFF};
	byte_t signature3[4] = {0x00, 0x00, 0x55, 0xAA};
} FSInfo;

/* A class representing a part of file, not a filesystem cluster */
class Cluster{
public:
	Cluster(uint32_t clusterLength){
		m_data = new byte_t[clusterLength];
		m_clusterLength = clusterLength;
	}

	Cluster(const Cluster &src){
		(*this) = src;
	}

	Cluster& operator=( const Cluster& src ) {
		m_data = new byte_t[src.m_clusterLength];
		m_clusterLength = src.m_clusterLength;
		strncpy((char*)m_data, (char*)src.m_data, m_clusterLength);

		return *this;
	}

	~Cluster(){
		delete[] m_data;
	}

	byte_t* m_data;
	uint32_t m_clusterLength;
};

typedef struct {
	byte_t name[11];
	byte_t extension[3];
	uint32_t length;
	std::list<Cluster> data;
} File;



#endif /* DEFINESFAT_H_ */
