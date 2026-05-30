param(
    [string]$Root = ".",
    [string]$BaseUrl = "http://localhost:5000",
    [switch]$VerboseBody
)

$ErrorActionPreference = "Stop"

$endpointSet = New-Object "System.Collections.Generic.HashSet[string]" ([System.StringComparer]::OrdinalIgnoreCase)

$known = @(
    "api_BuyItem3.aspx",
    "api_ChangeGamerTag.aspx",
    "api_ChangeGamerTag2.aspx",
    "api_CheckLoginSession.aspx",
    "api_ClientUpdateAchievements.aspx",
    "api_ClanApply.aspx",
    "api_ClanCreate.aspx",
    "api_ClanGetInfo.aspx",
    "api_ClanInvites.aspx",
    "api_ClanMgr.aspx",
    "api_Friends.aspx",
    "api_Gamersfirst.aspx",
    "api_GetCreateGameKey3.aspx",
    "api_GetDataGameRewards.aspx",
    "api_GetItemsInfo.aspx",
    "api_GetProfile4.aspx",
    "api_GetShop5.aspx",
    "api_GNAGetBalance.aspx",
    "api_GNALogin.aspx",
    "api_LeaderboardGet.aspx",
    "api_LoadoutModify.aspx",
    "api_LoadoutReset.aspx",
    "api_LoadoutUnlock.aspx",
    "api_Login.aspx",
    "api_LoginSessionPoller.aspx",
    "api_ModifySlot.aspx",
    "api_MysteryBox.aspx",
    "api_ReportHWInfo.aspx",
    "api_ReportHWInfo_Customer.aspx",
    "api_RetBonus.aspx",
    "api_SetRSUpdateStatus.aspx",
    "api_SkillLearn.aspx",
    "api_SkillReset.aspx",
    "api_SrvAddCheatAttempts.aspx",
    "api_SrvAddLogInfo.aspx",
    "api_SrvAddUserRoundResult4.aspx",
    "api_SrvAddWeaponStats.aspx",
    "api_SrvGiveItem.aspx",
    "api_SrvGiveItemInMinutes.aspx",
    "api_SrvRemoveItem.aspx",
    "api_SrvSetCreateGameKey2.aspx",
    "api_SrvUpdateAchievements.aspx",
    "api_SrvUploadLogFile.aspx",
    "api_SteamBuyGP.aspx",
    "api_SteamCreateAcc.aspx",
    "api_SteamLogin.aspx",
    "api_UpdateLoginSession.aspx",
    "api_WeaponAttach.aspx",
    "api_WeaponAttachSet.aspx",
    "api_WelcomePackage4.aspx"
)

foreach ($item in $known) {
    [void]$endpointSet.Add($item)
}

$files = Get-ChildItem -Path $Root -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object {
        $_.FullName -notmatch "\\\.git\\" -and
        $_.Extension -in ".cpp", ".h", ".hpp", ".c", ".cs", ".aspx", ".config", ".xml", ".txt", ".php"
    }

$regex = [regex]'api_[A-Za-z0-9_]+\.aspx'

foreach ($file in $files) {
    $text = Get-Content -LiteralPath $file.FullName -Raw -ErrorAction SilentlyContinue

    if ([string]::IsNullOrWhiteSpace($text)) {
        continue
    }

    foreach ($match in $regex.Matches($text)) {
        [void]$endpointSet.Add($match.Value)
    }
}

$endpoints = $endpointSet.ToArray() | Sort-Object

Write-Host ""
Write-Host "WARINC LEGACY API SMOKE TEST"
Write-Host "Root:    $Root"
Write-Host "BaseUrl: $BaseUrl"
Write-Host "Found:   $($endpoints.Count) endpoints"
Write-Host ""

$failed = 0
$notFound = 0

foreach ($endpoint in $endpoints) {
    $url = "$BaseUrl/$endpoint"

    try {
        $response = Invoke-WebRequest `
            -Uri $url `
            -Method POST `
            -Body "s_id=1&s_key=1&func=smoke&LoadoutID=1&Class=0&BuyIdx=4&i1=0&i2=0&i3=0&i4=0&i5=0&i6=0&i7=0&i8=0&i9=0&i10=0&i11=0&i12=0&i13=0" `
            -ContentType "application/x-www-form-urlencoded" `
            -UseBasicParsing `
            -TimeoutSec 15

        $status = [int]$response.StatusCode
        $body = [string]$response.Content

        if ($status -eq 404) {
            $notFound++
            $failed++
            Write-Host "[404] $endpoint" -ForegroundColor Red
            continue
        }

        if ($status -ge 400) {
            $failed++
            Write-Host "[$status] $endpoint" -ForegroundColor Yellow
            if ($VerboseBody) {
                Write-Host $body
            }
            continue
        }

        Write-Host "[$status] $endpoint" -ForegroundColor Green

        if ($VerboseBody) {
            Write-Host $body
        }
    }
    catch {
        $failed++
        Write-Host "[ERR] $endpoint $($_.Exception.Message)" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "Done."
Write-Host "Failed: $failed"
Write-Host "404:    $notFound"

if ($notFound -gt 0) {
    exit 2
}

if ($failed -gt 0) {
    exit 1
}

exit 0