/*Copyright 2017 hyperchain.net  (Hyper Block Chain)
/*
/*Distributed under the MIT software license, see the accompanying
/*file COPYING or https://opensource.org/licenses/MIT.
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

#include "db/dbmgr.h"
#include "util/cppsqlite3.h"
#include "p2p/headers/inter_public.h"

namespace DBSQL {
	//存证记录
	const std::string EVIDENCES_TBL =
		"CREATE TABLE IF NOT EXISTS evidence_tbl "
		"("
		"    [hash]                                 TEXT NOT NULL,"
		"    [blocknum]                             INTEGER DEFAULT 0,"
		"    [filename]                             TEXT NOT NULL,"
		"    [custominfo]                           TEXT DEFAULT '',"
		"    [owner]                                TEXT DEFAULT '',"
		"    [filestate]                            INTEGER DEFAULT 0,"
		"    [regtime]                              INTEGER DEFAULT 0,"
		"    [filesize]                             INTEGER DEFAULT '',"
		"    [extra]                                TEXT DEFAULT '',"
		"    PRIMARY KEY(hash, blocknum)"
		");";

	const std::string HYPERBLOCK_TBL =
		"CREATE TABLE IF NOT EXISTS hyperblock ("
		"  [hash]	TEXT NOT NULL DEFAULT '',"
		"  [id]		INTEGER DEFAULT 0,"
		"  [type]	INTEGER DEFAULT 0,"
		"  [hid]		INTEGER DEFAULT 0,"
		"  [hhash]	TEXT NOT NULL DEFAULT '',"
		"  [hash_prev]	TEXT NOT NULL DEFAULT '',"
		"  [payload]	TEXT NOT NULL DEFAULT '',"
		"  [ctime]	INTEGER DEFAULT 0,"
		"  PRIMARY KEY (hash)"
		");";
}




////////////////////////////////////////////////////
static const std::string scEvidenceInsert = "INSERT OR REPLACE INTO evidence_tbl(hash,blocknum,filename,custominfo,owner,filestate,regtime,filesize,extra) "
                                         "VALUES(?,?,?,?,?,?,?,?,?);";
////////////////////////////////////////////////////
static const std::string scHyperblockInsert = "INSERT OR REPLACE INTO hyperblock(hash,id,type,hid,hhash,hash_prev,payload,ctime)  "
"VALUES(?,?,?,?,?,?,?,?);";
////////////////////////////////////////////////////
DBmgr *DBmgr::instance()
{
    static DBmgr s;
    return &s;
}

DBmgr::DBmgr(){

}

DBmgr::~DBmgr()
{
    if (_db){
        if (_db->isOpen()){
            _db->close();
        }
        delete _db;
        _db = nullptr;
    }
}

int DBmgr::open(const char *dbpath)
{
    int ecode = 0;
    try{
        if (_db){
            if (_db->isOpen()){
                _db->close();
            }
        }

        if (!_db){
            _db = new CppSQLite3DB();
        }

        _db->open(dbpath);

#ifndef _DEBUG
        //int result = sqlite3_key(_db->getDB(), "123456!@#$%^", 12);
#endif

        int threadSafe = sqlite3_threadsafe();

        createTbls();

        updateDB();
    }
    catch (CppSQLite3Exception& sqliteException){
        ecode = sqliteException.errorCode();
    }
    catch (...){
        ecode = -1;
    }

    return ecode;
}

bool DBmgr::isOpen()
{
    if (_db && _db->isOpen())
    {
        return true;
    }

    return false;
}

int DBmgr::close()
{
    int ecode = 0;

    try{
        _db->close();
    }
    catch (CppSQLite3Exception& sqliteException){
        ecode = sqliteException.errorCode();
    }
    catch (...){
        ecode = -1;
    }

    return ecode;
}

bool DBmgr::ifColExist(const char *tbl, const char *col)
{
    CppSQLite3Statement stmt = _db->compileStatement("SELECT sql FROM sqlite_master WHERE type='table' AND name = ?");
    stmt.bind(1, tbl);

    std::string sql;
    CppSQLite3Query query = stmt.execQuery();
    while (!query.eof())
    {
        sql = query.getStringField(0);
        break;
    }

    return std::string::npos != sql.find(col);
}

bool DBmgr::ifTblOrIndexExist(const char *name, int type)
{
    int exist = 0;

    try{
        std::string sql;
        if (1 == type){
            sql = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name = ?";
        }
        else{
            sql = "SELECT COUNT(*) FROM sqlite_master WHERE type='index' AND name = ?";
        }

        CppSQLite3Statement stmt = _db->compileStatement(sql.c_str());
        stmt.bind(1, name);

        CppSQLite3Query query = stmt.execQuery();
        if (!query.eof()){
            exist = query.getIntField(0);
        }
    }
    catch (CppSQLite3Exception& ex){
    }

    return exist == 0 ? false : true;
}

int DBmgr::insertEvidence(const TEVIDENCEINFO &evidence)
{
    try
    {
        //hash,blocknum,filename,custominfo,owner,filestate,regtime,filesize,extra
        CppSQLite3Statement stmt = _db->compileStatement(scEvidenceInsert.c_str());
        stmt.bind(1, evidence.cFileHash.c_str());
        stmt.bind(2, 0);
        stmt.bind(3, evidence.cFileName.c_str());
        stmt.bind(4, evidence.cCustomInfo.c_str());
        stmt.bind(5, evidence.cRightOwner.c_str());
        stmt.bind(6, evidence.iFileState);
        stmt.bind(7, (int64)evidence.tRegisTime);
        stmt.bind(8, (int64)evidence.iFileSize);
        stmt.bind(9, "");

        stmt.execDML();
    }
    catch (CppSQLite3Exception& ex)
    {
        return ex.errorCode();
    }

    return 0;
}

int DBmgr::getEvidences(QList<TEVIDENCEINFO> &evidences, int page, int size)
{
    int ret = 0;

    try
    {
        CppSQLite3Statement stmt;
        std::string sql;
        if (page == -1){
            sql = "SELECT * FROM evidence_tbl ORDER BY regtime DESC;";
        }
        else{
            sql = "SELECT * FROM evidence_tbl ORDER BY regtime DESC LIMIT ? OFFSET ?;";
        }

        stmt = _db->compileStatement(sql.c_str());

        if (page != -1){
            stmt.bind(1, size);
            stmt.bind(2, page * size);
        }

        CppSQLite3Query query = stmt.execQuery();
        while (!query.eof())
        {
            //hash,blocknum,filename,custominfo,owner,filestate,regtime,filesize,extra
            TEVIDENCEINFO evi;
            evi.cFileHash = query.getStringField("hash");
            evi.cFileName = query.getStringField("filename");
            evi.cCustomInfo = query.getStringField("custominfo");
            evi.cRightOwner = query.getStringField("owner");
            evi.iFileState = query.getIntField("filestate");
            evi.tRegisTime = query.getInt64Field("regtime");
            evi.iFileSize = query.getInt64Field("filesize");

            evidences.append(evi);

            query.nextRow();
        }
    }
    catch (CppSQLite3Exception& ex)
    {
        ret = ex.errorCode();
    }

    return ret;
}

int DBmgr::updateEvidence(const TEVIDENCEINFO &evidence, int type)
{
    try
    {
        //hash,blocknum,filename,custominfo,owner,filestate,regtime,filesize,extra
        std::string sql;
        if (1 == type){
            sql = "UPDATE evidence_tbl SET filestate=?"
                  "WHERE hash=?;";
        }

        CppSQLite3Statement stmt = _db->compileStatement(sql.c_str());
        if (1 == type){
            stmt.bind(1,  evidence.iFileState);
            stmt.bind(2,  evidence.cFileHash.c_str());
        }

        stmt.execDML();
    }
    catch (CppSQLite3Exception& ex)
    {
        return ex.errorCode();
    }

    return 0;
}

int DBmgr::delEvidence(std::string hash)
{
    try
    {
        CppSQLite3Statement stmt = _db->compileStatement("DELETE FROM evidence_tbl WHERE hash=?;");
        stmt.bind(1, hash.c_str());
        stmt.execDML();
    }
    catch (CppSQLite3Exception& ex)
    {
        return ex.errorCode();
    }

    return 0;
}

int DBmgr::createTbls()
{
    _db->execDML(DBSQL::EVIDENCES_TBL.c_str());
	_db->execDML(DBSQL::HYPERBLOCK_TBL.c_str());
    return 0;
}

int DBmgr::updateDB()
{
    return 0;
}


int DBmgr::insertHyperblock(const T_HYPERBLOCKDBINFO &hyperblock)
{
	try
	{
		//hash,id,type,hid,hhash,hhash_prev,payload,ctime
		CppSQLite3Statement stmt = _db->compileStatement(scHyperblockInsert.c_str());
		stmt.bind(1, hyperblock.strHashSelf.c_str());
		stmt.bind(2, (int64)hyperblock.uiBlockId);
		stmt.bind(3, (int64)hyperblock.ucBlockType);
		stmt.bind(4, (int64)hyperblock.uiReferHyperBlockId);
		stmt.bind(5, hyperblock.strHyperBlockHash.c_str());
		stmt.bind(6, hyperblock.strPreHash.c_str());
		stmt.bind(7, hyperblock.strPayload.c_str());
		stmt.bind(8, (int64)hyperblock.uiBlockTimeStamp);
		stmt.execDML();
	}
	catch (CppSQLite3Exception& ex)
	{
		return ex.errorCode();
	}

	return 0;
}

