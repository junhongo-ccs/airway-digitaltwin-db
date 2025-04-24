#pragma once

//! @file CommonUtil.h
//! @brief 共通ユーティリティ　クラス　ヘッダー

#include <string>
#include <algorithm>
#include <filesystem>
#include <vector>

//! @brief  共通ユーティリティ　クラス
class CommonUtil
{
public:

	//! @brief 指定文字列で始まっているかをチェック
	//! 
	//! @param target チェック対象文字列
	//! @param prefix プレフィックス
	//! @return チェック結果
	static bool startsWith(const std::string_view& target, const std::string& prefix) {
		if (target.size() < prefix.size()) {
			return false;
		}
		return std::equal(std::begin(prefix), std::end(prefix), std::begin(target));
	}

	//! @brief str2がstr1に含まれていないかチェック
	//! 
	//! @param target チェック対象文字列
	//! @param prefix プレフィックス
	//! @return チェック結果
	static bool contains(const std::string str1, const  std::string str2) {
		return (str1.find(str2) == std::string::npos);
	}

	//! @brief 処理対象のLファイルリストを取得する
	//! 
	//! @param path チェック対象のフォルダ
	//! @param ptn 正規表現 
	//! @return 取得リスト
	static std::vector<std::string> getFileList(const std::string path, const std::string ptn = "");


	//! @brief ディレクトリを生成する
	//! 
	//! @param dir ディレクトリパス
	static bool forceDirectories(const std::string& dir);

	//! @brief ファイルパスからディレクトリを生成する
	//! 
	//! @param dir ファイルパス
	//! @return ディレクトリパス
	static std::string path2dir(const std::string& path) {
		const std::string::size_type pos = std::max<signed>(path.find_last_of('/'), path.find_last_of('\\'));
		return (pos == std::string::npos) ? std::string()
			: path.substr(0, pos + 1);
	}

	static std::vector<std::string> split(const std::string& str, char del);

	static std::string getParam(std::string str, std::string sw);

	static std::string changeDrive(std::string path, std::string drive);

	static int mixColor(int inR, int inG, int inB, int mixR, int mixG, int mixB, double ratio = 0.2);
	static void mixColorEx(int& inOutR, int& inOutG, int& inOutB, int mixR, int mixG, int mixB, double ratio = 0.2);

};