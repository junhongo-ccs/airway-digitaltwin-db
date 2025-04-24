#pragma once

#include <spdlog/spdlog-inl.h>
#include "Config.h"

template <class T> class ObjectRefContainer{
	std::shared_ptr<spdlog::logger> logger;
	static const int DEF_BLOCK_SIZE = 100;
	int numOfObjects = 0;
	T** ppObjects = nullptr;
	int capacity = 0;
	int blockSize = DEF_BLOCK_SIZE;

public:
	ObjectRefContainer() : ObjectRefContainer(DEF_BLOCK_SIZE) {

		//logger->trace("in ObjectRefContainer: type={0}", typeid(T).name());
	}

	ObjectRefContainer(int blockSize) {
		Config& config = Config::getInstance();
		logger = spdlog::get(config.getLoggerName());
		if (blockSize <= 0) {
			blockSize = DEF_BLOCK_SIZE;
		}

		this->blockSize = blockSize;

		//if (blockSize > 10) {
		//	logger->trace("in ObjectRefContainer: type={0}, blockSize:{1}", typeid(T).name(), blockSize);
		//}
	}

	virtual ~ObjectRefContainer() {
		//logger->trace("---- ~ObjectRefContainer　called");
		if (ppObjects != nullptr) {
			std::free(ppObjects);
		}

	}

	void resetAll() {
		if (ppObjects != nullptr) {
			std::free(ppObjects);
			ppObjects = nullptr;
		}
		numOfObjects = 0;
		capacity = 0;
	}

	bool isEmpty() {
		return numOfObjects <= 0;
	}

	void addObjects(int num, T* pObjects) {
		if (numOfObjects + num > capacity) {
			//logger->trace("in addObject: type={0}, numOfObjects:{1}, num:{2}, capacity:{3}", typeid(T).name(), numOfObjects, num, capacity);

			int addSize = blockSize;
			if (num > blockSize) {
				//std::cout << "--------------num:" << num << ",blockSize:" << blockSize << std::endl;
				addSize += (num / blockSize) * blockSize;
			}

			//追加対象が一つしかないけど、性能改善のためにまとまったメモリを確保している
			int requestSize = sizeof(T *) * (capacity + addSize);
			if (ppObjects == nullptr) {
				ppObjects = (T**)std::malloc(requestSize);
			}
			else {
				T** ppTemp = ppObjects;
				ppObjects = (T**)std::realloc(ppTemp, requestSize);
				if (ppObjects == nullptr) {
					std::free(ppTemp);
				}
			}
			
			{//debug
				std::string tmpTypeName = typeid(T).name();
				if (tmpTypeName.find("GeoPolygon") != std::string::npos) {
					logger->debug("(ObjectRefContainer.addObjects), capacity={0},numOfObjects={1},num={2}, type:{3}, requestSize:{4}, addSize:{5}, sizeOf(T):{6}, blockSize:{7}",
						capacity, numOfObjects, num, typeid(T).name(), requestSize, addSize, sizeof(T*), blockSize);
				}
			}
			if (ppObjects == nullptr) {
				logger->error("メモリ確保に失敗しました(ObjectRefContainer.addObjects), capacity={0},numOfObjects={1},num={2}, type:{3}, requestSize:{4}, addSize:{5}, sizeOf(T):{6}, blockSize:{7}", 
					capacity, numOfObjects, num, typeid(T).name(),requestSize, addSize, sizeof(T*), blockSize);
				throw std::bad_alloc();
			}
			capacity += addSize;
		}

		for (int i = 0; i < num; i++) {
			ppObjects[numOfObjects + i] = pObjects + i;
		}
		numOfObjects += num;
	}

	T** getObjectRef() {
		return ppObjects;
	}

	int getObjectCount() {
		return numOfObjects;
	}

	T* getObject(int i) {
		return (i >= 0 && i < numOfObjects ? ppObjects[i] : nullptr);
	}

	T* operator[](int index) {
		return getObject(index);
	}
};