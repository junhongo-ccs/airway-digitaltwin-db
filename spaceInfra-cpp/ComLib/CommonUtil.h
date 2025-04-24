#pragma once

//! @file CommonUtil.h
//! @brief 共通関数群

#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <regex>

namespace ComLib {
	class CommonUtil
	{
	public:
		static const char PATH_SEP =
#ifdef _WIN32
			'\\';
#else
			'/';
#endif


		//! @brief ファイルパスを連結する
		//! 
		//! @param first パスの前の部分
		//! @param second パスの後ろの部分
		//! @return 連結した結果
		static std::string pathJoin(const std::string_view& first, const std::string_view& second) {
			std::string newFirst = std::string(first);
			std::string newSecond = std::string(second);
#ifdef _WIN32
			//変換しなくてもエラーにならないようだけど、見にくい
			std::replace(newFirst.begin(), newFirst.end(), '/', PATH_SEP);
			std::replace(newSecond.begin(), newSecond.end(), '/', PATH_SEP);
#else
			newFirst.replace(newFirst.begin(), newFirst.end(), '\\', PATH_SEP);
			newSecond.replace(newSecond.begin(), newSecond.end(), '\\', PATH_SEP);
#endif

			std::filesystem::path result(newFirst);
			result.append(newSecond);

			return result.string();
		}

		//! @brief 指定文字列で始まっているかをチェック
		//! 
		//! @param target チェック対象文字列
		//! @param prefix プレフィックス
		static bool startsWith(const std::string_view& target, const std::string& prefix) {
			if (target.size() < prefix.size()) {
				return false;
			}

			return std::equal(std::begin(prefix), std::end(prefix), std::begin(target));
		}

		static bool startsWithIgnoreCase(const std::string_view& target, const std::string& prefix) {
			if (target.size() < prefix.size()) {
				return false;
			}

			std::string tgt(target);
			std::string pre(prefix);

			std::transform(tgt.begin(), tgt.end(), tgt.begin(), [](char const& c) {
				return std::toupper(c);
				});
			std::transform(pre.begin(), pre.end(), pre.begin(), [](char const& c) {
				return std::toupper(c);
				});


			return std::equal(std::begin(pre), std::end(pre), std::begin(tgt));
		}


		static bool endsWith(const std::string& s, const std::string& suffix) {
			if (s.size() < suffix.size()) return false;
			return std::equal(std::rbegin(suffix), std::rend(suffix), std::rbegin(s));
		}

		//! 処理対象のLASファイルリストを取得する
		//! 
		//! @param folder チェック対象のフォルダ
		//! @param ptn ファイルパターン
		//! @param targetFile 絞りたいファイル名 ※指定された場合高々1ファイル
		//! @return 取得リスト
		static std::vector<std::string> getFileList(const std::string& folder, const std::string& ptn,
			const std::string& targetFile) {
			std::regex re(ptn);

			std::vector<std::string> resultList;
			for (const auto& file : std::filesystem::directory_iterator(folder)) {
				if (file.is_regular_file()) {
					std::string fileName = file.path().filename().string();

					if (!targetFile.empty() && targetFile != fileName) {
						continue;
					}

					bool matched = (ptn.empty() || std::regex_match(fileName, re));
					if (matched) {
						resultList.push_back(CommonUtil::pathJoin(folder, fileName));
					}
				}
			}

			return resultList;

		}

		static std::vector<std::string> getFolderList(const std::string& folder) {

			std::vector<std::string> resultList;
			for (const auto& file : std::filesystem::directory_iterator(folder)) {
				if (file.is_directory()) {
					std::string fileName = file.path().filename().string();

					resultList.push_back(fileName);
				}
			}

			return resultList;

		}


		//! @brief フォルダーの自動作成
		//! 
		//! @param dir 対象フォルダー
		//! @return trueなら成功
		static bool forceDirectories(const std::string& dir) {

			return std::filesystem::create_directories(dir);

		}

		//! @brief アプリIDを取得する
		//! 
		//! @param fullPath 実行中のモジュール名
		//! @return 取得結果
		static std::string getAppName(const std::string& fullPath) {
			std::filesystem::path  file(fullPath);
			std::string fileName = file.filename().string();

			return fileName.substr(0, fileName.find_last_of('.'));
		}

		//! @brief 文字列の整形
		//! 
		//! @param fmt 形式文字列
		//! @param args 可変長パラメータ
		template <typename ... Args>
		static std::string format(const std::string& fmt, Args ... args)
		{
			char buf[512]; //512-1までOK
			sprintf_s(buf, sizeof(buf) - 1, fmt.c_str(), args...);

			return buf;
		}

		static std::string join(const std::vector<std::string>& v, const char* delim = nullptr) {
			std::string s;
			if (!v.empty()) {
				s += v[0];
				for (decltype(v.size()) i = 1, c = v.size(); i < c; ++i) {
					if (delim != nullptr) s += delim;
					s += v[i];
				}
			}
			return s;
		}

