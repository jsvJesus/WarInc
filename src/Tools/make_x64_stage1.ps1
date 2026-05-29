$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$SrcRoot = (Resolve-Path (Join-Path $ScriptDir "..")).Path

$SlnPath = Join-Path $SrcRoot "EclipseStudio\WarInc.sln"

$Projects = @(
    (Join-Path $SrcRoot "EclipseStudio\Studio.vcxproj"),
    (Join-Path $SrcRoot "GameEngine\GameEngine.vcxproj"),
    (Join-Path $SrcRoot "Eternity\Eternity.vcxproj"),
    (Join-Path $SrcRoot "External\RakNet\RakNet.vcxproj")
)

function Save-Utf8NoBom {
    param(
        [string]$Path,
        [string]$Text
    )

    $Encoding = New-Object System.Text.UTF8Encoding $false
    [System.IO.File]::WriteAllText($Path, $Text, $Encoding)
}

function Add-SlnLine {
    param(
        [string[]]$Lines,
        [string]$SectionName,
        [string]$Line
    )

    if ($Lines -contains $Line) {
        return ,$Lines
    }

    $Start = -1
    for ($i = 0; $i -lt $Lines.Count; $i++) {
        if ($Lines[$i] -like "*GlobalSection($SectionName)*") {
            $Start = $i
            break
        }
    }

    if ($Start -lt 0) {
        throw "Section not found: $SectionName"
    }

    $End = -1
    for ($i = $Start + 1; $i -lt $Lines.Count; $i++) {
        if ($Lines[$i].Trim() -eq "EndGlobalSection") {
            $End = $i
            break
        }
    }

    if ($End -lt 0) {
        throw "EndGlobalSection not found: $SectionName"
    }

    $Before = @()
    $After = @()

    if ($End -gt 0) {
        $Before = $Lines[0..($End - 1)]
    }

    $After = $Lines[$End..($Lines.Count - 1)]

    return ,@($Before + $Line + $After)
}

function Patch-Solution {
    param([string]$Path)

    $Lines = [System.IO.File]::ReadAllLines($Path)

    $Lines = Add-SlnLine $Lines "SolutionConfigurationPlatforms" "`t`tDebug|x64 = Debug|x64"
    $Lines = Add-SlnLine $Lines "SolutionConfigurationPlatforms" "`t`tFinal|x64 = Final|x64"
    $Lines = Add-SlnLine $Lines "SolutionConfigurationPlatforms" "`t`tRelease|x64 = Release|x64"

    $Eternity = "{3D51BE84-6441-4AC7-A73F-587C6B36DDAF}"
    $Studio = "{22AD274B-9E09-431E-94D7-41780F91B239}"
    $GameEngine = "{D14FACF2-B61D-445F-A307-FDC587F901F4}"
    $RakNet = "{143F9B67-C223-4F2D-B8F9-B644CA710CA6}"

    $Map = @(
        "`t`t$Eternity.Debug|x64.ActiveCfg = Debug_NoCrashRpt|x64",
        "`t`t$Eternity.Debug|x64.Build.0 = Debug_NoCrashRpt|x64",
        "`t`t$Eternity.Final|x64.ActiveCfg = Final|x64",
        "`t`t$Eternity.Final|x64.Build.0 = Final|x64",
        "`t`t$Eternity.Release|x64.ActiveCfg = Release|x64",
        "`t`t$Eternity.Release|x64.Build.0 = Release|x64",

        "`t`t$Studio.Debug|x64.ActiveCfg = Debug|x64",
        "`t`t$Studio.Debug|x64.Build.0 = Debug|x64",
        "`t`t$Studio.Final|x64.ActiveCfg = Final|x64",
        "`t`t$Studio.Final|x64.Build.0 = Final|x64",
        "`t`t$Studio.Release|x64.ActiveCfg = Release|x64",
        "`t`t$Studio.Release|x64.Build.0 = Release|x64",

        "`t`t$GameEngine.Debug|x64.ActiveCfg = Debug|x64",
        "`t`t$GameEngine.Debug|x64.Build.0 = Debug|x64",
        "`t`t$GameEngine.Final|x64.ActiveCfg = Final|x64",
        "`t`t$GameEngine.Final|x64.Build.0 = Final|x64",
        "`t`t$GameEngine.Release|x64.ActiveCfg = Release|x64",
        "`t`t$GameEngine.Release|x64.Build.0 = Release|x64",

        "`t`t$RakNet.Debug|x64.ActiveCfg = Debug|x64",
        "`t`t$RakNet.Debug|x64.Build.0 = Debug|x64",
        "`t`t$RakNet.Final|x64.ActiveCfg = Release|x64",
        "`t`t$RakNet.Final|x64.Build.0 = Release|x64",
        "`t`t$RakNet.Release|x64.ActiveCfg = Release|x64",
        "`t`t$RakNet.Release|x64.Build.0 = Release|x64"
    )

    foreach ($Line in $Map) {
        $Lines = Add-SlnLine $Lines "ProjectConfigurationPlatforms" $Line
    }

    Save-Utf8NoBom $Path ($Lines -join "`r`n")
}

