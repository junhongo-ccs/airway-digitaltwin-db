#pragma once

//! @file Config.h
//! @brief 設定関連
#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <regex>
#include <vector>
#include <fstream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include "CommonUtil.h"
#include <argparse/argparse.hpp>
#include <rapidjson/Document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/error/en.h>


namespace ComLib {
	//! @brief 設定情報を管理するためのクラス
	template <class T> 
	class Config {
	public:
		static T& getInstance()
		{
			//static typename T::Config_pointer_type _config(T::createInstance());

			//return getReference(_config);
			static T _instance;
			return _instance;
		}

	private:
		//typedef std::unique_ptr<T> Config_pointer_type;

		//inline static T* createInstance() { return new T(); }

		//inline static T& getReference(const Config_pointer_type& ptr) { return *ptr; }

	protected:
		Config() {}

	private:
		Config(const Config&) = delete;
		Config& operator=(const Config&) = delete;
		Config(Config&&) = delete;
		Config& operator=(Config&&) = delete;

	//-----------------------------------------------------------------------
	protected:
		//common
		std::string ownerId;
		std::string serverId;
		std::string batchType;
		std::string projectRoot;
		std::string dataRoot;
		std::string pointInRoot;
		std::string pointOutRoot;
		std::string confirmRoot;
		std::string drmRoot;
		std::string mapRoot;
		//std::string citygmlInRoot;
		//std::string citygmlOutRoot;
		std::string logRoot;
		std::string logsink = "console";
		std::string loglevel = "info";
		int colorByte = 2;
		int decimalPoint = 99;

		std::string dbHost;
		std::string dbName;
		std::string dbUser;
		std::string dbPass;

		std::string userDbHost;
		std::string userDbName;
		std::string userDbUser;
		std::string userDbPass;

		//app:common アプリ共通定義
		int mapEpsg;
		int pointInLasEpsg;
		int pointCsvEpsg;
		int pointOutLasEpsg;
		int confirmLasEpsg;
		double lasScale = 0.0001;
		double lasScaleZ = 0.0001;
		int threadCount = 4;
		std::string fromDatetime;
		std::string toDatetime;

		//コマンド引数関連
		int argVerbose = 0;
		std::string batchId = ""; 
		std::string batchFile = ""; //指定されていない場合全て

		//その他
		std::string logLevel = "info";
		std::string configJsonFile;
		std::string loggerName;

	protected:
		virtual void addExtraArguments(argparse::ArgumentParser& argParser) {}
		virtual void parseExtraArguments(argparse::ArgumentParser& argParser) {}
		//virtual bool needBatchId() {
		//	return true;
		//}
		virtual std::vector<std::string> getBatchIdDefs() {
			std::vector<std::string> resultList;
			resultList.push_back("batchid");
			resultList.push_back("バッチID [ファイルID]");
			resultList.push_back("2");
			return resultList;
		}

		virtual std::string getConfigFileName() {
			return "./commonConfig.json";
		}

		virtual void loadExtraConfig(rapidjson::Document& doc) {}

		//! @brief コマンド引数から設定情報をロードする
		//! 
		//! @param options コマンド引数
		//! @param appName アプリID
		void loadArguments(const std::vector<std::string>& options, const std::string& appName) {
			//コマンド引数(>configファイル)
			argparse::ArgumentParser argParser(appName, "1.0", argparse::default_arguments::none);

			std::vector<std::string> batchIdDefs = getBatchIdDefs();
			if (batchIdDefs.size() == 3) {
				argParser.add_argument(batchIdDefs[0]) //ただヘルプを表示させるために。
					.default_value("")
					.required()
					.help(batchIdDefs[1])
					.nargs(1, std::atoi(batchIdDefs[2].c_str()))
					.append();

				argParser.add_argument("--config")
					.help("設定ファイル。デフォルト：commonConfig.json");
			}

			//argParser.add_argument("--featureId")
			//	.default_value("")
			//	.help("対象地物");

			argParser.add_argument("--verbose")
				.help("ログ出力レベル(1:ERROR, 2:WARN, 3:INFO, 4:DEBUG, 5:TRACE)。デフォルト：3")
				.scan<'i', int>();

			addExtraArguments(argParser);

			argParser.add_argument("-h", "--help")
				.action([&](const std::string& s) {
					std::cout << argParser.help().str();
					exit(1);
					})
				.default_value(false)
						.help("shows help message")
						.implicit_value(true)
						.nargs(0)
						.help("ヘルプ");


			//コマンド引数
			if (!options.empty()) {
				argParser.parse_args(options);
			}



			//必要に応じてconfigを更新
			if (argParser.is_used(batchIdDefs[0])) {
				const auto& arg = argParser.get<std::vector<std::string>>(batchIdDefs[0]);

				if (arg.size() > 0) {
					batchId = arg[0];
				}
				if (arg.size() > 1) {
					batchFile = arg[1];
					std::replace(batchFile.begin(), batchFile.end(), '/', '-');
					//if (!batchFile.empty() && batchFile.find_last_of('.') == std::string::npos) {
					//	batchFile += ".las";
					//}
				}
			}

			if (batchId.empty()) {
				std::cout << argParser.help().str() << std::endl;
				exit(1);
			}


			if (argParser.is_used("--config")) {
				configJsonFile = argParser.get("--config");
			}

			//if (argParser.is_used("--epsg")) {
			//    argEpsg = argParser.get("--epsg");
			//}

			//if (argParser.is_used("--epsgLas")) {
			//    argEpsgLas = argParser.get("--epsgLas");
			//}

			//if (argParser.is_used("--epsgCsv")) {
			//    argEpsgCsv = argParser.get("--epsgCsv");
			//}

			if (argParser.is_used("--verbose")) {
				argVerbose = argParser.get<int>("--verbose");
			}

			parseExtraArguments(argParser);

		}

