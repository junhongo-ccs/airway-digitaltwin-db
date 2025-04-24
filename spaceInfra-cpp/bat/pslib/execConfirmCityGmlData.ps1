# UTF-8 BOM付でないと文字化けが発生することがあるので注意
# 初回　Set-ExecutionPolicy RemoteSigned を　powershell　Terminal（管理者として実行）から実行しておく必要がある

function execConfirmCityGmlData {
    Param (
        $configPath,
        [string]$owId,
        [string]$btIds
    )

    [string] $appName = "confirmCityGmlData"
    [string] $AppName = "ConfirmCityGmlData"
    [string] $execPath = $C_execPathArr[$appName]["appPath"]

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
        # 第二引数のファイルID指定は直接アプリ起動で使用する
        # Las2csvsp %1 --espgLas=%2 --espgCsv=%3 --config=%configPath%
        # Las2csvsp %1 --config=$configPath
        $proc = Start-Process -FilePath  $execPath -ArgumentList ($btIdDef+" --config="+$configPath) -NoNewWindow -PassThru -Wait -WorkingDirectory $C_PSScriptRoot
        if ($proc.ExitCode -ne 0) {
            return $proc.ExitCode
        }
    }

    putLog ("batManager.ps1 exec"+$AppName+" ended")
    return 0

}
