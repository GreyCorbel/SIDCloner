#include "StdAfx.h"
#include "CloneResult.h"
#include "SidCloner.h"
#include "DsHelper.h"
#include "CredentialHelper.h"

namespace GreyCorbel {
	/// <summary>
	/// Initializes cloner object with source and target domain.
	/// Connection to domains uses implicit credentials - either caller's identity, or identity stored for domain in Windows Credential Manager.
	/// Domain controlllers to talk with are autodiscovered based on DNS SRV records.
	/// </summary>
	/// <param name="sourceDomain">DNS name of source domain</param>
	/// <param name="targetDomain">DNS name of target domain</param>
	void SidCloner::Initialize(String^ sourceDomain, String^ targetDomain)
	{
		this->Initialize(sourceDomain, targetDomain, nullptr, nullptr, nullptr, nullptr);
	}

	/// <summary>
	/// Initializes cloner object with source and target domain and preferred domain controllers to talk to. Initialization fails of preferred domain controller is not accessible.
	/// Connection to domains uses implicit credentials - either caller's identity, or identity stored for domain in Windows Credential Manager.
	/// </summary>
	/// <param name="sourceDomain">DNS name of source domain</param>
	/// <param name="targetDomain">DNS name of target domain</param>
	/// <param name="sourceDC">FQDN of DC for source domain to use</param>
	/// <param name="targetDC">FQDN of DC for source domain to use</param>
	void SidCloner::Initialize(
		String^ sourceDomain, 
		String^ targetDomain, 
		String^ sourceDC, 
		String^ targetDC)
	{
		this->Initialize(sourceDomain, targetDomain, sourceDC, targetDC, nullptr, nullptr);
	}

	/// <summary>
	/// Initializes cloner object with source and target domain and explicit credentials to use for cloning of SIDs.
	/// Domain controlllers to talk with are autodiscovered based on DNS SRV records.
	/// Source or target credential can be null - in such case, implicit credentials (caller's identitity of credentials stored in Windows Credential Manager) are used.
	/// </summary>
	/// <param name="sourceDomain">DNS name of source domain</param>
	/// <param name="targetDomain">DNS name of target domain</param>
	/// <param name="sourceCredential">Credential to use when authenticating operation against source domain. Use UPN rather than domain\sAMAccountName</param>
	/// <param name="targetCredential">Credential to use when authenticating operation against target domain. Use UPN rather than domain\sAMAccountName</param>
	void SidCloner::Initialize(
		String^ sourceDomain, 
		String^ targetDomain, 
		PSCredential^ sourceCredential, 
		PSCredential^ targetCredential)
	{
		this->Initialize(sourceDomain, targetDomain, nullptr, nullptr, sourceCredential, targetCredential);
	}

	/// <summary>
	/// Initializes cloner object with explicit domain names, DC names and credentials to use.
	/// Some parameters can be null - in such case, implicit values are used - see other overloads for details
	/// </summary>
	/// <param name="sourceDomain">DNS name of source domain</param>
	/// <param name="targetDomain">DNS name of target domain</param>
	/// <param name="sourceDC">FQDN of DC for source domain to use</param>
	/// <param name="targetDC">FQDN of DC for source domain to use</param>
	/// <param name="sourceCredential">Credential to use when authenticating operation against source domain. Use UPN rather than domain\sAMAccountName</param>
	/// <param name="targetCredential">Credential to use when authenticating operation against target domain. Use UPN rather than domain\sAMAccountName</param>
	void SidCloner::Initialize(
		String^ sourceDomain, 
		String^ targetDomain, 
		String^ sourceDC, 
		String^ targetDC, 
		PSCredential^ sourceCredential, 
		PSCredential^ targetCredential)
	{
		if (String::IsNullOrEmpty(sourceDomain) || String::IsNullOrEmpty(targetDomain))
			throw gcnew System::ArgumentException("You must provide source and target domain");

		ReleaseUnmanagedResources();

		marshal_context^ ctx = gcnew marshal_context();

		m_SourceDomain = ctx->marshal_as<const wchar_t*>(sourceDomain);
		m_TargetDomain = ctx->marshal_as<const wchar_t*>(targetDomain);

		if (!String::IsNullOrEmpty(sourceDC))
			m_SourceDc = ctx->marshal_as<const wchar_t*>(sourceDC);

		if (!String::IsNullOrEmpty(targetDC))
			m_TargetDc = ctx->marshal_as<const wchar_t*>(targetDC);

		if (nullptr != sourceCredential)
		{
			RPC_AUTH_IDENTITY_HANDLE h = NULL;
			auto cred = sourceCredential->GetNetworkCredential();
			CredentialHelper::NormalizeCredential(cred, sourceDomain);
			DsHelper::GetRpcCredentials(cred, &h);
			m_sourceAuthHandle = marshal_as<IntPtr>(h);
		}

		HANDLE targetDsHandle = NULL;
		if (nullptr != targetCredential)
		{
			RPC_AUTH_IDENTITY_HANDLE h = NULL;
			auto cred = targetCredential->GetNetworkCredential();
			CredentialHelper::NormalizeCredential(cred, targetDomain);
			DsHelper::GetRpcCredentials(cred, &h);
			m_targetAuthHandle = marshal_as<IntPtr>(h);
			DsHelper::GetDSHandle(m_TargetDomain, m_TargetDc, h, &targetDsHandle);
		}
		else
		{
			DsHelper::GetDSHandle(m_TargetDomain, m_TargetDc, NULL, &targetDsHandle);
		}
		m_TargetDsHandle = marshal_as<IntPtr>(targetDsHandle);

		m_initialized = true;
	}

	/// <summary>
	/// Adds SID of source principal to SID Histroy of target principal.
	/// </summary>
	/// <param name="sourceIdentity">sAMAccountName of principal from source domain - source of the SID</param>
	/// <param name="targetIdentity">sAMAccountName of principal from target domain - receiver of the SID</param>
	void SidCloner::CloneSid(String^ SourcePrincipal, String^ TargetPrincipal)
	{
		if (!m_initialized)
			throw gcnew System::InvalidOperationException("You must call Initialize() first");

		if (String::IsNullOrWhiteSpace(SourcePrincipal) || String::IsNullOrWhiteSpace(TargetPrincipal))
			throw gcnew System::ArgumentException("SourcePrincipal and TargetPrincipal must not be empty or whitespace only");


		pin_ptr<const wchar_t> pSourceIdentity = PtrToStringChars(SourcePrincipal);
		pin_ptr<const wchar_t> pTargetIdentity = PtrToStringChars(TargetPrincipal);

		HANDLE targetDSHandle = marshal_as<HANDLE>(m_TargetDsHandle);
		RPC_AUTH_IDENTITY_HANDLE sourceAuthHandle = marshal_as<RPC_AUTH_IDENTITY_HANDLE>(m_sourceAuthHandle);

		DWORD dwRslt = DsAddSidHistory(targetDSHandle, NULL, m_SourceDomain, pSourceIdentity, m_SourceDc, sourceAuthHandle, m_TargetDomain, pTargetIdentity);
		if (dwRslt != ERROR_SUCCESS)
			throw gcnew System::ComponentModel::Win32Exception(dwRslt);
	}
}