function Has-Attr {
    param(
        [System.Xml.XmlNode]$Node,
        [string]$Name
    )

    if ($Node.NodeType -ne [System.Xml.XmlNodeType]::Element) {
        return $false
    }

    return $Node.Attributes[$Name] -ne $null
}

function Get-Attr {
    param(
        [System.Xml.XmlNode]$Node,
        [string]$Name
    )

    if (-not (Has-Attr $Node $Name)) {
        return ""
    }

    return $Node.Attributes[$Name].Value
}

function Set-Condition-X64 {
    param([System.Xml.XmlNode]$Node)

    if (Has-Attr $Node "Condition") {
        $Cond = Get-Attr $Node "Condition"
        $Node.Attributes["Condition"].Value = $Cond.Replace("|Win32", "|x64")
    }

    foreach ($Child in @($Node.ChildNodes)) {
        if ($Child.NodeType -eq [System.Xml.XmlNodeType]::Element) {
            Set-Condition-X64 $Child
        }
    }
}

function Add-Define {
    param(
        [string]$Text,
        [string]$Define
    )

    $Escaped = [regex]::Escape($Define)

    if ($Text -match "(^|;)$Escaped(;|$)") {
        return $Text
    }

    if ($Text.Contains("%(PreprocessorDefinitions)")) {
        return $Text.Replace("%(PreprocessorDefinitions)", "$Define;%(PreprocessorDefinitions)")
    }

    if ([string]::IsNullOrWhiteSpace($Text)) {
        return $Define
    }

    return "$Text;$Define"
}

