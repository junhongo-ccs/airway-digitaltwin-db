#pragma once

//! @file CsvWriter.h

#include <string>
#include <fstream>

//! @brief CSV書き出し用
class CsvWriter {
	std::ofstream stream;
	std::unique_ptr<char[]> buffer = nullptr;
	int bufferSize = 128000;
	int currentPos = 0;

	int lineBufferSize = 256;
	std::unique_ptr<char[]> lineBuffer = nullptr;
public:
	CsvWriter(const std::string& outputFileName) : CsvWriter(outputFileName, 128000, 256) {}

	CsvWriter(const std::string& outputFileName, int bufferSize, int lineBufferSize) :  
			bufferSize(bufferSize), lineBufferSize(lineBufferSize)  {

		buffer = std::make_unique<char[]>(bufferSize);
		lineBuffer = std::make_unique<char[]>(lineBufferSize);

		stream.open(outputFileName);
		//stream.imbue(std::locale("Japanese", LC_ALL));


		currentPos = 0;
	}

	virtual ~CsvWriter() {
		close();

	}



	void close() {
		if (stream.is_open()) {
			if (currentPos > 0) {
				stream.write(buffer.get(), currentPos);
				currentPos = 0;
			}
			stream.close();
		}
	}

	//! @brief CSVに1行書き出す
	//! 
	//! @param format 整形フォーマット
	//! @param args 出力データ（可変長引数)
	template <typename... Args> void writeRow(const char* format, Args... args) {

		sprintf_s(lineBuffer.get(), lineBufferSize, format, args ...);

		int len = std::strlen(lineBuffer.get());

		if (currentPos + len > bufferSize) {
			stream.write(buffer.get(), currentPos);
			currentPos = 0;
		}

		std::memcpy(buffer.get() + currentPos, lineBuffer.get(), len);
		currentPos += len;
	}
	
};

