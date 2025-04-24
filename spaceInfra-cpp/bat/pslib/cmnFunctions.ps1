# UTF-8 BOM付でないと文字化けが発生することがあるので注意
# 初回　Set-ExecutionPolicy RemoteSigned を　powershell　Terminal（管理者として実行）から実行しておく必要がある

function putLog {
    Param (
        [string]$message,
        [string]$type=""
    )
    [Datetime]$time = Get-Date
    if ($C_logFilePath -eq "") {
        if ($type -eq "start" ) {
            Write-Host ""
            Write-Host $time" : "$message -ForegroundColor Blue
        } elseif ($type -eq "end" ) {
            Write-Host $time" : "$message -ForegroundColor Blue
            Write-Host ""
        } elseif ($type -eq "error" ) {
            Write-Host $time" : "$message -ForegroundColor Red
            Write-Host ""
        } elseif (($type -eq "warn") -Or ($type -eq "warning") ) {
            Write-Host $time" : "$message -ForegroundColor Yellow
        } else {
            Write-Host $time" : "$message
        }
    } else {
        if (Test-Path $C_logFilePath) {
            Add-Content $C_logFilePath $time" : "$message
        } else {
            Set-Content $C_logFilePath $time" : "$message
        }
    }
}

function putDebugLog {
    Param (
        [string]$message,
        [string]$type=""
    )
    if ($C_debug) {
        putLog $message $type
    }

}

function putMsg {
    Param (
        [string]$message
    )
    Write-Host $message -ForegroundColor Green
}


function getBatUniqId {
    Param (
        [string]$ownerId
    )
    [string]$time = Get-Date
    $time = $time.Replace("/","")
    $time = $time.Replace(" ","")
    $time = $time.Replace(":","")
    return $ownerId+"_"+$time
}
