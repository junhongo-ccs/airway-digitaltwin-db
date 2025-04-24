# UTF-8 BOM付でないと文字化けが発生することがあるので注意
# 初回　Set-ExecutionPolicy RemoteSigned を　powershell　Terminal（管理者として実行）から実行しておく必要がある

Param(
    [string]$cmd="",            # コマンド 実行アプリ又はその組み合わせ
    [string]$owId="",           # オーナーID
    [string]$btIds="",          # バッチID　（ALL or batId or batId1:batId2:batId3）
    [string]$spIds="",          # 空間ID  (batId1/17-1-1111-2221:batId1/17-1-1111-2222:batId1/17-1-1111-2223)
    [string]$groupId="latest",  # ３DWeb　html group ID
    [string]$featureId="",      # 地物ID（road,building,water,steeltower,powerline,railway）又は　all
    [string]$repConfig="",      # configの置換　（チューニングオプションの指定などに利用）
    [string]$fileId="",         # LasMerge時のマージ対象を示すJSON
    [string]$clear="",          # データクリア指示
    [string]$log="",            # 空文字：コンソール出力　file：C:\cppProjectRoot\project\spaceInfraCpp\dat\log\batManager.logに追加書き　その他は指定パスのログファイルに追加書き
    [string]$debug=$false       # 
)

# include cmnFunctions
. ".\env.ps1"
. ".\systemConfig.ps1"
. ".\cmnFunctions.ps1"
. ".\appFunctions.ps1"
. ".\execCopyEnvToWeb.ps1"
. ".\execCsvsp2space.ps1"
. ".\execLas2csvsp.ps1"
. ".\execMake3D.ps1"
. ".\execMakeMapJsonFromPmResult.ps1"
. ".\execMakeMergeJsonFromConstFolder.ps1"
. ".\execMakeMergeJsonFromPmResult.ps1"
. ".\execMergeSpatialLasFromPmResult.ps1"
. ".\execMakeVoxel.ps1"
. ".\execMakeHash.ps1"
. ".\execMergeVoxel.ps1"
. ".\execMergeLas.ps1"
. ".\execPointMatch.ps1"
. ".\execPointMatchDetails.ps1"
. ".\execConfirmData.ps1"
. ".\execSpaceInit.ps1"
. ".\execUpdateMapData.ps1"
. ".\execConfirmUserData.ps1"
. ".\execMakeCityGml.ps1"
. ".\execSpaceMerge.ps1"
. ".\execConfirmCityGmlData.ps1"
. ".\execMakeVoxelCityGml.ps1"
. ".\execMakeRadioWaveVoxel.ps1"
. ".\execMakePopulationData.ps1"
. ".\execMakePointCloudVoxel.ps1"

$C_logFilePath = ""

# 現在ディレクトリの取得
# [String] $currentPath = Get-Location

if ($C_debug -eq $true) {
    printEnv
    printExecCmd
}

# オーナIDチェック
if ($owId -ne "") {
    if ($owId -eq "?") {
        printOwnerIdsDef
        exit 1
    }
    if (checkOwnerId($owId) -eq $false) {
        putLog("owId : "+$owId+" は、config.ps1に、定義されていません") "error"
        printOwnerIdsDef
        exit 9
    }
} else {
    putLog "オーナーID(owId) の指定は必須です" "Error"
    printOwnerIdsDef
    exit 9
}

[string]$configPath = "../config/commonConfig-"+$owId+".json"

if (Test-Path $configPath) {
    $commonConfig = ConvertFrom-Json -InputObject (Get-Content $configPath -Raw)
    # 下記の様に　commonConfigの取得値を利用できる
    if ($owId -ne $commonConfig.common.ownerId) {
        putLog ("指定オーナーID と commonConfigのオーナーIDが一致しません "+$commonConfig.common.ownerId) "error"
        exit 9
    }
} else {
    putLog ("指定オーナーIDのcommonConfigが存在しません "+$configPath) "error"
    exit 9
}

$batType = $commonConfig.common.batType

# ログファイルのリセット
if ($log -eq "?") {
    printLogExplain
    exit 1
} elseif ($log -eq "") {
    $C_logFilePath = ""
} elseif ($log -eq "file") {
    $C_logFilePath = $C_logFileRoot+"/batManager_"+$owId+".log"
} else {
    $C_logFilePath = $log
}

# ログファイルを削除します
if ($cmd -eq "clearLog") {
    if ($C_logFilePath -eq "") {
        putLog ("-log でログファイルが指定されていません") "error"
        putMsg ("-log でログファイルが指定されていません") "error"
        exit 9
    } else {
        if (Test-Path $C_logFilePath) {
            Remove-item $C_logFilePath
            putMsg ("ログファイルを削除しました")
            exit 0
        } else {
            putLog ("-log で指定されたログファイルが存在しません") "error"
            putMsg ("-log で指定されたログファイルが存在しません") "error"
            exit 9
        }
    }
}
if ($log -ne "") {
    Write-Host "ログは "$C_logFilePath" に出力します" -ForegroundColor Green
}

# バッチIDチェック
if (($btIds -ne "") -And ($btIds -ne "?")) {
    if (checkBtIds($owId,$btIds) -eq $false) {
        putLog ("btIds : "+$btIds+" は、指定のオーナIDで、configに未定義です") "error"
        printBtIdsExplain $owId $cmd
        exit 9
    }
    # putLog("btIds : "+$btIds)
}