		//! @brief Loggerを準備する
		//! 
		//! @param appName アプリID
		virtual void prepareLogger(const std::string& appName) {
			//----------------------------------------------
			//■ログについてはとりあえず以下のように実装（spdlogger利用）
			//・3パターンのsinkを用意
			//     ①console →　画面に出力
			//     ②basic  →　日付なしのファイルを作成。例（pointmatch.log)
			//     ③daily  →　日付ありのファイルを作成。例（pointmatch-2023-04-06.log)。
			//・フォルダ名は固定。ただsinkがconsoleの場合、ファイル出力を行わない。※リダイレクトでファイルに落とせる
			//・古いログファイルを移動するためのbatファイルは必要なら別途作成。たとえば
			//     、{logroot}\pointmatch\pointmatch-2023-04-06.logを{logroot}\history\pointmatch\の下に移動させるとか。
			//
			// ※アプリを平行に複数個実行させる必要がある場合、PointMatchをシングルスレッドモード(threadCount=1)にしたほうがいい
			std::shared_ptr<spdlog::logger> logger;
			std::string loggerName = appName;
			if (logsink != "console" && logsink != "stdout") {
				std::string dest = logRoot;
				dest = CommonUtil::pathJoin(dest, appName);
				CommonUtil::forceDirectories(dest);

				if (logsink == "basic") {
					dest = CommonUtil::pathJoin(dest, appName).append(".log");
					logger = spdlog::basic_logger_mt(loggerName, dest);
				}
				else { //daily
					dest = CommonUtil::pathJoin(dest, appName).append("-%Y-%m-%d.log");
					logger = spdlog::daily_logger_format_mt(loggerName, dest, 0, 0); //0時0分
				}
			}
			else {
				logger = spdlog::stdout_color_mt(loggerName);
			}

			if (loglevel == "trace") {
				logger->set_level(spdlog::level::trace);
			}
			else if (loglevel == "debug") {
				logger->set_level(spdlog::level::debug);
			}
			else if (loglevel == "info") {
				logger->set_level(spdlog::level::info);
			}
			else if (loglevel == "warn") {
				logger->set_level(spdlog::level::warn);
			}
			else {
				logger->set_level(spdlog::level::err);
			}

		}

