# UTF-8 BOM付でないと文字化けが発生することがあるので注意
# 初回　Set-ExecutionPolicy RemoteSigned を　powershell　Terminal（管理者として実行）から実行しておく必要がある

# data/work配下のオーナー別ディレクトリに結果を出力する
function execMergeSpatialLasFromPmResult {
    Param (
        $commonConfig,
        $configPath,
        [string]$owId
    )
    
    $pointMatchOutputRoot = getPointMatchOutputRoot $commonConfig
    $pointMatchOutputFolder = getPointMatchOutputForder $commonConfig
    
    try {
        $files = Get-ChildItem -Path $pointMatchOutputRoot -File -Recurse
    }
    catch {
        putLog ("ファイル一覧取得で、エラーが発生しました。"+$pointMatchOutputRoot) "error"
        return 9
    }
    $outputJson = @{}
    $outputJson['param'] = @{}
    $outputJson['exec'] = @()
    $inputs = @()
    $pointPathOld = ""
    foreach ($file in $files) {
        $fileName = $file.FullName
        $folderId = "\"+$pointMatchOutputFolder.Replace("/", "\")+"\"
        if ($fileName.Contains($folderId)) {
            $pointPath,$featureLas = $file.FullName.split("_")
            if (!($featureLas -eq "ALL.las")) {
                if (!($pointPathOld -eq $pointPath) -and !($pointPathOld -eq "")) {
                    $exec = @{}
                    $exec['input'] = $inputs
                    $exec['output'] = $pointPathOld + "_ALL.las"
                    $outputJson['exec'] += $exec
                    $inputs = @()
                }
                $inputs += $file.FullName.Replace("\", "/")
                $pointPathOld = $pointPath
            }
        }
    }
    if (!($pointPathOld -eq "")) {
        $exec = @{}
        $exec['input'] = $inputs
        $exec['output'] = $pointPathOld + "_ALL.las"
        $outputJson['exec'] += $exec
    }
    $outputFile = $pointMatchOutputRoot+ "/outputLasMerge.json"
    try {
        ConvertTo-Json -InputObject $outputJson -Depth 5 | Out-File -LiteralPath $outputFile -Encoding utf8 -ErrorAction Stop
    }
    catch {
        putLog ("ファイル出力で、エラーが発生しました。"+$outputFile) "error"
        return 9
    }
    $execPath = $C_execPathArr["lasMerge"]["appPath"]
    $proc = Start-Process -FilePath $execPath -ArgumentList ($outputFile+" --config="+$configPath) -NoNewWindow -PassThru -Wait -WorkingDirectory $C_PSScriptRoot
    if ($proc.ExitCode -ne 0) {
        return $proc.ExitCode
    }
    return 0
    
}