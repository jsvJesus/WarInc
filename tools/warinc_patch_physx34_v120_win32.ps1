$ErrorActionPreference = "Stop"

$Root = (Resolve-Path ".").Path

$Projects = @(
    "src\EclipseStudio\Studio.vcxproj",
    "src\Eternity\Eternity.vcxproj",
    "src\GameEngine\GameEngine.vcxproj"
)

$PhysXIncludes = @(
    ".\..\External\PhysX_3.4\Include",
    ".\..\External\PhysX_3.4\Include\common",
    ".\..\External\PhysX_3.4\Include\geometry",
    ".\..\External\PhysX_3.4\Include\extensions",
    ".\..\External\PhysX_3.4\Include\characterkinematic",
    ".\..\External\PhysX_3.4\Include\cooking",
    ".\..\External\PhysX_3.4\Include\pvd",
    ".\..\External\PxShared\include",
    ".\..\External\PxShared\include\foundation",
    ".\..\External\PxShared\include\pvd",
    ".\..\External\PxShared\include\task"
)

$PhysXLibDirs = @(
    ".\..\External\PhysX_3.4\Lib\vc12win32",
    ".\..\External\PhysX_3.4\bin\vc12win32",
    ".\..\External\PxShared\lib\vc12win32"
)

$DebugPhysXLibs = @(
    "PhysX3DEBUG_x86.lib",
    "PhysX3CommonDEBUG_x86.lib",
    "PhysX3CookingDEBUG_x86.lib",
    "PhysX3CharacterKinematicDEBUG_x86.lib",
    "PhysX3ExtensionsDEBUG.lib",
    "PxFoundationDEBUG_x86.lib",
    "PxPvdSDKDEBUG_x86.lib",
    "PsFastXmlDEBUG_x86.lib",
    "PxTaskDEBUG_x86.lib"
)

$ReleasePhysXLibs = @(
    "PhysX3_x86.lib",
    "PhysX3Common_x86.lib",
    "PhysX3Cooking_x86.lib",
    "PhysX3CharacterKinematic_x86.lib",
    "PhysX3Extensions.lib",
    "PxFoundation_x86.lib",
    "PxPvdSDK_x86.lib",
    "PsFastXml_x86.lib",
    "PxTask_x86.lib"
)

function Backup-File {
    param([string]$Path)

    $Backup = "$Path.bak"
    if (!(Test-Path $Backup)) {
        Copy-Item $Path $Backup
    }
}

function Normalize-List {
    param([string[]]$Items)

    $Out = New-Object System.Collections.Generic.List[string]

    foreach ($Item in $Items) {
        $Clean = $Item.Trim()
        if ($Clean.Length -eq 0) {
            continue
        }

        $Exists = $false
        foreach ($Old in $Out) {
            if ($Old -ieq $Clean) {
                $Exists = $true
                break
            }
        }

        if (!$Exists) {
            $Out.Add($Clean)
        }
    }

    return $Out
}

function Patch-Toolset {
    param([string]$Text)

    $Text = $Text -replace "<PlatformToolset>v143</PlatformToolset>", "<PlatformToolset>v120</PlatformToolset>"
    $Text = $Text -replace "<ToolsVersion>Current</ToolsVersion>", "<ToolsVersion>12.0</ToolsVersion>"
    $Text = $Text -replace 'ToolsVersion="Current"', 'ToolsVersion="12.0"'

    return $Text
}

function Patch-Definitions {
    param([string]$Text)

    return [regex]::Replace(
        $Text,
        "<PreprocessorDefinitions>(.*?)</PreprocessorDefinitions>",
        {
            param($m)

            $Items = $m.Groups[1].Value -split ";"
            $Final = @(
                "WARINC_PHYSX_34=1",
                "APEX_ENABLED=0",
                "VEHICLES_ENABLED=0"
            )

            foreach ($Item in $Items) {
                $Clean = $Item.Trim()

                if ($Clean -match "^WARINC_PHYSX_34") {
                    continue
                }

                if ($Clean -match "^APEX_ENABLED") {
                    continue
                }

                if ($Clean -match "^VEHICLES_ENABLED") {
                    continue
                }

                if ($Clean.Length -gt 0) {
                    $Final += $Clean
                }
            }

            $Final = Normalize-List $Final

            "<PreprocessorDefinitions>" + ($Final -join ";") + "</PreprocessorDefinitions>"
        },
        [System.Text.RegularExpressions.RegexOptions]::Singleline
    )
}

