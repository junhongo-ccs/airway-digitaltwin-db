#pragma once

//! @file ObjectPool.h

#include <memory>

//! @brief メモリ管理用クラス
//! 
//! @detail 重要なクラスのひとつである。メモリ確保回数を減らすために、このクラスで必要に応じてまとまったメモリを確保するようになっている。
template <class T> class ObjectPool {
	struct _Block {
		T* objectPool = nullptr;
		_Block* pPrev = nullptr;
		_Block* pNext = nullptr;
		int capacity = 0;
		int used = 0;
	};
	_Block* pHead = nullptr;
	_Block* pCur = nullptr;

	int blockCount = 0;
	int blockSize = 256;
	//std::allocator<T> alloc;
public:
	ObjectPool():ObjectPool(512) {
	
	}
	ObjectPool(int blockSize) {
		if (blockSize <= 0) {
			blockSize = 256;
		}
		this->blockSize = blockSize;
		

		pHead = new _Block;
		pCur = pHead;
		pCur->pPrev = nullptr;
		pCur->pNext = nullptr;

		//最初のブロックを確保
		pCur->objectPool = new T[blockSize];
		pCur->capacity = blockSize;
		pCur->used = 0;
		blockCount = 1;

	};
	virtual ~ObjectPool() {
		pCur = pHead;


		while (pCur != nullptr) {
			_Block* pNext = pCur->pNext;

			delete[] pCur->objectPool;
			delete pCur;

			pCur = pNext;
		}

	}

	void resetAll() {
		pCur = pHead;


		while (pCur != nullptr) {
			_Block* pNext = pCur->pNext;

			delete[] pCur->objectPool;
			delete pCur;

			pCur = pNext;
		}
		blockCount = 0;

		//確保しなおす
		pHead = new _Block;
		pCur = pHead;
		pCur->pPrev = nullptr;
		pCur->pNext = nullptr;

		//最初のブロックを確保
		pCur->objectPool = new T[blockSize];
		pCur->capacity = blockSize;
		pCur->used = 0;
		blockCount = 1;

	}

	//! @brief プールよりメモリブロックを取得する。
	//! 
	//! @detail プールに十分なメモリのない場合、内部でメモリブロックを増やしている
	//! @param 必要なオブジェクトの数（デフォルト：１）
	//! @return 確保できた連続したメモリの先頭アドレス
	T* createObjectFromPool(int nCount = 1);
};

//! @brief プールよりメモリブロックを取得する。
//! 
//! @detail プールに十分なメモリのない場合、内部でメモリブロックを増やしている
//! @param 必要なオブジェクトの数（デフォルト：１）
//! @return 確保できた連続したメモリの先頭アドレス
template <class T> T* ObjectPool<T>::createObjectFromPool(int nCount) {
	if (blockCount >= 50000 && nCount < 100) {
		//無駄なメモリを少なくしようとしている処理。性能に影響する場合は要調整
		//最初から空きを探す。pCurを先にチェックしたほうがもっと効率的だが
		_Block* p = pHead;
		while (p != nullptr) {
			if (p->used + nCount <= p->capacity) {
				T* start = p->objectPool + p->used;
				p->used += nCount;
				return start; //pCurを変更していない
			}
			p = p->pNext;
		}
	}

	//念のためにpCurを最後に移動
	while (pCur->pNext != nullptr) {
		pCur = pCur->pNext;
	}


	if (pCur->used + nCount > pCur->capacity) {
		T* pStart = nullptr;

		_Block* pNew = new _Block();

		//以下の処理を_Blockの中に入れたほうがよさそうだけど、structをシンプルにしたい
		pNew->pPrev = pCur;
		pCur->pNext = pNew;
		pNew->pNext = nullptr;
		pCur = pNew;

		int realBlockSize = (nCount > blockSize ? nCount : blockSize);

		pCur->objectPool = new T[realBlockSize]; // alloc.allocate(realBlockSize);
		pCur->capacity = realBlockSize;
		pCur->used = 0;
		blockCount++;



	}


	T* pStart = pCur->objectPool + pCur->used;
	pCur->used += nCount;

	return pStart;
}
