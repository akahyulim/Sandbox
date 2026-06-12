param (
    [string]$ShaderFile,
    [string]$Stage = "ps_5_0",
    [string]$Entry = "MainPS"
)

$fxc = "C:\Program Files (x86)\Windows Kits\10\bin\x64\fxc.exe"

if (-not (Test-Path $fxc)) {
    Write-Host "❌ FXC not found at: $fxc"
    exit 1
}

if (-not (Test-Path $ShaderFile)) {
    Write-Host "❌ Shader file not found: $ShaderFile"
    exit 1
}

$baseName = [System.IO.Path]::GetFileNameWithoutExtension($ShaderFile)
$dir = [System.IO.Path]::GetDirectoryName($ShaderFile)
$outFile = "$dir\$baseName.cso"

Write-Host "🔧 Compiling $ShaderFile"
Write-Host "   → Stage: $Stage"
Write-Host "   → Entry: $Entry"
Write-Host "   → Output: $outFile"

& "$fxc" /T $Stage /E $Entry /Fo $outFile $ShaderFile

if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Compilation succeeded: $outFile"
} else {
    Write-Host "❌ Compilation failed with exit code $LASTEXITCODE"
}
