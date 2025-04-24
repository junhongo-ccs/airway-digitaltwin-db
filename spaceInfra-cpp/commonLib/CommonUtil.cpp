//! @file CommonUtil.cpp
//! @brief 共通ユーティリティ　クラス
//! 
//! 
#include <regex>
#include <iostream>
#include <filesystem>
#include "CommonUtil.h"

//! @brief 指定フォルダのファイルリストを取得する
//! 
//! @param folder 入力フォルダ
//! @param ptn ファイル検索パターン
//! @return ファイル一覧
std::vector<std::string> CommonUtil::getFileList(const std::string folder, const std::string ptn) {
	std::regex re(ptn);

	std::vector<std::string> resultList;
	for (const auto& file : std::filesystem::directory_iterator(folder)) {
		if (file.is_regular_file()) {
			bool matched = (ptn.empty() || std::regex_match(file.path().filename().string(), re));
			if (matched) {
				resultList.push_back(file.path().filename().string());
			}
		}
	}

	return resultList;

}

//! @brief 階層的にディレクトリを作成する
//! 
//! @param dir ディレクトリ
bool CommonUtil::forceDirectories(const std::string& dir) {

	return std::filesystem::create_directories(dir);

}

//! @brief 文字列を特定文字で分割する
//! 
//! @param str 文字列
//! @param del 区切り文字
//! @return 文字列一覧
std::vector<std::string> CommonUtil::split(const std::string& src, char delim) {
    std::vector<std::string> vec;
    std::string::size_type len = src.length();

    for (std::string::size_type i = 0, n; i < len; i = n + 1) {
        n = src.find_first_of(delim, i);
        if (n == std::string::npos) {
            n = len;
        }
        vec.push_back(src.substr(i, n - i));
    }

    return vec;
}

//! @brief カンマ区切りの文字列からパラメタを抽出する
//! 
//! @param str カンマ区切り文字列　例１）xxxxx,drive=d  　例２）xxxxx,root=d:/xxxxx/yyyy/zzz  
//! @param sw 指定パラメタキー　""　"drive"　"root"  "level"
//! @return 抽出パラメタ
std::string  CommonUtil::getParam(std::string str, std::string sw) {

    std::string originalParam = "";
    std::string driveParam = "";
    std::string rootParam = "";
    std::string levelParam = "1";
    char del = ',';
    std::vector<std::string> strArr =  CommonUtil::split(str, del);
    for (int i = 0; i < strArr.size(); i++) {
        std::string param = strArr[i];
        del = '=';
        std::vector<std::string> strArr2 = CommonUtil::split(param, del);
        if (strArr2.size() == 1) {
            originalParam = strArr2[0];
        }
        else {
            if (strArr2[0] == "drive") {
                driveParam = strArr2[1];
            } else if (strArr2[0] == "root") {
                rootParam = strArr2[1];
            } else if (strArr2[0] == "level") {
                levelParam = strArr2[1];
            }
        }
    }

    if (sw == "") {
        return originalParam;
    }
    else if (sw == "drive") {
        return driveParam;
    }
    else if (sw == "root") {
        return rootParam;
    }
    else if (sw == "level") {
        return levelParam;
    }
    return "";
}

//! @brief パスを指定driveで差し替える
//! 
//! @param path driveから指定されたパス  
//! @param drive 置き換えるdrive
//! @return 抽出パラメタ
std::string CommonUtil::changeDrive(std::string path, std::string drive) {

    std::string newPath = drive + path.substr(1);
    return newPath;

}

//! @brief 色のミックス
//! 
//! @param inR 元カラー・赤(0～255)
//! @param inG 元カラー・緑(0～255)
//! @param inB 元カラー・青(0～255)
//! @param mixR ターゲットカラー・赤(0～255)
//! @param mixG ターゲットカラー・緑(0～255)
//! @param mixG ターゲットカラー・青(0～255)
//! @param ratio 割合（0～1.0)
//! @return ミックスしたあとの色（red*65536+green*256+blue)
int CommonUtil::mixColor(int inR, int inG, int inB, int mixR, int mixG, int mixB, double ratio) {
    int diffR = mixR - inR;
    int diffG = mixG - inG;
    int diffB = mixB - inB;

    int red = inR + static_cast<int>(diffR * ratio);
    int green = inG + static_cast<int>(diffG * ratio);
    int blue = inB + static_cast<int>(diffB * ratio);

    return (red << 16) + (green << 8) + blue;
}

//! @brief 色のミックス
//! ※入力カラーのバイト数は内部で推測
//! ※結果を入力変数に設定して返す
//! 
//! @param inOutR 元カラー・赤(0～255か0～65535)
//! @param inOutG 元カラー・緑(0～255か0～65535)
//! @param inOutB 元カラー・青(0～255か0～65535)
//! @param mixR ターゲットカラー・赤(0～255)
//! @param mixG ターゲットカラー・緑(0～255)
//! @param mixG ターゲットカラー・青(0～255)
//! @param ratio 割合（0～1.0)
//! 
void CommonUtil::mixColorEx(int& inOutR, int& inOutG, int& inOutB, int mixR, int mixG, int mixB, double ratio) {
    int inR = inOutR;
    int inG = inOutG;
    int inB = inOutB;

    const int max1byte = 255;

    //入力が２バイトの場合１バイトに戻す
    //※性能のためにビットシフトにしているだけ。元データを見ても下位バイトは00みたい
    if (inOutR > max1byte) inR = inOutR >> 8;
    if (inOutG > max1byte) inG = inOutG >> 8;
    if (inOutB > max1byte) inB = inOutB >> 8;


    int diffR = mixR - inR;
    int diffG = mixG - inG;
    int diffB = mixB - inB;

    int red = inR + static_cast<int>(diffR * ratio);
    int green = inG + static_cast<int>(diffG * ratio);
    int blue = inB + static_cast<int>(diffB * ratio);

    //出力
    inOutR = red<<8;
    inOutG = green<<8;
    inOutB = blue<<8;
}
