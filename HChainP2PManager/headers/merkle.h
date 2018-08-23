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
#ifndef __MERKLE_H__

#define __MERKLE_H__

#include "shastruct.h"

typedef enum merkle_tree_error {
	SUCCESS = 0,
	OUT_Of_MEMORY,
	ILLEGAL_PARAM,
	ILLEGAL_STATE,
	ROOT_MISMATCH,
	UNSPECIFIED
} mt_error;

class CMerkleTree
{
#define HASH_LENGTH   32 
#define TREE_LEVELS   20
#define MAX_ELEMS     524288

#define ERROR_CHECK(f) do {mt_error r = f;if (r != SUCCESS) {return r;}} while (0)

private:

	typedef struct merkle_tree_array_list {
		uint32 elems;
		T_SHA256 *data;

		merkle_tree_array_list()
		{
			elems = 0;
			data = NULL;
		}
	} mt_al;

	class CMerkleTreeArrayList
	{
	public:
		CMerkleTreeArrayList();
		CMerkleTreeArrayList(mt_al*mtal);
		~CMerkleTreeArrayList();

		mt_al *create(void);

		void remove();

		mt_error add(const T_SHA256&h);

		mt_error update(const T_SHA256&h, uint32 offset);

		mt_error addOrUpdate(const T_SHA256&h, uint32 offset);

		const T_SHA256* getOffset(uint32 offset);

		inline uint32 getSize()
		{
			if (!m_merkleTreeArrayList) return 0;

			return m_merkleTreeArrayList->elems;
		}

	private:

		int isPowerOfTwo(uint32_t v);

	private:
		mt_al* m_merkleTreeArrayList;

	};

	typedef struct merkle_tree {
		uint32 elems;
		CMerkleTreeArrayList *levels[TREE_LEVELS];

		merkle_tree()
		{
			elems = 0;
			for (size_t i = 0; i<TREE_LEVELS; i++)
				levels[i] = NULL;
		}
	} mt_t;

public:
	mt_t* create(void);

	void remove();

	mt_error add(const T_SHA256*v, size_t count);

	uint32 getSize();

	mt_error verify(const T_SHA256&v, uint32 offset);

	mt_error getRoot(T_SHA256&root);

private:
	mt_error add(const T_SHA256&v);

	int exists(uint32 offset);

	uint32 hasNextLevel(uint32 curLevel);

	const T_SHA256* findRight(uint32 offset, int32 level);

	int getRight(uint32 offset);

	int getLeft(uint32 offset);

	static mt_error hash(const T_SHA256 left, const T_SHA256 right,
		T_SHA256&messageDigest);

private:
	mt_t*  m_merkleTree;

};

#endif