		//! @brief 設定を指定ファイルからロードする
		//! 
		//! @param configFileName 設定ファイル
		virtual void loadConfigFile(std::string& configFileName) {
			std::ifstream ifs(configFileName.data());
			rapidjson::IStreamWrapper iws(ifs);

			rapidjson::Document doc;
			rapidjson::ParseResult parseOk = doc.ParseStream(iws);
			if (!parseOk) {
				std::cout << CommonUtil::format("config parse error(%s,%d)",
					rapidjson::GetParseError_En(parseOk.Code()), parseOk.Offset()) << std::endl;
				//throw std::runtime_error("Configファイルには誤りがあります");
				std::exit(9);
			}

			//common
			if (doc.HasMember("common")) {
				auto& commonDict = doc["common"];

				if (commonDict.HasMember("ownerId")) {
					ownerId = commonDict["ownerId"].GetString();
				}
				if (commonDict.HasMember("serverId")) {
					serverId = commonDict["serverId"].GetString();
				}
				if (commonDict.HasMember("batchType")) {
					batchType = commonDict["batchType"].GetString();
				}

				if (commonDict.HasMember("projectRoot")) {
					projectRoot = commonDict["projectRoot"].GetString();
				}
				if (commonDict.HasMember("dataRoot")) {
					dataRoot = commonDict["dataRoot"].GetString();
				}
				if (commonDict.HasMember("pointRoot")) {
					pointInRoot = commonDict["pointRoot"].GetString();
					pointOutRoot = pointInRoot;
				}
				if (commonDict.HasMember("confirmRoot")) {
					confirmRoot = commonDict["confirmRoot"].GetString();
				}
				if (commonDict.HasMember("mapRoot")) {
					mapRoot = commonDict["mapRoot"].GetString();
				}
				if (commonDict.HasMember("drmRoot")) {
					drmRoot = commonDict["drmRoot"].GetString();
				}
				//if (commonDict.HasMember("citygmlRoot")) {
				//	citygmlInRoot = commonDict["citygmlRoot"].GetString();
				//	citygmlOutRoot = citygmlInRoot;
				//}
				if (commonDict.HasMember("logRoot")) {
					logRoot = commonDict["logRoot"].GetString();
				}
				//ログ関連
				if (commonDict.HasMember("logsink")) {
					logsink = commonDict["logsink"].GetString();

					if (commonDict.HasMember("loglevel")) {
						loglevel = commonDict["loglevel"].GetString();
					}
				}

				//colorByte
				if (commonDict.HasMember("colorByte")) {
					colorByte = commonDict["colorByte"].GetInt();
				}
				//decimalPoint
				if (commonDict.HasMember("decimalPoint")) {
					decimalPoint = commonDict["decimalPoint"].GetInt();
				}

				//db
				if (commonDict.HasMember("btDbHost")) {
					dbHost = commonDict["btDbHost"].GetString();
				}
				if (commonDict.HasMember("btDbUser")) {
					dbUser = commonDict["btDbUser"].GetString();
				}
				if (commonDict.HasMember("btDbPass")) {
					dbPass = commonDict["btDbPass"].GetString();
				}
				if (commonDict.HasMember("btDbName")) {
					dbName = commonDict["btDbName"].GetString();
				}

				if (commonDict.HasMember("btUserDbHost")) {
					userDbHost = commonDict["btUserDbHost"].GetString();
				}
				if (commonDict.HasMember("btUserDbUser")) {
					userDbUser = commonDict["btUserDbUser"].GetString();
				}
				if (commonDict.HasMember("btUserDbPass")) {
					userDbPass = commonDict["btUserDbPass"].GetString();
				}
				if (commonDict.HasMember("btUserDbName")) {
					userDbName = commonDict["btUserDbName"].GetString();
				}
			}

			//app
			if (doc.HasMember("app")) {
				auto& appDict = doc["app"];

				//app/common
				if (appDict.HasMember("common")) {
					auto& appCommonDict = appDict["common"];

					if (appCommonDict.HasMember("mapEpsg")) {
						std::string strMapEpsg = appCommonDict["mapEpsg"].GetString();
						mapEpsg = std::atoi(strMapEpsg.c_str());
					}

					if (appCommonDict.HasMember("pointInLasEpsg")) {
						std::string strPointInLasEpsg = appCommonDict["pointInLasEpsg"].GetString();
						pointInLasEpsg = std::atoi(strPointInLasEpsg.c_str());
					}

					if (appCommonDict.HasMember("pointCsvEpsg")) {
						std::string strPointCsvEpsg = appCommonDict["pointCsvEpsg"].GetString();
						pointCsvEpsg = std::stoi(strPointCsvEpsg.c_str());
					}
					else {
						pointCsvEpsg = pointInLasEpsg;
					}

					if (appCommonDict.HasMember("pointOutLasEpsg")) {
						std::string strPointOutLasEpsg = appCommonDict["pointOutLasEpsg"].GetString();
						pointOutLasEpsg = std::atoi(strPointOutLasEpsg.c_str());
					}
					else {
						pointOutLasEpsg = pointCsvEpsg;
					}

					if (appCommonDict.HasMember("confirmLasEpsg")) {
						std::string strConfirmLasEpsg = appCommonDict["confirmLasEpsg"].GetString();
						confirmLasEpsg = std::atoi(strConfirmLasEpsg.c_str());
					}
					else {
						confirmLasEpsg = pointOutLasEpsg;
					}

					if (appCommonDict.HasMember("lasScale")) {
						double tmpScale = appCommonDict["lasScale"].GetDouble();
						if (tmpScale >= 1.0e-12) {
							lasScale = tmpScale;
						}
					}
					if (appCommonDict.HasMember("lasScaleZ")) {
						double tmpScale = appCommonDict["lasScaleZ"].GetDouble();
						if (tmpScale >= 1.0e-12) {
							lasScaleZ = tmpScale;
						}
					}

					if (appCommonDict.HasMember("threadCount")) {
						threadCount = appCommonDict["threadCount"].GetInt();
					}

					if (appCommonDict.HasMember("fromDatetime")) {
						fromDatetime = appCommonDict["fromDatetime"].GetString();
					}

					if (appCommonDict.HasMember("toDatetime")) {
						toDatetime = appCommonDict["toDatetime"].GetString();
					}
				}
			}

			loadExtraConfig(doc);
		}

