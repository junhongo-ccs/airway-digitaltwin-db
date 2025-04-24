# UTF-8 BOM付でないと文字化けが発生することがあるので注意
# 初回　Set-ExecutionPolicy RemoteSigned を　powershell　Terminal（管理者として実行）から実行しておく必要がある

function checkOwnerId{
    Param (
        [string]$ownerId
    )
    foreach ($ownerDef in $C_ownerDefArr) {
        if ($ownerIdDef -eq $ownerId) {
            return $true
        }
    }
    return $false
}

function checkBtIds {
    Param (
        [string]$owId,
        [string]$btIds
    )
    if (! $C_ownerDefArr.ContainsKey($owId)){
        return $false;
    }
    if ($btIds -eq "all") {
        return $true
    }
    foreach ($btIdDef in $C_ownerDefArr[$owId]["btIds"]) {
        if ($btIdDef -eq $btId) {
            return $true
        }
    }
    return $false

}

function getBtIdArr {
    Param (
        [string]$owId,
        [string]$btIds
    )
    if (($btIds -eq "all") -Or ($btIds -eq "ALL") -Or ($btIds -eq "All")) {
        return $C_ownerDefArr[$owId]["btIds"]
    }
    $btIdArr = $btIds.split(":")
    return $btIdArr
}

function getSpIdArr {
    Param (
        [string]$owId,
        [string]$spIds
    )
    $spIdArr = $spIds.split(":")
    return $spIdArr
}

function printCmdDef {
    Param (
        [string]$owId,
        [string]$batType
    )
    putMsg("")
    putMsg("【定義済みコマンド一覧】")
    putMsg("-----------------------------")
    $keys = $C_execPathArr.Keys
    foreach ($key in $keys) {
        if (($C_execPathArr[$key]["status"] -eq "実行可能") -And ($C_execPathArr[$key]["batType"] -eq $batType)) {
            putMsg($C_execPathArr[$key]["description"])
        }
    }
    putMsg("")
    putMsg("【ログオプション共通】")
    putMsg("-----------------------------")
    putMsg("log指定なし            コンソールにログ出力")
    putMsg("- log fileパス         指定ファイルにログを出力")
    putMsg("- log file             ログルート設定/batManager_"+$owId+".logにログ出力")
    putMsg("")
}

function printBtIdsExplain {
    Param (
        [string]$owId,
        [string]$cmd
    )
    putMsg("【 btIds:バッチID指定方法 】")
    putMsg("バッチID単一指定　：　xxxxxx")
    putMsg("バッチID複数指定　：　xxxxxx:yyyyyyy")
    putMsg("バッチID全て　　　：　all")
    putMsg("指定可能なバッチIDは、次のコマンドで確認します　　./batManager -owId "+$owId+" -cmd "+$cmd+" -btIds ?")
}

function printFileIdExplain {
    Param (
        [string]$owId,
        [string]$cmd
    )
    putMsg("【 fileId:ファイルID指定方法 】")
    putMsg("lasマージのJSONファイルを指定します")
}

function printSpIdsExplain {
    Param (
        [string]$owId,
        [string]$cmd
    )
    putMsg("【 spIds:空間ID指定方法 】")
    putMsg("空間IDは、バッチID/ハイフン区切り空間IDで表現されます")
    putMsg("空間ID単一指定例　：　08ND59_17/0/115951/51896")
    putMsg("空間ID複数指定例　：　08ND59_17/0/115951/51896:08ND68_17/0/115937/51908")
}

#function printBtIdsExplain {
#    Param (
#        [string]$owId,
#        [string]$cmd
#    )
#    putMsg("【btIds:バッチID指定方法】")
#    foreach ($btIdDef in $C_ownerDefArr[$owId]["btIds"]) {
#        putMsg("./batManager -owId "+$owId+" -cmd "+$cmd+" -btId "+$btIdDef)
#    }
#    putMsg("./batManager -owId "+$owId+" -cmd "+$cmd+" -btId all            全てのバッチID")
#    putMsg("./batManager -owId "+$owId+" -cmd "+$cmd+" -btId xxxxxx:yyyyy   複数指定")
#}

function getPointMatchOutputRoot {
    Param (
        $commonConfig
    )
    $pointRoot = $commonConfig.common.pointRoot
    if ($commonConfig.common.mapId -ne "ZRN") {
        $pointPath,$option = $commonConfig.app.PointMatchCommon.pointBatchPath.output.split(",")
    } else {
        $pointPath,$option = $commonConfig.app.PointMatch.pointBatchPath.output.split(",")
    }
    if ($null -ne $option) {
        $optionKey,$optionVal = $option.split("=")
        if ($null -ne $optionVal) {
            if ("drive" -eq $optionKey) {
                $pointRootDrive,$pointRootPath = $pointRoot.split(":")
                return ($optionVal+":"+$pointRootPath)
            } elseif ("root" -eq $optionKey) {
                return $optionVal
            }
        }
    }
    return $pointRoot
}
function getVoxelOutputRoot {
    Param (
        $commonConfig
    )
    $pointRoot = $commonConfig.common.pointRoot
    $voxelPath,$option = $commonConfig.app.MakeVoxel.pointBatchPath.output.split(",")
    if ($null -ne $option) {
        $optionKey,$optionVal = $option.split("=")
        if ($null -ne $optionVal) {
            if ("drive" -eq $optionKey) {
                $pointRootDrive,$pointRootPath = $pointRoot.split(":")
                return ($optionVal+":"+$pointRootPath)
            } elseif ("root" -eq $optionKey) {
                return $optionVal
            }
        }
    }
    return $pointRoot
}

function getPointMatchOutputForder {
    Param (
        $commonConfig
    )
    if ($commonConfig.common.mapId -ne "ZRN") {
        $pointPath,$option = $commonConfig.app.PointMatchCommon.pointBatchPath.output.split(",")
    } else {
        $pointPath,$option = $commonConfig.app.PointMatch.pointBatchPath.output.split(",")
    }
    return $pointPath
}

function getVoxelOutputForder {
    Param (
        $commonConfig
    )
    $voxelPath,$option = $commonConfig.app.MakeVoxel.pointBatchPath.output.split(",")
    return $voxelPath
}

function getMapRoot {
    Param (
        $commonConfig
    )
    $mapRoot = $commonConfig.common.mapRoot
    return $mapRoot
}

function checkFeaturesInJson() {
    Param (
        [string]$featureId,
        [string]$jsonFileId
    )
    $json = ConvertFrom-Json -InputObject (Get-Content $jsonFileId -Raw)
    if (0 -eq $json.exec.input.count) {
        return $false
    }
    Return $true
}