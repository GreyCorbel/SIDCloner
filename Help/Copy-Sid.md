---
external help file: SidCloner.dll-Help.xml
Module Name: SidCloner
online version:
schema: 2.0.0
---

# Copy-Sid

## SYNOPSIS
Command copies SID from SourcePrincipal in SourceDomain to SID History of TargetPrincipal in TargetDomain.

## SYNTAX

```powershell
Copy-Sid [-SourcePrincipal] <String> [-TargetPrincipal] <String> [-SourceDomain] <String> [-TargetDomain] <String> [[-SourceDC] <String>] [[-TargetDC] <String>] [[-SourceCredential] <PSCredential>] [[-TargetCredential] <PSCredential>] [<CommonParameters>]
 
```

## DESCRIPTION
Command copies SID from SourcePrincipal in SourceDomain to SID History of TargetPrincipal in TargetDomain.  
Command uses explicit domain controllers in Source or Target domain, or discovers suiteble Domain Controllers itself, if not specified.  
Command uses explicit credentials to authorize the operation, if provided, or identity of caller, if explicit credentials not provided.

## EXAMPLES

### Example 1
```powershell
$sourceCred=Get-Credential
$targetCred=Get-Credential

Import-Csv .\inputFile.csv | Copy-Sid -SourceDomain domain1.com -TargetDomain domain2.com -SourceCredential $sourceCred -TargetCredential $targetCred
```

Clones SIDs of accounts in inputFile.csv from domain1.com to accounts in domain2.com. Input file is CSV that needs to have 2 mandatory columns: SourcePrincipal and TargetPrincipal. CSV can contain additional columns as needed.

## PARAMETERS

### -SourceCredential
Explicit credentials to authorize operation is SourceDomain

```yaml
Type: System.Management.Automation.PSCredential
Parameter Sets: (All)
Aliases:

Required: False
Position: 6
Default value: None
Accept pipeline input: False
Accept wildcard characters: False
```

### -SourceDC
FQDN of Domain Controller in SourceDomain that will be used by the operation

```yaml
Type: System.String
Parameter Sets: (All)
Aliases:

Required: False
Position: 4
Default value: None
Accept pipeline input: False
Accept wildcard characters: False
```

### -SourceDomain
DNS name of AD domain hosting SourcePrincipal

```yaml
Type: System.String
Parameter Sets: (All)
Aliases:

Required: True
Position: 2
Default value: None
Accept pipeline input: False
Accept wildcard characters: False
```

### -SourcePrincipal
sAMAccountName of principal to copy the SID from.

```yaml
Type: System.String
Parameter Sets: (All)
Aliases:

Required: True
Position: 0
Default value: None
Accept pipeline input: True (ByPropertyName)
Accept wildcard characters: False
```

### -TargetCredential
Explicit credentials to authorize operation is TargetDomain

```yaml
Type: System.Management.Automation.PSCredential
Parameter Sets: (All)
Aliases:

Required: False
Position: 7
Default value: None
Accept pipeline input: False
Accept wildcard characters: False
```

### -TargetDC
FQDN of Domain Controller in TargetDomain that will be used by the operation

```yaml
Type: System.String
Parameter Sets: (All)
Aliases:

Required: False
Position: 5
Default value: None
Accept pipeline input: False
Accept wildcard characters: False
```

### -TargetDomain
DNS name of AD domain hosting TargetPrincipal

```yaml
Type: System.String
Parameter Sets: (All)
Aliases:

Required: True
Position: 3
Default value: None
Accept pipeline input: False
Accept wildcard characters: False
```

### -TargetPrincipal
sAMAccountName of principal that receives SID to its SID History from SourcePrincipal

```yaml
Type: System.String
Parameter Sets: (All)
Aliases:

Required: True
Position: 1
Default value: None
Accept pipeline input: True (ByPropertyName)
Accept wildcard characters: False
```

### CommonParameters
This cmdlet supports the common parameters: -Debug, -ErrorAction, -ErrorVariable, -InformationAction, -InformationVariable, -OutVariable, -OutBuffer, -PipelineVariable, -Verbose, -WarningAction, and -WarningVariable. For more information, see [about_CommonParameters](http://go.microsoft.com/fwlink/?LinkID=113216).

## INPUTS

### System.String
### System.Management.Automation.PSCredential

## OUTPUTS

### GreyCorbel.CloneResult
## NOTES
All prerequisites for usage of DsAddSidHistory API as spefied in [Microsoft Documentation](https://docs.microsoft.com/en-us/windows/win32/ad/using-dsaddsidhistory) must be fulfilled for command to succeed.
## RELATED LINKS
