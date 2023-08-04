# SIDCloner
Module demonstrates how to populate SID History on security principals migrated cross AD forest via standard Windows APU [DsAddSidHistory](https://learn.microsoft.com/en-us/windows/win32/api/ntdsapi/nf-ntdsapi-dsaddsidhistoryw)

## Overview
Module is available on [PowerShell Gallery](https://www.powershellgallery.com/packages/SidCloner).
As the code is developed in C++, installation of [Visual C++ Redistributable](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads) is required for its use (C++/CLI projects cannot be compiled to statically link with C++ runtime).
Module is available for amd64 platform only - I did not see any strong need for other HW platofrms. Let me know if you need it for diferent platform.

We routinely use this library in our migration projects as it can be easily integrated into PowerShell migration scripts and other toolset we develop and deliver to customers as a part of migration projects.

## Samples
PowerShell sample below demostrates how easy it is to clone SID on many principals. Format of input file is showcased in inputFile.csv in this repo.

_Note_: Command is optimized for passing of source and target proncipal via pipeline: Necessary initializations and cleanup are performed in begin/end blocks only once.


```powershell
Import-Module SidCloner
$sourceCred=Get-Credential
$targetCred=Get-Credential

Import-Csv .\inputFile.csv | Copy-Sid -SourceDomain domain1.com -TargetDomain domain2.com -SourceCredential $sourceCred -TargetCredential $targetCred
```

Assembly can also be used directly - provides static and instance methods for SID cloning.
## Typical errors
### Inappropriate authentication
This error may occur when yoz pass credentials for source and target forest as `<NetBIOSDomainName>\<userSamAccountName>`.  
Use userPrincipalName format to avoid it.
### Auditing must be enabled in source domain
This error occurs when auditing is not properly configured. Typical case is configuration of Account Management in classic audit of GPO: Computer policy - Security settings - Local policies - Audit policy, but not in Computer policy - Security settings - Advanced Audit policy configuration.  
Make sure that ``Success`` and ``Failure`` audit is enabled in both places