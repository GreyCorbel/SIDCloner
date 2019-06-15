# SIDCloner
Demonstrates how to populate SID History on security principals migrated cross AD forest.

We routinely use this library in our migration projects as it can be easily integrated into PowerShell migration scripts and other toolset we develop and deliver to customers as part of migration projects.

PowerShell sample below is modified real-life script that migrated SID history for 10K users in one session. It demonstrates various methods provided by SIDCloner class. Format of input CSV file is shown in identities.csv - see source code

```Powershell
param( 
    [parameter(Mandatory = $false)] 
    [String]$inFile 
    ) 
         
$errorPreference='Continue' 
 
#constants 
$targetDomain="target.test.cz" 
 
$cwd=(Get-Item .).FullName 
[System.Reflection.Assembly]::LoadFile("$cwd\SIDCloner.dll") | Out-Null 
 
#process parameters 
#customize file/folder names 
if([String]::IsNullOrEmpty($inFile)) { $inFile = ".\Data\identities.csv" } 
 
#clear the log file 
if([System.IO.File]::Exists(".\Log\CloneFailed.csv")) { 
    Remove-Item -path ".\Log\CloneFailed.csv" 
} 
$data=import-csv $inFile 
 
## authenticate using implicit credentials 
$i=-1 
foreach($record in $data) { 
    try { 
        $i++ 
        $percentComplete=[System.Convert]::ToInt32($i/($data.count)*100) 
        Write-Progress -Activity "Cloning users" -PercentComplete $percentComplete -CurrentOperation "Processing user: $($record.sourceDomain)\$($record.sourceSAMAccountName)" -Status "Completed: $percentComplete`%" 
 
        #uses credentials of logged-on user (or credentials stored in Credentials Manager); works against PDC in both domains 
        [wintools.sidcloner]::CloneSid( 
            $record.sourceSAMAccountName, 
            $record.sourceDomain, 
            $record.targetSAMAccountName, 
            $targetDomain 
        ) 
 
        Write-Host "Account $($record.sourceDomain)\$($record.sourceSAMAccountName) cloned" 
    } 
    catch { 
        Write-Warning -message:"Account $($record.sourceDomain)\$($record.sourceSAMAccountName) failed to clone`n`tError:$($($_.Exception).Message)" 
        "$($record.sourceSAMAccountName),$($record.sourceDomain),$($record.targetSAMAccountName)" >> ".\Log\CloneFailed.csv" 
    } 
} 
 
#explicit DC and credentials for source domain 
$sourceCredential=Get-Credential 
$sourceDC=Get-DC -domain:$record.sourceDomain    #not implemented here 
$i=-1 
foreach($record in $data) { 
    try { 
        $i++ 
        $percentComplete=[System.Convert]::ToInt32($i/($data.count)*100) 
        Write-Progress -Activity "Cloning users" -PercentComplete $percentComplete -CurrentOperation "Processing user: $($record.sourceDomain)\$($record.sourceSAMAccountName)" -Status "Completed: $percentComplete`%" 
 
        [wintools.sidcloner]::CloneSid( 
            $record.sourceSAMAccountName, 
            $record.sourceDomain, 
            $sourceDC, 
            $sourceCredential.UserName, 
            $sourceCredential.Password, 
            $record.targetSAMAccountName, 
            $targetDomain 
        ) 
 
        Write-Host "Account $($record.sourceDomain)\$($record.sourceSAMAccountName) cloned" 
    } 
    catch { 
        Write-Warning -message:"Account $($record.sourceDomain)\$($record.sourceSAMAccountName) failed to clone`n`tError:$($($_.Exception).Message)" 
        "$($record.sourceSAMAccountName),$($record.sourceDomain),$($record.targetSAMAccountName)" >> ".\Log\CloneFailed.csv" 
    } 
} 
 
#explicit DC and credentials for both domains 
$targetCredential=Get-Credential 
$targetDC=Get-DC -domain:$targetDomain    #not implemented here 
$i=-1 
foreach($record in $data) { 
    try { 
        $i++ 
        $percentComplete=[System.Convert]::ToInt32($i/($data.count)*100) 
        Write-Progress -Activity "Cloning users" -PercentComplete $percentComplete -CurrentOperation "Processing user: $($record.sourceDomain)\$($record.sourceSAMAccountName)" -Status "Completed: $percentComplete`%" 
 
        [wintools.sidcloner]::CloneSid( 
            $record.sourceSAMAccountName, 
            $record.sourceDomain, 
            $sourceDC, 
            $sourceCredential.UserName, 
            $sourceCredential.Password, 
            $record.targetSAMAccountName, 
            $targetDomain, 
            $targetDC, 
            $targetCredential.UserName, 
            $targetCredential.Password 
        ) 
 
        Write-Host "Account $($record.sourceDomain)\$($record.sourceSAMAccountName) cloned" 
    } 
    catch { 
        Write-Warning -message:"Account $($record.sourceDomain)\$($record.sourceSAMAccountName) failed to clone`n`tError:$($($_.Exception).Message)" 
        "$($record.sourceSAMAccountName),$($record.sourceDomain),$($record.targetSAMAccountName)" >> ".\Log\CloneFailed.csv" 
    } 
} 
 
```
