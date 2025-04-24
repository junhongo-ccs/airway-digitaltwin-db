#
#
# この設定を変更した場合は、Webtool側との連携を行うために下記　powershell　バッチを動かす必要があります
# C:\cppProjectRoot\project\spaceInfraCpp\bat\pslib
# ./batManager -owId localTest -cmd copyEnvToWeb
#
#

$C_debug = $false

[string] $C_PSScriptRoot = "C:/cppProjectRoot/project/spaceInfraCpp/bat/pslib"
[string] $C_tmpOutputRoot = "c:/data/work"
[string] $C_webStorageAppRoot = "c:/php8ProjectRoot/project/spwebtool/storage/app"
[string] $C_webPublicDataRoot = "c:/php8ProjectRoot/project/spwebtool/storage/app/public/data"
[string] $C_potreeConverterPath = "C:/PotreeConverter211/PotreeConverter.exe"
[string] $C_logFileRoot = "C:/cppProjectRoot/project/spaceInfraCpp/dat/log"
[string] $C_mergeConstRoot = "f:/data/xxxxx/point/XXXX/w02"

$C_ownerDefArr = @{
    "rpcCityGML" = @{
        "name" = "CityGML・ローカルテスト環境"
        "lat" = "35.613069"
        "lng" = "139.734707"
        "zoom" = "17"
        "batType" = "publicCityGML"
        "dataSourceId" = ""
        "pointMatchInputFolder" = ""       # TODO 定義なくてもエラーにしない
        "pointMatchOutDrive" = ""       # TODO 定義なくてもエラーにしない
        "pointMatchOutFolder" = ""       # TODO 定義なくてもエラーにしない
        "btIds" = @(       # TODO 定義なくてもエラーにしない
            "XXXXXXX"
        )
        "data3d" = @(       # TODO 定義なくてもエラーにしない
            ""
        )
    }
}


function printEnv {
    Param (
    )
    putMsg ("C_debug : "+$C_debug)
    putMsg ("C_PSScriptRoot : "+$C_PSScriptRoot)
    putMsg ("C_tmpOutputRoot : "+$C_tmpOutputRoot)
    putMsg ("C_webStorageAppRoot : "+$C_webStorageAppRoot)
    putMsg ("C_webPublicDataRoot : "+$C_webPublicDataRoot)
    putMsg ("C_potreeConverterPath : "+$C_potreeConverterPath)
    putMsg ("C_logFileRoot : "+$C_logFileRoot)
    putMsg ("C_mergeConstRoot : "+$C_mergeConstRoot)
}

function printOwnerIdsDef {
    Param (
    )
    putMsg("<< 定義済みオーナーID >>")
    $keys = $C_ownerDefArr.Keys
    foreach ($key in $keys) {
        putMsg($C_ownerDefArr[$key]["name"]+"　　：　下記コマンドで詳細利用方法を参照できます")
        putMsg("./batManager -owId "+$key)
    }
}
