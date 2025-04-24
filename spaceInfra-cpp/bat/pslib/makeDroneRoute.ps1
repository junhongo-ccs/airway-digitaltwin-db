# UTF-8 BOM付でないと文字化けが発生することがあるので注意
# 初回　Set-ExecutionPolicy RemoteSigned を　powershell　Terminal（管理者として実行）から実行しておく必要がある

Param (   
    [string]$droneRouteInfoId = "",     # ドローン航路情報ID
    [string]$droneRouteId = "",         # ドローン航路ID
    [string]$configPath=""              # configPath（"--config=C:/cppProjectRoot/project/spaceInfraCpp/bat/config/commonConfig-rpcDroneOnlineBatch.json"）
)

Set-Location c:/cppProjectRoot/project/spaceInfraCpp/bat/pslib

# include cmnFunctions
. ".\env.ps1"
. ".\systemConfig.ps1"
. ".\cmnFunctions.ps1"
. ".\appFunctions.ps1"

#
#  パラメータチェック
#
if ($droneRouteInfoId -eq "?" -Or $droneRouteInfoId -eq "") {
    putMsg("droneRouteInfoId is null")
    exit 1
} elseif ($droneRouteId -eq "?" -Or $droneRouteId -eq "") {
    putMsg("droneRouteId is null")
    exit 1
} elseif ($configPath -eq "?" -Or $configPath -eq "") {
    putMsg("configPath is null")
    exit 1
}


#
#   変数セット
#
# #1 set

# #2 set
$fileJsonPath = $C_tmpOutputRoot + "/json/droneRoute/" + $droneRouteId 
$fileJsonID = $fileJsonPath + "/droneRouteLazMerge.json"
$lazOutfilePath = $C_tmpOutputRoot + "/mergeDroneRoutelas/"
$lazOutfileId = $lazOutfilePath + "droneRoute_"  + $droneRouteId + ".las" 
# 
#Write-Host "fileJsonID= $fileJsonID"
#Write-Host "lazOutfileId= $lazOutfileId"
# 
# #4 set
$inFileId =    $lazOutfileId
$outFolderId = $C_tmpOutputRoot + "/potree/space/droneRoute/" + $droneRouteId
# 
#Write-Host "inFileId= $inFileId"
#Write-Host "outFolderId= $outFolderId"
#  
###########################
# #1: makeDroneRouteVoxcel
###########################
$execPath = $C_execPathArr["makeDroneRouteVoxel"]["appPath"]

if (Test-Path $execPath) {
    Write-Host "execPath= $execPath"
} else {
    Write-Host "execPath not found= $execPath"
    exit 1
}
##########
# $proc
##########
$proc = Start-Process -FilePath $execPath -ArgumentList ($droneRouteInfoId+" "+$configPath) -NoNewWindow -PassThru -Wait -WorkingDirectory $C_PSScriptRoot

$retCd = $proc.ExitCode
if ($retCd -gt 4) {
    putMsg("#1 makeDroneRouteVoxcel RC="+$retCd) " error"
    exit $retCd
}
exit 0