function Patch-Includes {
    param([string]$Text)

    return [regex]::Replace(
        $Text,
        "<AdditionalIncludeDirectories>(.*?)</AdditionalIncludeDirectories>",
        {
            param($m)

            $Items = $m.Groups[1].Value -split ";"
            $Filtered = @()

            foreach ($Item in $Items) {
                $Clean = $Item.Trim()

                if ($Clean -match "External\\PhysX\\" -and $Clean -notmatch "External\\PhysX_3\.4") {
                    continue
                }

                if ($Clean -match "External/PhysX/" -and $Clean -notmatch "External/PhysX_3\.4") {
                    continue
                }

                if ($Clean.Length -gt 0) {
                    $Filtered += $Clean
                }
            }

            $Final = @()
            $Final += $PhysXIncludes
            $Final += $Filtered

            $Final = Normalize-List $Final

            "<AdditionalIncludeDirectories>" + ($Final -join ";") + "</AdditionalIncludeDirectories>"
        },
        [System.Text.RegularExpressions.RegexOptions]::Singleline
    )
}

function Patch-LinkDirs {
    param([string]$Text)

    return [regex]::Replace(
        $Text,
        "<AdditionalLibraryDirectories>(.*?)</AdditionalLibraryDirectories>",
        {
            param($m)

            $Items = $m.Groups[1].Value -split ";"
            $Filtered = @()

            foreach ($Item in $Items) {
                $Clean = $Item.Trim()

                if ($Clean -match "External\\PhysX\\lib\\Win32") {
                    continue
                }

                if ($Clean -match "External/PhysX/lib/Win32") {
                    continue
                }

                if ($Clean.Length -gt 0) {
                    $Filtered += $Clean
                }
            }

            $Final = @()
            $Final += $PhysXLibDirs
            $Final += $Filtered

            $Final = Normalize-List $Final

            "<AdditionalLibraryDirectories>" + ($Final -join ";") + "</AdditionalLibraryDirectories>"
        },
        [System.Text.RegularExpressions.RegexOptions]::Singleline
    )
}

function Patch-StudioPhysXLibs {
    param([string]$Text)

    return [regex]::Replace(
        $Text,
        "<AdditionalDependencies>(.*?)</AdditionalDependencies>",
        {
            param($m)

            $Items = $m.Groups[1].Value -split ";"
            $Filtered = @()

            foreach ($Item in $Items) {
                $Clean = $Item.Trim()

                if ($Clean -match "^PhysX") {
                    continue
                }

                if ($Clean -match "^PxFoundation") {
                    continue
                }

                if ($Clean -match "^PxPvdSDK") {
                    continue
                }

                if ($Clean -match "^PxTask") {
                    continue
                }

                if ($Clean -match "^PsFastXml") {
                    continue
                }

                if ($Clean.Length -gt 0) {
                    $Filtered += $Clean
                }
            }

            $ConfigText = $Text.Substring(0, $m.Index)
            $UseDebug = $false

            $LastItemDefinition = $ConfigText.LastIndexOf("<ItemDefinitionGroup")
            if ($LastItemDefinition -ge 0) {
                $Header = $ConfigText.Substring($LastItemDefinition, [Math]::Min(350, $ConfigText.Length - $LastItemDefinition))
                if ($Header -match "Debug\|Win32") {
                    $UseDebug = $true
                }
            }

            $Final = @()

            if ($UseDebug) {
                $Final += $DebugPhysXLibs
            } else {
                $Final += $ReleasePhysXLibs
            }

            $Final += $Filtered
            $Final = Normalize-List $Final

            "<AdditionalDependencies>" + ($Final -join ";") + "</AdditionalDependencies>"
        },
        [System.Text.RegularExpressions.RegexOptions]::Singleline
    )
}

function Remove-OldPhysXSourceFiles {
    param([string]$Text)

    $Text = [regex]::Replace(
        $Text,
        "\s*<ClCompile Include=""\.\.\\External\\PhysX\\[^""]+"">.*?</ClCompile>",
        "",
        [System.Text.RegularExpressions.RegexOptions]::Singleline
    )

    $Text = [regex]::Replace(
        $Text,
        "\s*<ClCompile Include=""\.\.\\External\\PhysX\\[^""]+""\s*/>",
        "",
        [System.Text.RegularExpressions.RegexOptions]::Singleline
    )

    return $Text
}

foreach ($ProjectRelative in $Projects) {
    $ProjectPath = Join-Path $Root $ProjectRelative

    if (!(Test-Path $ProjectPath)) {
        Write-Host "NOT FOUND: $ProjectRelative"
        continue
    }

    Backup-File $ProjectPath

    $Text = Get-Content $ProjectPath -Raw

    $Text = Patch-Toolset $Text
    $Text = Patch-Definitions $Text
    $Text = Patch-Includes $Text
    $Text = Patch-LinkDirs $Text

    if ($ProjectRelative -like "*Studio.vcxproj") {
        $Text = Patch-StudioPhysXLibs $Text
    }

    if ($ProjectRelative -like "*GameEngine.vcxproj") {
        $Text = Remove-OldPhysXSourceFiles $Text
    }

    Set-Content -Path $ProjectPath -Value $Text -NoNewline

    Write-Host "patched: $ProjectRelative"
}

Write-Host "DONE"