function Fix-X64-Text {
    param(
        [string]$Name,
        [string]$Text
    )

    if ($Name -eq "PreprocessorDefinitions") {
        $Text = Add-Define $Text "WIN64"
        $Text = Add-Define $Text "_WIN64"
        $Text = Add-Define $Text "WARINC_X64=1"
        return $Text
    }

    if ($Name -eq "AdditionalOptions") {
        $Text = $Text.Replace("/MACHINE:I386", "/MACHINE:X64")
        $Text = $Text.Replace("/machine:I386", "/MACHINE:X64")
        $Text = $Text.Replace("/SAFESEH", "")
        $Text = $Text.Replace("/SAFESEH:NO", "")
        return $Text
    }

    if ($Name -eq "AdditionalDependencies") {
        $Text = $Text.Replace("DEBUG_x86.lib", "DEBUG_x64.lib")
        $Text = $Text.Replace("_x86.lib", "_x64.lib")
        $Text = $Text.Replace("fmodex_vc.lib", "fmodex64_vc.lib")
        $Text = $Text.Replace("fmod_event_net.lib", "fmod_event_net64.lib")
        $Text = $Text.Replace("fmod_event.lib", "fmod_event64.lib")
        $Text = $Text.Replace("ChilkatRel.lib", "ChilkatRel_x64.lib")
        $Text = $Text.Replace("ChilkatDbg.lib", "ChilkatDbg_x64.lib")
        return $Text
    }

    if ($Name -eq "AdditionalLibraryDirectories") {
        $Text = $Text.Replace("vc12win32", "vc12win64")
        $Text = $Text.Replace("\Win32\Msvc10", "\x64\Msvc10")
        $Text = $Text.Replace("/Win32/Msvc10", "/x64/Msvc10")
        $Text = $Text.Replace("\Win32\", "\x64\")
        $Text = $Text.Replace("/Win32/", "/x64/")
        $Text = [regex]::Replace($Text, "(?i)(dxsdk[\\/])lib(?![\\/])", '${1}Lib\x64')
        $Text = [regex]::Replace($Text, "(?i)(dxsdk[\\/]Lib)(?![\\/])", '${1}\x64')
        $Text = [regex]::Replace($Text, "(?i)(Chilkat[\\/]Libs)(?![\\/])", '${1}\x64')
        $Text = $Text.Replace(";;", ";")
        return $Text
    }

    if ($Name -eq "TargetEnvironment") {
        return "X64"
    }

    if ($Name -eq "DebugInformationFormat") {
        if ($Text -eq "EditAndContinue") {
            return "ProgramDatabase"
        }
    }

    if ($Name -eq "IntDir") {
        if (-not $Text.Contains('$(Platform)')) {
            $Text = $Text.Replace('$(Configuration)\', '$(Configuration)_$(Platform)\')
            $Text = $Text.Replace('$(Configuration)/', '$(Configuration)_$(Platform)/')
        }

        return $Text
    }

    if ($Name -eq "OutputFile") {
        if ($Text -match "\.exe$" -and $Text -notmatch "_x64\.exe$") {
            return ($Text -replace "\.exe$", "_x64.exe")
        }

        return $Text
    }

    return $Text
}

function Fix-X64-Subtree {
    param([System.Xml.XmlNode]$Node)

    if ($Node.NodeType -ne [System.Xml.XmlNodeType]::Element) {
        return
    }

    $RemoveNames = @(
        "EnableEnhancedInstructionSet",
        "MinimalRebuild",
        "OmitFramePointers"
    )

    if ($RemoveNames -contains $Node.LocalName) {
        [void]$Node.ParentNode.RemoveChild($Node)
        return
    }

    if ($Node.ChildNodes.Count -eq 1 -and $Node.FirstChild.NodeType -eq [System.Xml.XmlNodeType]::Text) {
        $Node.InnerText = Fix-X64-Text $Node.LocalName $Node.InnerText
    }

    foreach ($Child in @($Node.ChildNodes)) {
        Fix-X64-Subtree $Child
    }
}

function Has-Conditioned-Ancestor {
    param([System.Xml.XmlNode]$Node)

    $Parent = $Node.ParentNode

    while ($Parent -ne $null -and $Parent.LocalName -ne "Project") {
        if (Has-Attr $Parent "Condition") {
            $Cond = Get-Attr $Parent "Condition"
            if ($Cond.Contains("|Win32") -or $Cond.Contains("|x64")) {
                return $true
            }
        }

        $Parent = $Parent.ParentNode
    }

    return $false
}

function Patch-Vcxproj {
    param([string]$Path)

    [xml]$Doc = New-Object System.Xml.XmlDocument
    $Doc.PreserveWhitespace = $true
    $Doc.Load($Path)

    $Project = $Doc.DocumentElement

    $ProjectConfigGroup = $null

    foreach ($Node in @($Project.ChildNodes)) {
        if ($Node.NodeType -eq [System.Xml.XmlNodeType]::Element -and $Node.LocalName -eq "ItemGroup" -and (Get-Attr $Node "Label") -eq "ProjectConfigurations") {
            $ProjectConfigGroup = $Node
            break
        }
    }

    if ($ProjectConfigGroup -eq $null) {
        throw "ProjectConfigurations not found: $Path"
    }

    $ExistingConfigs = @{}

    foreach ($Node in @($ProjectConfigGroup.ChildNodes)) {
        if ($Node.NodeType -eq [System.Xml.XmlNodeType]::Element -and $Node.LocalName -eq "ProjectConfiguration") {
            $ExistingConfigs[(Get-Attr $Node "Include")] = $true
        }
    }

    $Win32Configs = @()

    foreach ($Node in @($ProjectConfigGroup.ChildNodes)) {
        if ($Node.NodeType -eq [System.Xml.XmlNodeType]::Element -and $Node.LocalName -eq "ProjectConfiguration") {
            $Include = Get-Attr $Node "Include"
            if ($Include.EndsWith("|Win32")) {
                $Win32Configs += $Node
            }
        }
    }

    foreach ($Config in $Win32Configs) {
        $OldInclude = Get-Attr $Config "Include"
        $NewInclude = $OldInclude.Replace("|Win32", "|x64")

        if (-not $ExistingConfigs.ContainsKey($NewInclude)) {
            $Clone = $Config.CloneNode($true)
            $Clone.Attributes["Include"].Value = $NewInclude

            foreach ($Child in @($Clone.ChildNodes)) {
                if ($Child.NodeType -eq [System.Xml.XmlNodeType]::Element -and $Child.LocalName -eq "Platform") {
                    $Child.InnerText = "x64"
                }
            }

            [void]$ProjectConfigGroup.AppendChild($Clone)
            $ExistingConfigs[$NewInclude] = $true
        }
    }

    $TopLevelWin32 = @()

    foreach ($Node in @($Project.ChildNodes)) {
        if ($Node.NodeType -eq [System.Xml.XmlNodeType]::Element -and (Has-Attr $Node "Condition")) {
            $Cond = Get-Attr $Node "Condition"

            if ($Cond.Contains("|Win32")) {
                $TopLevelWin32 += $Node
            }
        }
    }

    foreach ($Node in $TopLevelWin32) {
        $OldCond = Get-Attr $Node "Condition"
        $NewCond = $OldCond.Replace("|Win32", "|x64")

        $Exists = $false

        foreach ($Other in @($Project.ChildNodes)) {
            if ($Other.NodeType -eq [System.Xml.XmlNodeType]::Element -and $Other.LocalName -eq $Node.LocalName -and (Get-Attr $Other "Condition") -eq $NewCond) {
                $Exists = $true
                break
            }
        }

        if (-not $Exists) {
            $Clone = $Node.CloneNode($true)
            Set-Condition-X64 $Clone
            [void]$Project.InsertAfter($Clone, $Node)
        }
    }

    $ConditionalNodes = @()

    foreach ($Node in @($Doc.GetElementsByTagName("*"))) {
        if ($Node.NodeType -eq [System.Xml.XmlNodeType]::Element -and (Has-Attr $Node "Condition")) {
            $Cond = Get-Attr $Node "Condition"

            if ($Cond.Contains("|Win32") -and -not (Has-Conditioned-Ancestor $Node)) {
                $ConditionalNodes += $Node
            }
        }
    }

    foreach ($Node in $ConditionalNodes) {
        $OldCond = Get-Attr $Node "Condition"
        $NewCond = $OldCond.Replace("|Win32", "|x64")

        $Exists = $false

        foreach ($Other in @($Node.ParentNode.ChildNodes)) {
            if ($Other.NodeType -eq [System.Xml.XmlNodeType]::Element -and $Other.LocalName -eq $Node.LocalName -and (Get-Attr $Other "Condition") -eq $NewCond) {
                $Exists = $true
                break
            }
        }

        if (-not $Exists) {
            $Clone = $Node.CloneNode($true)
            Set-Condition-X64 $Clone
            [void]$Node.ParentNode.InsertAfter($Clone, $Node)
        }
    }

    foreach ($Node in @($Doc.GetElementsByTagName("*"))) {
        if ($Node.NodeType -eq [System.Xml.XmlNodeType]::Element -and (Has-Attr $Node "Condition")) {
            $Cond = Get-Attr $Node "Condition"

            if ($Cond.Contains("|x64")) {
                Fix-X64-Subtree $Node
            }
        }
    }

    foreach ($Node in @($ProjectConfigGroup.ChildNodes)) {
        if ($Node.NodeType -eq [System.Xml.XmlNodeType]::Element -and $Node.LocalName -eq "ProjectConfiguration") {
            $Include = Get-Attr $Node "Include"

            if ($Include.EndsWith("|x64")) {
                foreach ($Child in @($Node.ChildNodes)) {
                    if ($Child.NodeType -eq [System.Xml.XmlNodeType]::Element -and $Child.LocalName -eq "Platform") {
                        $Child.InnerText = "x64"
                    }
                }
            }
        }
    }

    $Settings = New-Object System.Xml.XmlWriterSettings
    $Settings.Encoding = New-Object System.Text.UTF8Encoding $false
    $Settings.Indent = $false
    $Settings.NewLineChars = "`r`n"

    $Writer = [System.Xml.XmlWriter]::Create($Path, $Settings)
    $Doc.Save($Writer)
    $Writer.Close()
}

Patch-Solution $SlnPath

foreach ($Project in $Projects) {
    Patch-Vcxproj $Project
}

Write-Host "x64 stage 1 complete."
Write-Host "Patched:"
Write-Host "  $SlnPath"

foreach ($Project in $Projects) {
    Write-Host "  $Project"
}