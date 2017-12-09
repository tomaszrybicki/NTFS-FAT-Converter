/*
 * FATManager.h
 *
 *  Created on: Dec 9, 2017
 *  Author: Tomasz Rybicki
 */

#ifndef FATMANAGER_H_
#define FATMANAGER_H_

#include "DefinesFAT.h"
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

class FATManager {
public:
	FATManager(string drive);
	virtual ~FATManager();

	void readPartitionTable();
	void writeBPB();


private:
	void read(streamoff offset, unsigned long long length, char* dest);
	void write(streamoff offset, unsigned long long length, char* src);
	LBA convertCHStoLBA(CHS source);
	CHS convertLBAtoCHS(LBA source);

private:
	Partition m_partitionTable[4];
	BPB m_bpb;
	string m_drive;

};

#endif /* FATMANAGER_H_ */
