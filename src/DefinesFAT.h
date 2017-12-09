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
 * Offsets from beginning of partion entry
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






typedef struct{
	char head;
	char cylinderHead[2];
} CHS;

typedef unsigned long long LBA;

/* Complete partition table entry */
typedef struct{
	char state;
	char beginHead;
	char beginCylHead[2];
	char type;
	char endHead;
	char endCylHead[2];
	char sectorsBetweenMBR[2];
	char sizeInSectors[4];
} Partition;

typedef struct{
	char bootCode[3];
	char OEMName[8];
	char bytesPerSector[2];
	char sectorsPerCluster;
	char reservedSectors[2];
	char fatCopiesCount;
	char maxRootEntries[2];
	char sectorsSmallPart[2];
	char mediaDescriptor;
	char unused1[2];
	char sectorsPerTrack[2];
	char numberOfHeads[2];
	char numberOfHiddenSectors[4];
	char numberOfSectors[4];
	char fatSectorSize[4];
	char flags[2];
	char driveVersion[2];
	char rootDirectoryCluster[4];
	char fsinfoSector[2];
	char backupVBRSector[2];
	char reserved[12];
	char logicalDriveNumber;
	char unused2;
	char extendedSignature;
	char serialNumber[4];
	char volumeName[11];
	char fatName[8];
	char code[420];
	char VBRsignature[2];
} BPB;





#endif /* DEFINESFAT_H_ */