		static std::vector<std::string> split(std::string str, char del) {
			int first = 0;
			int last = (int)str.find_first_of(del);

			std::vector<std::string> result;
			if (last == std::string::npos) {
				result.push_back(str);
				return result;
			}

			while (first < str.size()) {
				std::string subStr(str, first, last - first);

				result.push_back(subStr);

				first = last + 1;
				last = static_cast<int>(str.find_first_of(del, first));

				if (last == std::string::npos) {
					last = (int)str.size();
				}
			}

			return result;
		}

		static void toLower(std::string& str) {
			transform(str.begin(), str.end(), str.begin(), ::tolower);
		}


		static void deleteFiles(const std::string& folder, const std::string& ptn) {
			if (ptn.size() == 0 || folder.size() == 0) {
				return;
			}

			if (!std::filesystem::exists(folder)) {
				return;
			}

			//std::cout << "delPattern:" << ptn << std::endl;

			std::regex re(ptn);
			std::vector<std::string> delList;

			for (const auto& file : std::filesystem::directory_iterator(folder)) {
				if (file.is_regular_file()) {
					std::string ext = file.path().extension().string();
					toLower(ext);

					//安全のために、削除対象ファイルを*.las, *.laz, *.csv, *.geojson, *.gzに限定する
					if (ext != ".csv" && ext != ".las" && ext != ".geojson" && ext != ".gz" && ext != ".laz") {
						continue;
					}

					std::string fileName = file.path().filename().string();
					bool matched = (ptn.empty() || std::regex_match(fileName, re));
					if (matched) {
						delList.push_back(pathJoin(folder, fileName));
					}
				}
			}

			//ファイルを削除
			for (std::string& fileName : delList) {
				//std::cout << "delete:" << fileName << std::endl;

				std::filesystem::remove(fileName);
			}

		}

		static void spatialIdToIndex(const char* spatialId, int& zoom, int& altIndex, int& lonIndex, int& latIndex) {
			const int BUF_LIMIT = 80;
			char buf[BUF_LIMIT];
			const char* ptr = spatialId;
			size_t size = 0;
			int targetId = 0;
			int val = 0;
			while (*ptr != '\0') {
				if (*ptr == '/') {
					if (size >= BUF_LIMIT) {
						throw std::runtime_error("wrong format");
					}
					strncpy_s(buf, ptr - size, size);
					buf[size] = '\0';
					val = std::atoi(buf);
					switch (targetId) {
					case 0:
						zoom = val;
						break;
					case 1:
						altIndex = val;
						break;
					case 2:
						lonIndex = val;
						break;
					default:
						throw std::runtime_error("wrong format");
					}

					targetId++;
					size = 0;
				}
				else {
					size++;
				}

				ptr++;
			}

			if (targetId != 3 || size == 0 || size >= BUF_LIMIT) {
				throw std::runtime_error("wrong format");
			}

			strncpy_s(buf, ptr - size, size);
			buf[size] = '\0';
			latIndex = std::atoi(buf);

			//std::cout << "##### zoom:" << zoom << ", altIndex:" << altIndex << ", lonIndex:" << lonIndex << ", latIndex:" << latIndex << std::endl;
		}

		static std::string indexToSpatialId(int z, int f, int x, int y) {
			char buf[80];
			sprintf_s(buf, sizeof(buf), "%d/%d/%d/%d", z, f, x, y);

			std::string spatialId(buf);

			return spatialId;
		}

		static std::string getCurDate() {
			time_t curTime = time(NULL);
			struct tm local;
			localtime_s(&local, &curTime);
			char timeBuf[20];
			strftime(timeBuf, sizeof(timeBuf), "%Y%m%d", &local);
			std::string today = timeBuf;

			return today;
		}

		static std::string getCurDatetime() {
			time_t curTime = time(NULL);
			struct tm local;
			localtime_s(&local, &curTime);
			char timeBuf[20];
			strftime(timeBuf, sizeof(timeBuf), "%Y%m%d%H%M%S", &local);
			std::string today = timeBuf;

			return today;
		}


	};

	static std::string getCurDate() {
		time_t curTime = time(NULL);
		struct tm local;
		localtime_s(&local, &curTime);
		char timeBuf[20];
		strftime(timeBuf, sizeof(timeBuf), "%Y%m%d", &local);
		std::string today = timeBuf;

		return today;
	}

	static std::string getCurDatetime() {
		time_t curTime = time(NULL);
		struct tm local;
		localtime_s(&local, &curTime);
		char timeBuf[20];
		strftime(timeBuf, sizeof(timeBuf), "%Y%m%d%H%M%S", &local);
		std::string today = timeBuf;

		return today;
	}

};