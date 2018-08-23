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
#include "HChainP2PManager/crypto/sha.h"
#include "HChainP2PManager/headers/commonstruct.h"
#include "HChainP2PManager/headers/merkle.h"


CMerkleTree::CMerkleTreeArrayList::CMerkleTreeArrayList()
{
	m_merkleTreeArrayList=NULL;
}

CMerkleTree::CMerkleTreeArrayList::CMerkleTreeArrayList(mt_al*mtal)
{
	m_merkleTreeArrayList=mtal;
}

CMerkleTree::CMerkleTreeArrayList::~CMerkleTreeArrayList()
{
	if(m_merkleTreeArrayList) remove();
}

CMerkleTree::mt_al * CMerkleTree::CMerkleTreeArrayList::create(void)
{
    return (mt_al*)calloc(1, sizeof(mt_al));
}

void CMerkleTree::CMerkleTreeArrayList::remove()
{
	free(m_merkleTreeArrayList->data);
    free(m_merkleTreeArrayList);	
	m_merkleTreeArrayList=NULL;	
}

int CMerkleTree::CMerkleTreeArrayList::isPowerOfTwo(uint32_t v)
{
	return (v != 0) && ((v & (v -1)) == 0);
}

mt_error CMerkleTree::CMerkleTreeArrayList::add(const T_SHA256&h)
{
	assert(m_merkleTreeArrayList->elems < MAX_ELEMS);
	if (!(m_merkleTreeArrayList && h.pID)) 
		return ILLEGAL_PARAM;	

	if (m_merkleTreeArrayList->elems == 0)
	{   
		m_merkleTreeArrayList->data = (T_SHA256*)malloc(HASH_LENGTH*DEF_SHA256_LEN);
		if (!m_merkleTreeArrayList->data) 
			return OUT_Of_MEMORY;		
	}
	else if (isPowerOfTwo(m_merkleTreeArrayList->elems))
	{
		if (((m_merkleTreeArrayList->elems << 1) < m_merkleTreeArrayList->elems)
			|| (m_merkleTreeArrayList->elems << 1 > MAX_ELEMS)) 
				return ILLEGAL_STATE;		
		size_t alloc = m_merkleTreeArrayList->elems * 2 * HASH_LENGTH*DEF_SHA256_LEN;
		T_SHA256 *tmp = (T_SHA256*)realloc(m_merkleTreeArrayList->data, alloc);
		if (!tmp) 
			return OUT_Of_MEMORY;		
		m_merkleTreeArrayList->data = tmp;
	}
	memcpy(&m_merkleTreeArrayList->data[m_merkleTreeArrayList->elems * HASH_LENGTH], 
		&h, sizeof(T_SHA256));
	m_merkleTreeArrayList->elems += 1;

	return SUCCESS;
}

mt_error CMerkleTree::CMerkleTreeArrayList::update(const T_SHA256&h,uint32 offset)
{
	assert(m_merkleTreeArrayList->elems < MAX_ELEMS);
	if (!(m_merkleTreeArrayList && h.pID && offset < m_merkleTreeArrayList->elems)) 
		return ILLEGAL_PARAM;	
	memcpy(&m_merkleTreeArrayList->data[offset * HASH_LENGTH], h.pID, HASH_LENGTH);

	return SUCCESS;
}

mt_error CMerkleTree::CMerkleTreeArrayList::addOrUpdate(const T_SHA256&h,uint32 offset)
{
	assert(m_merkleTreeArrayList->elems < MAX_ELEMS);
	if (!(m_merkleTreeArrayList && h.pID) || offset > m_merkleTreeArrayList->elems) 
		return ILLEGAL_PARAM;	
	if (offset == m_merkleTreeArrayList->elems) 
		return add(h);
	else 
		return update(h, offset);	
}


const T_SHA256 * CMerkleTree::CMerkleTreeArrayList::getOffset(uint32 offset)
{
	assert(m_merkleTreeArrayList->elems < MAX_ELEMS);
	if (!(m_merkleTreeArrayList && offset < m_merkleTreeArrayList->elems)) 
		return NULL;
	
	return &m_merkleTreeArrayList->data[offset * HASH_LENGTH];
}


CMerkleTree::mt_t* CMerkleTree::create(void)
{  
	m_merkleTree = (mt_t*)calloc(1, sizeof(mt_t));
	if (!m_merkleTree) return NULL;
	
	for (size_t i = 0; i < TREE_LEVELS; ++i) 
	{
		mt_al *tmp = m_merkleTree->levels[i]->create();
		if (!tmp) 
		{
			for (size_t j = 0; j < i; ++j) 
				m_merkleTree->levels[j]->remove();			
			free(m_merkleTree);
			return NULL;
		}
		m_merkleTree->levels[i] =new CMerkleTreeArrayList(tmp);
	}

	return m_merkleTree;
}

void CMerkleTree::remove()
{
	if (!m_merkleTree) return;
	for (size_t i = 0; i < TREE_LEVELS; ++i) 
		m_merkleTree->levels[i]->remove();
	for (size_t i = 0; i < TREE_LEVELS; i++)
		if(m_merkleTree->levels[i]) delete m_merkleTree->levels[i];
	
	free(m_merkleTree);
	m_merkleTree=NULL;
}