# 空間IDチェック
if ($spIds -ne "") {
    if (checkBtIds($owId,$spIds) -eq $false) {
        putLog ("spIds : "+$spIds+" に含まれるバッチIDは、指定のオーナIDで、configに未定義です") "error"
        printSpIdsDef($owId)
        exit 9
    }
    putLog "spIds : "$spIds
}

# バッチの環境設定をWebに転送します
if ($cmd -eq "copyEnvToWeb") {
    $retCd = execCopyEnvToWeb $configPath $owId
    
# 同一バッチ内で、同一の空間IDのデータをマージします
} elseif ($cmd -eq "csvsp2space") {
    $retCd = execCsvsp2space $configPath $owId $btIds
 
# インプットのデータを空間ID毎に分割します
} elseif ($cmd -eq "las2csvsp") {
    $retCd = execLas2csvsp $configPath $owId $btIds
    
# ３D作成バッチ
} elseif ($cmd -eq "make3D") {
    $retCd = execMake3D  $commonConfig $configPath $owId $groupId $featureId

# ポイントマッチ結果から　地図上への地物マーク表示用JSON作成
} elseif ($cmd -eq "makeMapJsonFromPmResult") {
    $retCd = execMakeMapJsonFromPmResult $commonConfig $configPath $owId
    
# ポイントマッチ結果から　地図上への地物マーク表示用JSON作成
} elseif ($cmd -eq "makeMergeJsonFromPmResult") {
    #各コマンド内から呼び出される（今のところ、直接呼出しは想定していない）
    
# envで指定されたフォルダから　地図上へLASマージ用JSON作成
} elseif ($cmd -eq "makeMergeJsonFromConstFolder") {
    #各コマンド内から呼び出される（今のところ、直接呼出しは想定していない）
    $retCd = execMakeMergeJsonFromConstFolder $commonConfig $configPath $owId

# ポイントマッチ結果から　地物を空間ID毎にマージします
} elseif ($cmd -eq "mergeSpatialLasFromPmResult") {
    $retCd = execMergeSpatialLasFromPmResult $commonConfig $configPath $owId

# ボクセル作成バッチ
} elseif ($cmd -eq "makeVoxel") {
    $retCd = execMakeVoxel $configPath $owId $btIds

# ハッシュ作成バッチ
} elseif ($cmd -eq "makeHash") {
    $retCd = execMakeHash $configPath $owId $btIds

# ボクセルマージ処理
} elseif ($cmd -eq "mergeVoxel") {
    $retCd = execMergeVoxel $configPath $owId $btIds

# JSON指定による LASマージバッチ
} elseif ($cmd -eq "mergeLas") {
    $retCd = execMergeLas $commonConfig $configPath $owId $fileId

# 地物別・点群抽出のみのバッチ
} elseif ($cmd -eq "pointMatch") {
    $retCd = execPointMatch $commonConfig $configPath $owId $btIds $spIds $repConfig

# 地物ID別・ボクセル抽出のバッチ
} elseif ($cmd -eq "pointMatchDetails") {
    $retCd = execPointMatchDetails $commonConfig $configPath $owId $btIds $spIds $repConfig

# 確定処理バッチ
} elseif ($cmd -eq "confirmData") {
    $retCd = execConfirmData $configPath $owId $btIds

# バッチ単位の分離空間IDのマージバッチ（一括のみ）
} elseif ($cmd -eq "spaceInit") {
    $retCd = execSpaceInit $configPath $owId $btIds

# 地図データ追加・削除（鉄塔）
} elseif ($cmd -eq "updateMapData") {
    $retCd = execUpdateMapData $commonConfig $configPath $owId $featureId

# ユーザー作成個別地物デー他の空間分割・ボクセル作成・確定処理
} elseif ($cmd -eq "confirmUserData") {
    $retCd = execConfirmUserData $configPath $owId $clear

# CityGmlからLASを作成する処理
} elseif ($cmd -eq "makeCityGml") {
    $retCd = execMakeCityGml $configPath $owId $btIds

# CityGmlバッチ間空間ＩＤのマージ
} elseif ($cmd -eq "spaceMerge") {
    $retCd = execSpaceMerge $configPath $owId $btIds

# CityGml利用者側への確定処理
} elseif ($cmd -eq "confirmCityGmlData") {
    $retCd = execConfirmCityGmlData $configPath $owId $btIds

# CityGmlボクセル作成バッチ
} elseif ($cmd -eq "makeVoxelCityGml") {
    $retCd = execMakeVoxelCityGml $configPath $owId $btIds

# 電波情報からVoxelを作成する処理
} elseif ($cmd -eq "makeRadioWaveVoxel") {
    $retCd = execMakeRadioWaveVoxel $configPath $owId $btIds

# 人流情報をDBに取り込む処理
} elseif ($cmd -eq "makePopulationData") {
    $retCd = execMakePopulationData $configPath $owId $btIds

# 点群情報をDBに取り込む処理
} elseif ($cmd -eq "makePointCloudVoxel") {
    $retCd = execMakePointCloudVoxel $configPath $owId $btIds

} elseif (($cmd -eq "?") -Or ($cmd -eq "")) {
    printCmdDef $owId $batType
    exit

} else {
    putLog ("cmd : "+$cmd+" は存在しません") "error"
    putMsg ("cmd 一覧を参照する場合は　./batManager -owId "+$owId+"　-cmd ?　　と入力してください")
    putMsg ("")
    exit 9
}

if ($retCd -gt 4) {
    putLog("異常終了しました "+$retCd) "error"
    exit $retCd
}

exit 0
