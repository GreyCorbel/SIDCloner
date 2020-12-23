# SIDCloner
Demonstrates how to populate SID History on security principals migrated cross AD forest.

We routinely use this library in our migration projects as it can be easily integrated into PowerShell migration scripts and other toolset we develop and deliver to customers as a part of migration projects.

PowerShell sample below demostrates how easy it is to clone SID on many principals. Format of input file is showcased in inputFile.csv in this repo.


```powershell
Import-Module SidCloner
$sourceCred=Get-Credential
$targetCred=Get-Credential

Import-Csv .\inputFile.csv | Copy-Sid -SourceDomain domain1.com -TargetDomain domain2.com -SourceCredential $sourceCred -TargetCredential $targetCred
```

Assembly can also be used directly - provided static and instance methods for SID cloning.

PowerShell sample below is modified real-life script that migrated SID history for 10K users in one session. Sample demonstrates various methods provided by SIDCloner class - both stateless implementation via static methods and stateful instance methods following pattern Create -> Initialize -> Use.

We added instance methods for better performance, however testing has shown so far that peformance gain is smaller than expected - static methods are slower by less than 1%. Interested to hear from others - obviously there is a lot of optimization/caching on OS side as well.  


```powershell
param( 
    [parameter(Mandatory = $true)] 
    [String]$inFile,
    [parameter(Mandatory = $true)] 
    [String]$targetDomain,
    
    ) 
         
$errorPreference='Continue' 
 
$cwd=(Get-Item .).FullName 
[System.Reflection.Assembly]::LoadFile("$cwd\SIDCloner.dll") | Out-Null 
 
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
        [greycorbel.sidcloner]::CloneSid( 
            $record.sourceSAMAccountName, 
            $record.sourceDomain, 
            $record.targetSAMAccountName, 
            $targetDomain 
        ) 
 
        Write-Host "Account $($record.sourceDomain)\$($record.sourceSAMAccountName) cloned" 
    } 
    catch { 
        Write-Warning -message:"Account $($record.sourceDomain)\$($record.sourceSAMAccountName) failed to clone`n`tError:$($_.Exception.Message)" 
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
 
        [greycorbel.sidcloner]::CloneSid( 
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
#we use instance methods rather than static methods now
$targetCredential=Get-Credential 
$targetDC=Get-DC -domain:$targetDomain    #not implemented here 
$i=-1 
$cloner = new-object GreyCorbel.SidCloner
$cloner.Initialize($sourceDomain, $targetDomain,$sourceDc, $targetDc, $sourceCredential, $targetCredential)
foreach($record in $data) { 
    try { 
        $i++ 
        $percentComplete=[System.Convert]::ToInt32($i/($data.count)*100) 
        Write-Progress -Activity "Cloning users" -PercentComplete $percentComplete -CurrentOperation "Processing user: $sourceDomain`\$($record.sourceSAMAccountName)" -Status "Completed: $percentComplete`%" 
 
        $cloner.CloneSid( 
            $record.sourceSAMAccountName, 
            $record.targetSAMAccountName, 
        ) 
        Write-Host "Account $sourceDomain`\$($record.sourceSAMAccountName) cloned" 
    } 
    catch { 
        Write-Warning -message:"Account $sourceDomain`\$($record.sourceSAMAccountName) failed to clone`n`tError:$($_.Exception.Message)" 
        "$($record.sourceSAMAccountName),$sourceDomain,$($record.targetSAMAccountName)" >> ".\Log\CloneFailed.csv" 
    } 
} 
 
```