mt_error CMerkleTree::hash(const T_SHA256 left, const T_SHA256 right,
    T_SHA256& messageDigest) 
{
	if (!(left.pID && right.pID && messageDigest.pID)) 
		return ILLEGAL_PARAM;		
	SHA256Context ctx;
	if (SHA256Reset(&ctx) != shaSuccess) 
		return ILLEGAL_STATE;		
	if (SHA256Input(&ctx, left.pID, DEF_SHA256_LEN) != shaSuccess) 
		return ILLEGAL_STATE;		
	if (SHA256Input(&ctx, right.pID, DEF_SHA256_LEN) != shaSuccess) 
		return ILLEGAL_STATE;		
	if (SHA256Result(&ctx, messageDigest.pID) != shaSuccess) 
		return ILLEGAL_STATE;	

	SHA256Reset(&ctx);
	if (SHA256Input(&ctx, messageDigest.pID, DEF_SHA256_LEN) != shaSuccess)
		return ILLEGAL_STATE;
	if (SHA256Input(&ctx, messageDigest.pID, DEF_SHA256_LEN) != shaSuccess)
		return ILLEGAL_STATE;
	if (SHA256Result(&ctx, messageDigest.pID) != shaSuccess)
		return ILLEGAL_STATE;

	return SUCCESS;
}

mt_error CMerkleTree::add(const T_SHA256*v,size_t count)
{
	if (!(m_merkleTree && v && count<MAX_ELEMS))
		return ILLEGAL_PARAM;
	
	for(size_t i=0; i<count; i++)
		add(v[i]);	

	return SUCCESS;
}


mt_error CMerkleTree::add(const T_SHA256&v)
{	
	ERROR_CHECK(m_merkleTree->levels[0]->add(v));
	m_merkleTree->elems += 1;	
	if (m_merkleTree->elems == 1) 
		return SUCCESS;	
	uint32 order = m_merkleTree->elems - 1;
	uint32 level = 0; 
	T_SHA256 messageDigest(v);
	while (order > 0 && level < TREE_LEVELS)
	{		
		if (getRight(order)) 
		{
			const T_SHA256 * left = m_merkleTree->levels[level]->getOffset(order - 1);			
			ERROR_CHECK(hash(*left, messageDigest, messageDigest));
			ERROR_CHECK(m_merkleTree->levels[level + 1]->addOrUpdate( messageDigest, (order >> 1)));
		}
		order >>= 1;
		level += 1;		
	}
	assert(!memcmp(messageDigest.pID, m_merkleTree->levels[level]->getOffset(order)->pID, HASH_LENGTH));	

	return SUCCESS;
}


mt_error CMerkleTree::verify(const T_SHA256&v,uint32 offset)
{
	if ( !(m_merkleTree && (offset < m_merkleTree->elems)) ) 
		return ILLEGAL_PARAM;

	T_SHA256 messageDigest(v);
	uint32 order = offset;
	uint32 level = 0;	
	while (hasNextLevel(level))
	{
		if (!getRight(order)) 
		{ 			
			const T_SHA256 *right=findRight(order + 1, level);
			if (right != NULL) 
				ERROR_CHECK(hash(messageDigest, *right, messageDigest));			
		} 
		else 
		{      		
			const T_SHA256 * left = m_merkleTree->levels[level]->getOffset(order - 1);
			ERROR_CHECK(hash(*left, messageDigest, messageDigest));
		}
		order >>= 1;
		level += 1;
	}
	int r = memcmp(messageDigest.pID, m_merkleTree->levels[level]->getOffset(order)->pID, HASH_LENGTH);

	if (r) 
		return ROOT_MISMATCH;
	 else 
		return SUCCESS;	
}

uint32 CMerkleTree::getSize()
{
	if (!m_merkleTree) return ILLEGAL_PARAM;
	
	return m_merkleTree->levels[0]->getSize();
}


int CMerkleTree::exists(uint32 offset)
{
	if (!m_merkleTree || offset > MAX_ELEMS) 
		return ILLEGAL_PARAM;
	
	return (m_merkleTree->levels[0]->getOffset(offset) != NULL);
}

uint32 CMerkleTree::hasNextLevel(uint32 curLevel)
{
	if (!m_merkleTree) return 0;

	return (curLevel + 1 < TREE_LEVELS - 1)
		& (m_merkleTree->levels[(curLevel + 1)]->getSize() > 0);
}

const T_SHA256* CMerkleTree::findRight(uint32 offset,int32 level)
{
	if (!m_merkleTree) return NULL;

	do 
	{
		if (offset < m_merkleTree->levels[level]->getSize()) 
			return m_merkleTree->levels[level]->getOffset(offset);			
		level -= 1;
		offset <<= 1;
	} while (level > -1);

	return NULL;
}

int CMerkleTree::getRight(uint32 offset)
{  
	return offset & 0x01;
}

int CMerkleTree::getLeft(uint32 offset)
{  
	return !(offset & 0x01);
}	


mt_error CMerkleTree::getRoot(T_SHA256&root)
{
	if (!(m_merkleTree && root.pID)) 
		return ILLEGAL_PARAM;

	uint32 level = 0;         
	while (hasNextLevel(level)) 
		level += 1;	
	memcpy(&root.pID[0], &m_merkleTree->levels[level]->getOffset(0)->pID[0],
		sizeof(unsigned char)*DEF_SHA256_LEN);

	return SUCCESS;
}


