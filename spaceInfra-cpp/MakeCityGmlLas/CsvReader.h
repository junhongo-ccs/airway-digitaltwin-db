#pragma once

//! @file CsvReader.h

#include <string>
#include <fstream>

//! @brief CSV読み込み用
//! ※現在利用されていない
class CsvReader
{
	std::ifstream stream;

	int lineBufferSize = 512;
	std::unique_ptr<char[]> lineBuffer = nullptr;
public:
	CsvReader(const std::string& inputFileName) : CsvReader(inputFileName, 512) {}

	CsvReader(const std::string& inputFileName, int bufferSize) : lineBufferSize(bufferSize) {
		stream.open(inputFileName);
		//stream.imbue(std::locale("Japanese", LC_ALL));

		lineBuffer = std::make_unique<char[]>(lineBufferSize);
	}

	virtual ~CsvReader() {
		close();

	}



	void close() {
		if (stream.is_open()) {

			stream.close();
		}
	}

	template <typename... Args> int readRow(const char* format, Args... args) {
		auto& ifs = stream.getline(lineBuffer.get(), lineBufferSize);
		if (!ifs) {
			return -1;
		}

		return sscanf_s(lineBuffer.get(), format, args ...);

	}

	bool skipHeader() {
		auto& ifs = stream.getline(lineBuffer.get(), lineBufferSize);
		return (ifs ? true : false);
	}

};

