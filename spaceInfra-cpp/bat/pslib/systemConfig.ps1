$C_featureIdArr = @{
    "POWERLINE" = @{
        "name" = "高圧線"
        "description" = ""
    }
    "STEELTOWER" = @{
        "name" = "鉄塔"
        "description" = ""
    }
    "WATER" = @{
        "name" = "水面"
        "description" = ""
    }
    "ROAD" = @{
        "name" = "道路"
        "description" = ""
    }
    "BUILDING" = @{
        "name" = "ビル"
        "description" = ""
    }
    "RAILWAY" = @{
        "name" = "鉄道"
        "description" = ""
    }
    "GROUND" = @{
        "name" = "地表"
        "description" = ""
    }
}

$C_execPathArr = [ordered]@{
    "copyEnvToWeb"= @{
        "batType" = "public"
        "name" = "バッチ環境設定値をWebToolにコピー"
        "description" = "-cmd copyEnvToWeb                          検証ツールにenv情報を連携"
        "detail" = "pslib/env.ps1の内容を Web側の storage/app の直下に .envFromBat.json として出力します"
        "status" = "実行可能"
        "arguments" = ""
        "appPath" = ""
        }
    "makeMapJsonFromPmResult"= @{
        "batType" = "public"
        "name" = "地図用地物JSON作成"
        "description" = "-cmd makeMapJsonFromPmResult               地物抽出結果から検証ツール表示用JSONを生成する"
        "detail" = "生成された地物LASから、生成空間IDを地図に表示するためのJSONを作成します"
        "status" = "実行可能"
        "arguments" = ""
        "appPath" = ""
        }
    "makeMergeJsonFromConstFolder"= @{
        "batType" = "public"
        "name" = "地図用地物JSON作成"
        "description" = "-cmd makeMergeJsonFromConstFolder               env定義のフォルダから検証ツール表示用JSONを生成する"
        "detail" = " env定義のフォルダのLASから、生成空間IDを地図に表示するためのJSONを作成します"
        "status" = "実行可能"
        "arguments" = ""
        "appPath" = ""
        }
    "mergeSpatialLasFromPmResult"= @{
        "batType" = "public"
        "name" = "空間ID別地物マージ"
        "description" = "-cmd mergeSpatialLasFromPmResult           地物抽出結果から空間ID別に全地物をマージします"
        "detail" = "生成された地物LASから、空間ID単位に全地物をマージします"
        "status" = "実行可能"
        "arguments" = ""
        "appPath" = ""
        }
    "makeVoxel" = @{
        "batType" = "public"
        "name" = "ボクセル作成処理"
        "description" = "-cmd makeVoxel -btIds ? -featureId ?       makeVoxel（ボクセル作成）を起動する"
        "detail" = "pointMatchにより生成された地物別＆マージ後＆元データLASから ボクセルを作成します。"
        "status" = "実行可能"
        "arguments" = "-btIds バッチID指定"
        "appPath" = "c:/cppProjectRoot/project/spaceInfraCpp/MakeVoxel/x64/Release/MakeVoxel.exe"
        }
    "clearVoxel" = @{   
        "batType" = "public" 
        "name" = "ボクセルクリア処理"
        "description" = "-cmd clearVoxel -btIds ?                   clearVoxel（ボクセルクリア処理）を起動する"
        "detail" = "指定されたバッチフォルダ内のボクセルとハッシュをクリアします"
        "status" = "実行可能"
        "arguments" = "-btIds バッチID指定"
        "appPath" = "c:/cppProjectRoot/project/spaceInfraCpp/ClearVoxel/x64/Release/ClearVoxel.exe"
        }
    "mergeVoxel" = @{
        "batType" = "public"
        "name" = "ボクセルマージ処理（zip）"
        "description" = "-cmd mergeVoxel -btIds ? -featureId ?      mergeVoxel（ボクセルマージ）を起動する"
        "detail" = "makeVoxelにより生成されたZip版Voxelデータについて全地物をマージします"
        "status" = "実行可能"
        "arguments" = "-btIds バッチID指定"
        "appPath" = "c:/cppProjectRoot/project/spaceInfraCpp/MergeVoxel/x64/Release/MergeVoxel.exe"
        }
    "lasMerge"= @{
        "batType" = "public"
        "name" = "LASマージ処理"
        "description" = "-cmd mergeLas -fileId ?                    指定されたJSONファイルに従い、LASマージを行う"
        "detail" = "jsonで指定されたLasをマージしてjsonで指定されたファイルに出力します"
        "status" = "実行可能"
        "arguments" = "-fileId マージ指示JSONファイル"
        "appPath" = "c:/cppProjectRoot/project/spaceInfraCpp/lasMerge/x64/Release/lasMerge.exe"
        }
    "clearLog"= @{
        "batType" = "public"
        "name" = "ログファイルのクリア"
        "description" = "-cmd"
        "detail" = "指定されたログファイルを削除します"
        "status" = "作成予定"
        "arguments" = "-log file または   -log ログパス"
        "appPath" = ""
        }
    "makeCityGml"= @{
        "batType" = "publicCityGML"
        "name" = "CityGMLからLAS作成"
        "description" = "-cmd makeCityGml -btIds ?                  CityGMLからLAS作成"
        "detail" = "CityGMLからLASを作成する"
        "status" = "実行可能"
        "arguments" = "-btIds バッチID指定"
        "appPath" = "c:/cppProjectRoot/project/spaceInfraCpp/MakeCityGmlLas/x64/Release/MakeCityGmlLas.exe"
        }
    "spaceMerge"= @{
        "batType" = "publicCityGML"
        "name" = "CityGMLバッチ間空間ＩＤのマージ"
        "description" = "-cmd spaceMerge -btIds ?                  CityGMLバッチ間空間ＩＤのマージ"
        "detail" = "CityGMLバッチ間空間ＩＤのマージする"
        "status" = "実行可能"
        "arguments" = "-btIds バッチID指定"
        "appPath" = "c:/cppProjectRoot/project/spaceInfraCpp/SpaceMerge/x64/Release/SpaceMerge.exe"
        }
    "confirmCityGmlData"= @{
        "batType" = "publicCityGML"
        "name" = "CityGML利用者側への確定処理"
        "description" = "-cmd confirmCityGmlData -btIds ?                  CityGML利用者側への確定処理"
        "detail" = "CityGML利用者側への確定処理"
        "status" = "実行可能"
        "arguments" = "-btIds バッチID指定"
        "appPath" = "c:/cppProjectRoot/project/spaceInfraCpp/ConfirmCityGmlData/x64/Release/ConfirmCityGmlData.exe"
    }
    "makeRadioWaveVoxel"= @{
        "batType" = "publicRadioWave"
        "name" = "電波情報からVoxelを作成し確定処理まで実施する"
        "description" = "-cmd makeRadioWaveVoxel -btIds ?                  電波情報からVoxelを作成し確定処理まで実施"
        "detail" = "電波情報からVoxelを作成し確定処理まで実施"
        "status" = "実行可能"
        "arguments" = "-btIds バッチID指定"
        "appPath" = "c:/cppProjectRoot/project/spaceInfraCpp/MakeRadioWaveVoxel/x64/Release/MakeRadioWaveVoxel.exe"
    }
    "makePopulationData"= @{
        "batType" = "publicPopulationData"
        "name" = "人流情報をDBに取り込みます"
        "description" = "-cmd makePopulationData -btIds ?            人流情報をDBに取り込み"
        "detail" = "人流情報をDBに取り込み"
        "status" = "実行可能"
        "arguments" = "-btIds バッチID指定"
        "appPath" = "c:/cppProjectRoot/project/spaceInfraCpp/MakePopulationData/x64/Release/MakePopulationData.exe"
    }
    "lazMerge"= @{
        "batType" = "public"
        "name" = "LAZマージ処理"
        "description" = "-cmd mergeLas -fileId ?                    指定されたJSONファイルに従い、LASマージを行う"
        "detail" = "jsonで指定されたLaZをマージしてjsonで指定されたファイルに出力します"
        "status" = "実行可能"
        "arguments" = "-fileId マージ指示JSONファイル"
        "appPath" = "c:/cppProjectRoot/project/spaceInfraCpp/lazMerge/x64/Release/lazMerge.exe"
    }
    "makeDroneRouteVoxel"= @{
        "batType" = "onlineBatch"
        "name" = "ドローン航路voxel作成"
        "description" = "-オンラインバッチから、makeDroneRoute.ps1 の中で動きます。"
        "detail" = "ドローン航路Voxelを作成します。"
        "status" = "実行可能"
        "arguments" = "-droneRouteInfoId ドローン航路情報ID"
        "appPath" = "c:/cppProjectRoot/project/spaceInfraCpp/MakeDroneRouteVoxel/x64/Release/MakeDroneRouteVoxel.exe"
    }
    "makeDroneRouteMergeJson"= @{
        "batType" = "onlineBatch"
        "name" = "ドローン航路json作成"
        "description" = "-オンラインバッチから、makeDroneRoute.ps1 の中で動きます。"
        "detail" = "ドローン航路でマージするjsonファイルを作成します。"
        "status" = "実行可能"
        "arguments" = "--droneRouteInfoId ドローン航路情報ID"
        "appPath" = "c:/cppProjectRoot/project/spaceInfraCpp/MakeDroneRouteMergeJson/x64/Release/MakeDroneRouteMergeJson.exe"
    }
    "makePointCloudVoxel"= @{
        "batType" = "publicPointCloud"
        "name" = "点群情報からVoxelを作成し確定処理まで実施する"
        "description" = "-cmd makePointCloudVoxel -btIds ?                  点群情報からVoxelを作成し確定処理まで実施"
        "detail" = "点群情報からVoxelを作成し確定処理まで実施"
        "status" = "実行可能"
        "arguments" = "-btIds バッチID指定"
        "appPath" = "c:/cppProjectRoot/project/spaceInfraCpp/MakePointCloudVoxel/x64/Release/MakePointCloudVoxel.exe"
    }

}

