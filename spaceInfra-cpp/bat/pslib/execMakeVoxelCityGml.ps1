# UTF-8 BOM付でないと文字化けが発生することがあるので注意
# 初回　Set-ExecutionPolicy RemoteSigned を　powershell　Terminal（管理者として実行）から実行しておく必要がある

function execMakeVoxelCityGml {
    Param (
        $configPath,
        [string]$owId,
        [string]$btIds
    )

    [string] $appName = "makeVoxel"
    [string] $AppName = "MakeVoxel"
    [string] $execPath = $C_execPathArr[$appName]["appPath"]
    [string] $clearExecPath = $C_execPathArr["clearVoxel"]["appPath"]

    if ($btIds -eq "") {
        putMsg("このコマンドのオプション指定は下記のとおりです")
        putMsg("./batManager -owId "+$owId+" -cmd "+$appName+" "+$C_execPathArr[$appName]["arguments"])
        printBtIdsExplain $owId $appName
        return 1
    } elseif ($btIds -eq "?") {
        printBtIdsExplain $owId $appName
        return 1
    }

    putLog ("batManager.ps1 exec"+$AppName+" started     configPath : "+$configPath)

    $btIdArr = getBtIdArr $owId $btIds
    putMsg("以下のバッチIDを実行します : "+$btIdArr)
    foreach ($btIdDef in $btIdArr) {
        putMsg("start : "+$btIdDef)
        # バッチフォルダ内のボクセルとハッシュをクリアします
        $proc = Start-Process -FilePath  $clearExecPath -ArgumentList ($btIdDef+" --config="+$configPath) -NoNewWindow -PassThru -Wait -WorkingDirectory $C_PSScriptRoot
        if ($proc.ExitCode -ne 0) {
            return $proc.ExitCode
        }
        # 地物分離処理結果LASのボクセル作成
        $proc = Start-Process -FilePath  $execPath -ArgumentList ($btIdDef+" --config="+$configPath+" --input=inputProcessingResults") -NoNewWindow -PassThru -Wait -WorkingDirectory $C_PSScriptRoot
        if ($proc.ExitCode -ne 0) {
            return $proc.ExitCode
        }
    ##    # 地物分離前のオリジナルLASのボクセル作成
    ##    $proc = Start-Process -FilePath  $execPath -ArgumentList ($btIdDef+" --config="+$configPath+" --input=inputOriginal") -NoNewWindow -PassThru -Wait -WorkingDirectory $C_PSScriptRoot
    ##    if ($proc.ExitCode -ne 0) {
    ##       return $proc.ExitCode
    ##   }
    }

    putLog ("batManager.ps1 exec"+$AppName+" ended")
    return 0

}
