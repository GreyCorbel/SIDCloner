# SIDCloner

SIDCloner is a PowerShell module and C++/CLI assembly for adding the SID of a principal in a source Active Directory forest to the `sIDHistory` attribute of a principal in a target forest. It uses the standard Windows API [DsAddSidHistory](https://learn.microsoft.com/en-us/windows/win32/api/ntdsapi/nf-ntdsapi-dsaddsidhistoryw).

## Overview

The module is available on the [PowerShell Gallery](https://www.powershellgallery.com/packages/SidCloner). It targets 64-bit Windows; install the [Visual C++ Redistributable](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads) before using it because C++/CLI projects cannot statically link the C++ runtime.

SID history preserves access to resources that still have ACLs containing SIDs from the source forest. Adding SID history is a privileged migration operation: validate each source-to-target mapping, protect the output records, and remove SID history only according to your migration and security-retention plan.

## Releases

Pushing a tag matching `vMAJOR.MINOR.PATCH` runs the GitHub Actions release workflow. It builds the x64 release binary, composes the PowerShell module, signs the DLL with Azure Artifact Signing, and publishes the module to the PowerShell Gallery.

The workflow uses GitHub OIDC through the `release` environment. Configure the environment in the repository settings and add a federated credential to the Azure app registration:

```bash
az ad app federated-credential create \
  --id "$TENANTINTEGRATION_CLIENTID" \
  --parameters '{
    "name": "github-sidcloner-release",
    "issuer": "https://token.actions.githubusercontent.com",
    "subject": "repo:GreyCorbel/SIDCloner:environment:release",
    "audiences": ["api://AzureADTokenExchange"]
  }'
```

Grant that app registration the **Artifact Signing Certificate Profile Signer** role on the `greycorbel` signing account. The repository also needs the `GC_PSGALLERY_APIKEY` secret; `TENANTINTEGRATION_CLIENTID` and `TENANTINTEGRATION_TENANTID` are read as organization variables.

## Prerequisites

Before running `Copy-Sid`, verify the requirements in [Using DsAddSidHistory](https://learn.microsoft.com/en-us/windows/win32/ad/using-dsaddsidhistory):

- The source and target forests/domains must meet the required trust and migration configuration requirements.
- The account performing the operation must have the required permissions in both environments. Prefer least-privileged, dedicated migration credentials.
- A writable target domain controller must be available. When required by your Active Directory configuration, direct the operation to the target domain's PDC Emulator by using `-TargetDC`.
- Domain controller discovery requires DNS connectivity; specify `-SourceDC` and `-TargetDC` with fully qualified domain names when discovery is unsuitable.
- The necessary source-domain auditing policy must be enabled before the operation.

## Samples

The following PowerShell example adds SID history for multiple principals. The expected input format is shown in [`inputFile.csv`](inputFile.csv).

> **Note:** `Copy-Sid` is optimized for pipeline input. It initializes the domain connections once in `BeginProcessing` and releases them once in `EndProcessing`.

```powershell
Import-Module SidCloner
$sourceCred = Get-Credential
$targetCred = Get-Credential

Import-Csv .\inputFile.csv | Copy-Sid -SourceDomain domain1.com -TargetDomain domain2.com -SourceCredential $sourceCred -TargetCredential $targetCred
```

The assembly can also be used directly through its static and instance methods.

## Typical errors

Confirm that every prerequisite in [Using DsAddSidHistory](https://learn.microsoft.com/en-us/windows/win32/ad/using-dsaddsidhistory) is fulfilled before troubleshooting application errors.

### Authentication format

Authentication can fail when source or target credentials use `<NetBIOSDomainName>\<sAMAccountName>`. Supply the user name in user principal name (UPN) form instead, for example `migration-admin@domain.example`.

### Auditing

SID history operations can fail when source-domain auditing is not configured. Enable both **Success** and **Failure** for the required auditing category in the classic audit policy and in the Advanced Audit Policy Configuration:

`Computer Configuration > Windows Settings > Security Settings > Local Policies > Audit Policy`

`Computer Configuration > Windows Settings > Security Settings > Advanced Audit Policy Configuration`