	public:
		std::string getOwnerId() {
			return ownerId;
		}

		std::string getServerId() {
			return serverId;
		}

		std::string getBatchType() {
			return batchType;
		}

		std::string getProjectRoot() {
			return projectRoot;
		}
		
		std::string getDataRoot() {
			return dataRoot;
		}

		std::string getPointInRoot() {
			return pointInRoot;
		}
		std::string getPointOutRoot() {
			return pointOutRoot;
		}

		std::string getConfirmRoot() {
			return confirmRoot;
		}

		std::string getMapRoot() {
			return mapRoot;
		}

		std::string getDrmRoot() {
			return drmRoot;
		}

		//std::string getCitygmlInRoot() {
		//	return citygmlInRoot;
		//}
		//std::string getCitygmlOutRoot() {
		//	return citygmlOutRoot;
		//}

		int getColorByte() {
			return colorByte;
		}

		int getDecimalPoint() {
			return decimalPoint;
		}

		int getMapEpsg() {
			return mapEpsg;
		}

		int getPointInLasEpsg() {
			return pointInLasEpsg;
		}

		int getPointCsvEpsg() {
			return pointCsvEpsg;
		}

		int getPointOutLasEpsg() {
			return pointOutLasEpsg;
		}

		int getConfirmLasEpsg() {
			return confirmLasEpsg;
		}

		std::string getFromDatetime() {
			return fromDatetime;
		}

		std::string getToDatetime() {
			return toDatetime;
		}

		std::string getDbHost() {
			return dbHost;
		}
		std::string getDbName() {
			return dbName;
		}
		std::string getDbUser() {
			return dbUser;
		}
		std::string getDbPass() {
			return dbPass;
		}
		std::string getUserDbHost() {
			return userDbHost;
		}
		std::string getUserDbName() {
			return userDbName;
		}
		std::string getUserDbUser() {
			return userDbUser;
		}
		std::string getUserDbPass() {
			return userDbPass;
		}

		double getLasScale() {
			return lasScale;
		}

		double getLasScaleZ() {
			return lasScaleZ;
		}

		int getThreadCount() {
			return threadCount;
		}

		void setVerbose(int level) {
			if (level >= 5) {
				loglevel = "trace";
			}
			else if (level >= 4) {
				loglevel = "debug";

			}
			else if (level >= 3) {
				loglevel = "info";

			}
			else if (level >= 2) {
				loglevel = "warn";

			}
			else if (level >= 1) {
				loglevel = "error";

			}
		}

		void setBatchId(std::string& batchFolder) {
			this->batchId = batchFolder;
		}

		std::string& getBatchId() {
			return batchId;
		}

		void setBatchFile(std::string& batchFile) {
			this->batchFile = batchFile;
		}

		std::string& getBatchFile() {
			return batchFile;
		}

		std::string getLoggerName() {
			return loggerName;
		}

		void loadConfig(int argc, char** argv) {
			const std::vector<std::string> args(argv, argv + argc);
			std::vector<std::string> options;

			std::string appName = CommonUtil::getAppName(argv[0]);
			loggerName = appName;

			int i = -1;
			for (const std::string& arg : args) {
				i++;
				//変換対象(argparseのデフォルト実装では「-v 3」のような指定に対応していないため、「--verbose 3」に変換してから処理させる
				std::map<std::string, std::string> convertMap = {
					{"-c", "--config"},
					{"-v", "--verbose" }
				};

				//[-] --> [--]変換用
				auto replaceFunc = [&convertMap](const std::string& arg, const std::string& from) {
					if (convertMap.find(from) == convertMap.end()) {//ない場合
						return arg;
					}
					std::string str(convertMap[from]);
					str += arg.substr(from.length());

					return str;
					};

				//[--]のものはそのまま追加
				if (arg.length() > 1 && arg[0] == '-' && arg[1] == '-') {
					options.push_back(arg);
				}
				else if (arg[0] == '-') {
					//[=]がなければそのまま追加
					if (arg.find("=") < 0) {
						if (convertMap.find(arg) == convertMap.end()) {
							options.push_back(arg);
						}
						else {
							options.push_back(convertMap[arg]);
						}
					}
					else {
						//[-] --> [--]
						options.push_back(replaceFunc(arg, arg.substr(0, 2)));
					}
				}
				else {
					options.push_back(arg);
				}

			}


			//コマンド引数
			loadArguments(options, appName);

			//config.jsonをロード
			if (configJsonFile.length() > 0) {
				loadConfigFile(configJsonFile);
			}

			if (argVerbose > 0) {
				setVerbose(argVerbose);
			}

			//ロガーを用意
			prepareLogger(appName);

		}

	};

};






