# UTF-8 BOM付でないと文字化けが発生することがあるので注意
# 初回　Set-ExecutionPolicy RemoteSigned を　powershell　Terminal（管理者として実行）から実行しておく必要がある

# data/work配下のオーナー別ディレクトリに結果を出力する
function execMakeMergeJsonFromPmResult {
    Param (
        $commonConfig,
        $configPath,
        [string]$owId,
        [string]$featureId
    )
    
    $pointMatchOutputRoot = getPointMatchOutputRoot $commonConfig
    $pointMatchOutputFolder = getPointMatchOutputForder $commonConfig
    $voxelOutputRoot = getvoxelOutputRoot $commonConfig
    $voxelOutputFolder = getVoxelOutputForder $commonConfig
    
    try {
        if ($featureId -eq "voxel") {
            $files = Get-ChildItem -Path $voxelOutputRoot -File -Recurse
        } else {
            $files = Get-ChildItem -Path $pointMatchOutputRoot -File -Recurse
        }
    }
    catch {
        putLog ("ファイル一覧取得で、エラーが発生しました。"+$pointMatchOutputRoot) "error"
        return 9
    }
    $outputJson = @{}
    $inputs = @()
    foreach ($file in $files) {
        $fileName = $file.FullName
        if ($featureId -eq "voxel") {
            # voxelを対象とする
            $folderId = "\"+$voxelOutputFolder.Replace("/", "\")+"\"
            if ($fileName.Contains($folderId)) {
                #putMsg("mergeLas : "+$file.FullName)
                $inputs += $file.FullName.Replace("\", "/")
            }
        } elseif (($featureId -eq "all") -Or ($fileName.Contains("_"+$featureId+".las"))) {
#        } elseif ($fileName.Contains("_"+$featureId+".las")) {
            # 地物を対象とする
            # ポイントマッチ用のフォルダのみ対象とする
            $folderId = "\"+$pointMatchOutputFolder.Replace("/", "\")+"\"
            if ($fileName.Contains($folderId)) {
                #putMsg("mergeLas : "+$file.FullName)
                $inputs += $file.FullName.Replace("\", "/")
            }
        }
    }
    $exec = @{}
    $exec['input'] = $inputs
    $exec['output'] = $C_tmpOutputRoot + "/mergedLas/"+$owId+"/3dTemp-" + $featureId + ".las"
    $outputJson['param'] = @{}
    $outputJson['exec'] = @()
    $outputJson['exec'] += $exec
    $outputFile = $C_tmpOutputRoot + "/json/"+$owId+"/outputLasFiles_"+$featureId+".json"
    try {
        New-Item -Path ($C_tmpOutputRoot + "/json/"+$owId) -ItemType Directory -Force > $null
        ConvertTo-Json -InputObject $outputJson -Depth 5 | Out-File -LiteralPath $outputFile -Encoding utf8 -ErrorAction Stop
    }
    catch {
        putLog ("ファイル出力で、エラーが発生しました。"+$outputFile) "error"
        return 9
    }
    return 0
    
}