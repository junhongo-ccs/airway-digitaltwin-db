#pragma once
#include <string>
#include <chrono>

class Config {
private:
    Config() = default;
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    std::string appId = "";
	//common 共通設定
    std::string ownerId = "";
    std::string confirmRoot = "";
    std::string execRoot = "";
    std::string dataRoot = "";
    std::string pointRoot = "";
    std::string mapRoot = "";
    std::string logRoot = "";
    std::string logsink = "console";
    std::string loglevel = "info";
    int colorByte = 2;
    int decimalPoint = 99;

	// MYSQL DB接続情報
    std::string btDbHost = "";
    std::string btUserDbHost = "";
    std::string btUserDbName = "";
    std::string btUserDbUser = "";
    std::string btUserDbPass = "";

    //app>common
    std::string mapEpsg = "";
    std::string pointInLasEpsg = "";
    std::string pointCsvEpsg;
    std::string pointOutLasEpsg = "";
    std::string confirmLasEpsg = "";
	std::string fromDatetime = "";
	std::string toDatetime = "";
    
	//app>MakeRadioWaveVoxel
    std::string makeRadioWaveVoxelInput = "";
    std::string makeRadioWaveVoxelInputDrive = "";
    std::string makeRadioWaveVoxelInputRoot = "";
    std::string makeRadioWaveVoxelInputProcessingResults = "";
    std::string makeRadioWaveVoxelInputProcessingResultsDrive = "";
    std::string makeRadioWaveVoxelInputProcessingResultsRoot = "";
    std::string makeRadioWaveVoxelOutput = "";
    std::string makeRadioWaveVoxelOutputDrive = "";
    std::string makeRadioWaveVoxelOutputRoot = "";
    std::string makeRadioWaveVoxelOutputFormat = "laz";
    std::string makeRadioWaveVoxelOutputLevel = "1";

    std::string makeRadioWaveVoxelColor = "";
    // 入力の電波情報UTM座標を分割するためのパラメータ
	double mapSegPointStartLng = 0.0;
	double mapSegPointStartLat = 0.0;
	double mapSegPointEndLng = 0.0;
	double mapSegPointEndLat = 0.0;

    std::string config_file_path = "";
    
    

    void processOutputPath(const std::string& output_relative_path);
    std::string getCurrentDateTime() const;
    std::string replaceBatchDateTime(const std::string& path) const;

public:
    void loadConfig(std::string configFileName);
    std::string getDir(std::string app_id, std::string ioId, std::string batId);
    void prepareLogger(std::string appId, std::string loggerName);
    std::string getRadioWaveVoxelOutputFormat();
    std::string getInputFolder() const;
    std::string getOutputFilePath() const;
    std::string getConfirmLasEpsg() const;
	std::string getFromDatetime() const;
	std::string getToDatetime() const;

    std::string getMakeRadioWaveVoxelColor() const;
    
    std::string getLoggerName() const;

	std::string getDbHost() const;
    std::string getUserDbHost() const;
    std::string getUserDbName() const;
	std::string getDbUser() const;
	std::string getDbPass() const;
	double getMapSegPointStartLng() const;
	double getMapSegPointStartLat() const;
	double getMapSegPointEndLng() const;
	double getMapSegPointEndLat() const;
    
	//! @brief Configクラスのインスタンスを取得する関数
    //! 
	//! @return Config& Configクラスのインスタンス
    static Config& getInstance() {
        static Config _config;
        return _config;
    }
};