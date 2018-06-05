/*copyright 2016-2018 hyperchain.net (Hyperchain)
/*
/*Distributed under the MIT software license, see the accompanying
/*file COPYING or https://opensource.org/licenses/MIT。
/*
/*Permission is hereby granted, free of charge, to any person obtaining a copy of this 
/*software and associated documentation files (the "Software"), to deal in the Software
/*without restriction, including without limitation the rights to use, copy, modify, merge,
/*publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
/*to whom the Software is furnished to do so, subject to the following conditions:
/*
/*The above copyright notice and this permission notice shall be included in all copies or
/*substantial portions of the Software.
/*
/*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
/*INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
/*PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
/*FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
/*OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/*DEALINGS IN THE SOFTWARE.
*/
#pragma once

#include "HChainP2PManager/headers/inter_public.h"

#include <thread>
#include <mutex>

#include <QList>
#include <map>

class CppSQLite3DB;

class DBmgr
{
public:
    static DBmgr* instance();

    virtual ~DBmgr();

    int open(const char * dbpath);
    bool isOpen();
    int close();

public:
    int insertEvidence(const TEVIDENCEINFO& evidence);
    int getEvidences(QList<TEVIDENCEINFO>& evidences, int page, int size = 20);
    int updateEvidence(const TEVIDENCEINFO& evidence, int type = 1);
	int getNoConfiringList(QList<TEVIDENCEINFO>& evidences);
    int delEvidence(std::string hash);
	int delEvidence(const TEVIDENCEINFO &evidence);
	int insertHyperblock(const T_HYPERBLOCKDBINFO &hyperblock);
	int updateHyperblock(const T_HYPERBLOCKDBINFO &hyperblock);
	int existHyperblock(const T_HYPERBLOCKDBINFO &hyperblock);
	int getHyperblock(QList<T_HYPERBLOCKDBINFO> &list, int page, int size);
	int getHyperblockshead(QList<T_HYPERBLOCKDBINFO> &queue, int nStartHyperID);
	int getAllHyperblockNumInfo(std::list<uint64> &queue);
	int getHyperblocks(QList<T_HYPERBLOCKDBINFO> &queue, int nStartHyperID, int nEndHyperID);
    int getUpqueue(QList<TUPQUEUE> &queue, int page, int size);
	int delUpqueue(std::string hash);
	int addUpqueue(string sHash);

	int getLatestHyperBlockNo();
	int getLatestHyperBlock(T_HYPERBLOCKDBINFO &hyperblock);

	string hash256tostring(const unsigned char* hash);
	void strtohash256(unsigned char* hash, const char* szHash);

private:
    DBmgr();

private:
    int createTbls();

private:
    int updateDB();

private:
    bool ifColExist(const char* tbl, const char* col);

    bool ifTblOrIndexExist(const char* name, int type = 1);

private:
    CppSQLite3DB *_db;
    std::mutex _mutex;
};
