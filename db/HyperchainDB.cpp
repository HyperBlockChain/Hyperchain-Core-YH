/*Copyright 2016-2018 hyperchain.net (Hyperchain)

Distributed under the MIT software license, see the accompanying
file COPYING or https://opensource.org/licenses/MIT.

Permission is hereby granted, free of charge, to any person obtaining a copy of this 
software and associated documentation files (the "Software"), to deal in the Software
without restriction, including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "HyperchainDB.h"

const T_HYPERBLOCKDBINFO& HyperBlockDB::GetHyperBlock()const
{
	return hyperBlock;
}

LocalChainDB& HyperBlockDB::GetMapLocalChain()
{
	return mapLocalChain;
}



int CHyperchainDB::saveHyperBlockToDB(const T_HYPERBLOCKDBINFO &hyperblock)
{
	return DBmgr::instance()->insertHyperblock(hyperblock);
}

int CHyperchainDB::saveHyperBlocksToDB(const vector<T_HYPERBLOCKDBINFO> &vHyperblock)
{
	for (auto it = vHyperblock.begin(); it != vHyperblock.end(); it ++) {
        T_HYPERBLOCKDBINFO d = *it;
		CHyperchainDB::saveHyperBlockToDB(d);
    }
	return 0;
}

int CHyperchainDB::AddHyperBlockDataRecord(HyperchainDB &hyperchainDB, T_HYPERBLOCKDBINFO blockInfo)
{
	uint64_t uiHyperID = blockInfo.uiReferHyperBlockId;
	uint8 uiType = blockInfo.ucBlockType;

	HyperchainDB::iterator it = hyperchainDB.find(uiHyperID);
	if(it == hyperchainDB.end()) {

		HyperBlockDB hyperBlock;

		if (uiType == HYPER_BLOCK) { 
			hyperBlock.hyperBlock = blockInfo;
		}
		else if (uiType == LOCAL_BLOCK)
		{
			qDebug(blockInfo.strPayload.c_str());
			LocalChainDB mapLocalChain;
			LocalBlockDB mapLocalBlock;
			mapLocalBlock.insert(LocalBlockDB::value_type(blockInfo.uiBlockId, blockInfo));
			hyperBlock.mapLocalChain.insert(LocalChainDB::value_type(blockInfo.uiLocalChainId, mapLocalBlock));
		}
		else 
			return 1;


		hyperchainDB.insert(HyperchainDB::value_type(uiHyperID, hyperBlock));
	}
	else {
		HyperBlockDB hyperBlock = it->second;
		if (uiType == HYPER_BLOCK) {
			hyperBlock.hyperBlock = blockInfo;
		}
		else if (uiType == LOCAL_BLOCK)
		{
			qDebug(blockInfo.strPayload.c_str());
			LocalChainDB::iterator itLocalChain = hyperBlock.mapLocalChain.find(blockInfo.uiLocalChainId);
			if (itLocalChain == hyperBlock.mapLocalChain.end()) { //没有找到localchain
				LocalChainDB mapLocalChain;
				LocalBlockDB mapLocalBlock;
				mapLocalBlock.insert(LocalBlockDB::value_type(blockInfo.uiBlockId, blockInfo));
				hyperBlock.mapLocalChain.insert(LocalChainDB::value_type(blockInfo.uiLocalChainId, mapLocalBlock));
			}
			else {
				LocalBlockDB mapLocalBlock = itLocalChain->second;
				LocalBlockDB::iterator itLocalBlock = mapLocalBlock.find(blockInfo.uiBlockId);
				if (itLocalBlock == mapLocalBlock.end()) { //没有找到对应localblodk id 的 localBlock
					mapLocalBlock.insert(LocalBlockDB::value_type(blockInfo.uiBlockId, blockInfo));
					hyperBlock.mapLocalChain[blockInfo.uiLocalChainId] = mapLocalBlock;
				}
			}
		}
		else
			return 1;

		hyperchainDB[uiHyperID] = hyperBlock;
	} 

	return 0;
}

int CHyperchainDB::cleanTmp(HyperchainDB &hyperchainDB)
{
	if (hyperchainDB.size() > 0)
	{
		HyperchainDB::iterator it = hyperchainDB.begin();
		  for (;it != hyperchainDB.end();++it)  
		  {
				HyperBlockDB hyperBlock = it->second;
				LocalChainDB mapLocalChain = hyperBlock.mapLocalChain;
				LocalChainDB::iterator itLocalChain = mapLocalChain.begin();
				for (; itLocalChain!= mapLocalChain.end(); ++itLocalChain)
				{
					LocalBlockDB localBlock = itLocalChain->second;
					localBlock.erase(localBlock.begin(), localBlock.end());
				}
				mapLocalChain.erase(mapLocalChain.begin(), mapLocalChain.end());
		  }

		hyperchainDB.erase(hyperchainDB.begin(), hyperchainDB.end());
	}
	return 0;
}

int CHyperchainDB::getHyperBlocks(HyperchainDB &hyperchainDB, uint64 nStartHyperID, uint64 nEndHyperID)
{
		QList<T_HYPERBLOCKDBINFO> queue;
		int nRet = DBmgr::instance()->getHyperblocks(queue, nStartHyperID, nEndHyperID);
		if (nRet == 0)
		{ 
			int i = 0;
			for (; i != queue.size(); ++i) {
				T_HYPERBLOCKDBINFO info = queue.at(i);
				qDebug((const char*)info.strHashSelf);
				CHyperchainDB::AddHyperBlockDataRecord(hyperchainDB, info);
			}
			return i;
		}

	return 0;

}

int CHyperchainDB::getAllHyperBlocks(HyperchainDB &hyperchainDB)
{
	QList<T_HYPERBLOCKDBINFO> queue;
	int nRet = DBmgr::instance()->getHyperblock(queue, 1, -1);
	if (nRet == 0)
	{ 
		int i = 0;
		for (; i != queue.size(); ++i) {
			T_HYPERBLOCKDBINFO info = queue.at(i);
			qDebug((const char*)info.strHashSelf);
			AddHyperBlockDataRecord(hyperchainDB, info);
		}
		return i;
	}
	 
	return 0;
}


uint64 CHyperchainDB::GetLatestHyperBlockNo()
{
	return DBmgr::instance()->getLatestHyperBlockNo();
}

int CHyperchainDB::GetHyperBlockNumInfo(std::list<uint64> &HyperBlockNum)
{
	int nRet = DBmgr::instance()->getAllHyperblockNumInfo(HyperBlockNum);
	
	return 0;
}

int CHyperchainDB::GetLatestHyperBlock(HyperBlockDB &hyperblockDB)
{
	uint64 nHyperId = DBmgr::instance()->getLatestHyperBlockNo();

	if (nHyperId) {
		QList<T_HYPERBLOCKDBINFO> queue;
		int nRet = DBmgr::instance()->getHyperblocks(queue, nHyperId, nHyperId);
		if (nRet == 0) {
			int i = 0;
			for (; i != queue.size(); ++i) {
				T_HYPERBLOCKDBINFO blockInfo = queue.at(i);

				uint8 uiType = blockInfo.ucBlockType;

				if (uiType == HYPER_BLOCK) {
					hyperblockDB.hyperBlock = blockInfo;
				}
				else if (uiType == LOCAL_BLOCK) {

					LocalChainDB::iterator itLocalChain = hyperblockDB.mapLocalChain.find(blockInfo.uiLocalChainId);
					if (itLocalChain == hyperblockDB.mapLocalChain.end()) { //没有找到localchain
						LocalChainDB mapLocalChain;
						LocalBlockDB mapLocalBlock;
						mapLocalBlock.insert(LocalBlockDB::value_type(blockInfo.uiBlockId, blockInfo));
						hyperblockDB.mapLocalChain.insert(LocalChainDB::value_type(blockInfo.uiLocalChainId, mapLocalBlock));
					}
					else {
						LocalBlockDB mapLocalBlock = itLocalChain->second;
						LocalBlockDB::iterator itLocalBlock = mapLocalBlock.find(blockInfo.uiBlockId);
						if (itLocalBlock == mapLocalBlock.end()) { //没有找到对应localblodk id 的 localBlock
							mapLocalBlock.insert(LocalBlockDB::value_type(blockInfo.uiBlockId, blockInfo));
							hyperblockDB.mapLocalChain[blockInfo.uiLocalChainId] = mapLocalBlock;
						}
					}
				}
			}
		}
	}
	return 0;
}


