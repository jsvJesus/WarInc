$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot

$projects = @(
    "src\Eternity\Eternity.vcxproj",
    "src\GameEngine\GameEngine.vcxproj",
    "src\EclipseStudio\Studio.vcxproj",
    "src\External\RakNet\RakNet.vcxproj"
)

function Save-XmlUtf8NoBom {
    param(
        [xml]$Xml,
        [string]$Path
    )

    $settings = New-Object System.Xml.XmlWriterSettings
    $settings.Encoding = New-Object System.Text.UTF8Encoding($false)
    $settings.Indent = $true
    $settings.NewLineChars = "`r`n"

    $writer = [System.Xml.XmlWriter]::Create($Path, $settings)
    $Xml.Save($writer)
    $writer.Close()
}

foreach ($relativePath in $projects) {
    $path = Join-Path $root $relativePath

    if (!(Test-Path $path)) {
        Write-Host "SKIP: $relativePath"
        continue
    }

    Copy-Item $path "$path.bak" -Force

    [xml]$xml = Get-Content $path

    $ns = New-Object System.Xml.XmlNamespaceManager($xml.NameTable)
    $ns.AddNamespace("msb", "http://schemas.microsoft.com/developer/msbuild/2003")

    $configGroups = $xml.SelectNodes("//msb:PropertyGroup[contains(@Condition, '|x64') and @Label='Configuration']", $ns)

    foreach ($group in $configGroups) {
        $toolset = $group.SelectSingleNode("msb:PlatformToolset", $ns)

        if ($toolset -eq $null) {
            $toolset = $xml.CreateElement("PlatformToolset", "http://schemas.microsoft.com/developer/msbuild/2003")
            [void]$group.AppendChild($toolset)
        }

        $toolset.InnerText = "v120"
    }

    $languageNodes = @($xml.SelectNodes("//msb:LanguageStandard", $ns))
    foreach ($node in $languageNodes) {
        [void]$node.ParentNode.RemoveChild($node)
    }

    Save-XmlUtf8NoBom $xml $path

    Write-Host "OK: $relativePath -> PlatformToolset v120, removed LanguageStandard"
}

Write-Host ""
Write-Host "DONE. Backups created as *.bak"