function printBtIdsDef {
    Param (
        [string]$owId
    )
    putMsg("【定義済みバッチID】")
    foreach ($btIdDef in $C_ownerDefArr[$owId]["btIds"]) {
        putMsg("owId : "+$owId+"  btId : "+$btIdDef)
    }
}

function printExecCmd {
    Param (
    )
    $keys = $C_execPathArr.Keys
    foreach ($key in $keys) {
        putMsg ($key+" : "+$C_execPathArr[$key]["name"])
    }
}

function printFeaturesExplain {
    $keys = $C_featureIdArr.Keys
    foreach ($key in $keys) {
        putMsg ("-featureId "+$key+" : "+$C_featureIdArr[$key]["name"])
    }
    putMsg("-featureId all は、全地物をマージします")
    putMsg("-featureId each は、全地物を別々にマージします")
    putMsg("-featureId voxel は、voxel（las)をマージします")
}

function printFeatures {
    $keys = $C_featureIdArr.Keys
    foreach ($key in $keys) {
        putMsg ("-featureId "+$key+" : "+$C_featureIdArr[$key]["name"])
    }
}
function printGroupExplain {
    putMsg("groupId は、potreeのHTMLフォルダIDになります")
    putMsg("指定なしの時は  latest  フォルダを上書きます")
}

function printRepConfigExplain {
    putMsg ("-repConfig コンフィグファイルパス   オーナID指定のデフォルトconfig を置き換えます")
    putMsg ("-repConfig spId   空間ID指定の時、オーナID指定のデフォルトconfig内の空間ID定義をオーバライドします")
}

function printLogExplain {
    putMsg ("指定のない時コンソールに出力されます")
    putMsg ("-log file    ログルート/batManager_"+$owId+".log  にログ出力されます")
    putMsg ("-log ログファイルパス    指定されたログファイルに出力されます")
}