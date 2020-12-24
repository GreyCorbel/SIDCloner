// SIDCloner.h

#pragma once
using namespace System;
using namespace System::Collections::Generic;
using namespace System::Security;
using namespace System::Net;
using namespace msclr::interop;
using namespace System::Management::Automation;

namespace GreyCorbel {
#pragma region Cmdlet
	
	[CmdletAttribute("Copy", "Sid")]
	/// <summary>
	/// <para> Command copies SID from SourcePrincipal in SourceDomain to SID History of TargetPrincipal in TargetDomain.</para>
	/// <para>Command uses explicit domain controllers in Source or Target domain, or discovers suiteble Domain Controllers itself, if not specified</para>
	/// <para>Command uses explicit credentials to authorize the operation, if provided, or identity of caller, if explicit credentials not provided</para>
	/// </summary>
	public ref class SidCloner:public PSCmdlet
	{
	public:
		[Parameter(Mandatory = true, ValueFromPipelineByPropertyName = true, Position = 0, )]
		/// <summary>
		/// sAMAccountName of principal to copy the SID from.
		/// </summary>
		property String^ SourcePrincipal;

		[Parameter(Mandatory = true, ValueFromPipelineByPropertyName = true, Position = 1 )]
		/// <summary>
		/// sAMAccountName of principal that receives SID to its SID History from SourcePrincipal
		/// </summary>
		property String^ TargetPrincipal;

		[Parameter(Position = 2, Mandatory = true)]
		/// <summary>
		/// DNS name of domain hosting SourcePrincipal
		/// </summary>
		property String^ SourceDomain;

		[Parameter(Position = 3, Mandatory = true)]
		/// <summary>
		/// DNS name of domain hosting TargetPrincipal
		/// </summary>
		property String^ TargetDomain;

		[Parameter(Position = 4 )]
		/// <summary>
		/// FQDN of Domain Controller in SourceDomain that will be used by the operation
		/// </summary>
		property String^ SourceDC;

		[Parameter(Position = 5 )]
		/// <summary>
		/// FQDN of Domain Controller in TargetDomain that will be used by the operation
		/// </summary>
		property String^ TargetDC;

		[Parameter(Position = 6)]
		/// <summary>
		/// Explicit credentials to authorize operation is SourceDomain
		/// </summary>
		property PSCredential^ SourceCredential;

		[Parameter(Position = 7)]
		/// <summary>
		/// Explicit credentials to authorize operation is TargetDomain
		/// </summary>
		property PSCredential^ TargetCredential;

	public:
		virtual void BeginProcessing() override
		{
			Initialize(SourceDomain, TargetDomain, SourceDC, TargetDC, SourceCredential, TargetCredential);
		}

		virtual void ProcessRecord() override
		{
			if (String::IsNullOrWhiteSpace(SourcePrincipal) || String::IsNullOrWhiteSpace(TargetPrincipal))
				throw gcnew System::ArgumentException("Identities parameter must contain SourcePrincipal and TargetPrincipal properties.");

			CloneResult^ result = gcnew CloneResult();
			result->SourcePrincipal = SourcePrincipal;
			result->TargetPrincipal = TargetPrincipal;
			try
			{

				CloneSid(SourcePrincipal, TargetPrincipal);
				result->Result = ResultType::OK;
			}
			catch (Exception^ ex)
			{
				result->Result = ResultType::Error;
				result->ErrorDetail = ex;
			}

			WriteObject(result);
		}

		virtual void EndProcessing() override
		{
			ReleaseUnmanagedResources();
		}
#pragma endregion

#pragma region InstanceMethods
	public:
		void Initialize(String^ sourceDomain, String^ targetDomain);
		void Initialize(String^ sourceDomain, String^ targetDomain, String^ sourceDC, String^ targetDC);
		void Initialize(String^ sourceDomain, String^ targetDomain, PSCredential^ sourceCredential, PSCredential^ targetCredential);
		void Initialize(String^ sourceDomain, String^ targetDomain, String^ sourceDC, String^ targetDC, PSCredential^ sourceCredential, PSCredential^ targetCredential);

		void CloneSid(String^ sourceIdentity, String^ targetIdentity);

		SidCloner() {}
		~SidCloner()
		{
			if (m_disposed)
				return;
			
			this->!SidCloner();
			m_disposed = true;
		}

		!SidCloner()
		{
			ReleaseUnmanagedResources();
		}
#pragma endregion

#pragma region StaticInterface
		static void CloneSid(String^ sourceIdentity, String^ sourceDomain, String^ targetIdentity, String^ targetDomain);
		static void CloneSid(String^ sourceIdentity, String^ sourceDomain, String^ sourceDC, String^ targetIdentity, String^ targetDomain, String^ targetDC);
		static void CloneSid(String^ sourceIdentity, String^ sourceDomain, String^ sourceDC, String^ sourceUserName, SecureString^ sourcePassword, String^ targetIdentity, String^ targetDomain);
		static void CloneSid(String^ sourceIdentity, String^ sourceDomain, String^ sourceDC, String^ sourceUserName, SecureString^ sourcePassword, String^ targetIdentity, String^ targetDomain, String^ targetDC, String^ targetUserName, SecureString^ targetPassword);
#pragma endregion

#pragma region Privates

	private:
		static void CloneSid(String^ sourceIdentity, String^ sourceDomain, String^ sourceDC, String^ sourceUserName, String^ sourceUserDomain, const wchar_t* pSourcePassword, String^ targetIdentity, String^ targetDomain, String^ targetDC, String^ targetUserName, String^ targetUserDomain, const wchar_t* pTargetPassword);

	private:
		bool m_disposed = false;
		bool m_initialized = false;

		const wchar_t *m_SourceDomain = nullptr;
		const wchar_t *m_SourceDc = nullptr;
		const wchar_t *m_TargetDomain = nullptr;
		const wchar_t *m_TargetDc = nullptr;

		IntPtr m_sourceAuthHandle = IntPtr::Zero;
		IntPtr m_targetAuthHandle = IntPtr::Zero;
		IntPtr m_TargetDsHandle = IntPtr::Zero;

		void ReleaseUnmanagedResources() {
			if (m_TargetDsHandle != IntPtr::Zero)
			{
				HANDLE h = marshal_as<HANDLE>(m_TargetDsHandle);
				DsUnBind(&h);
				m_TargetDsHandle = IntPtr::Zero;
			}

			if (m_targetAuthHandle != IntPtr::Zero)
			{
				RPC_AUTH_IDENTITY_HANDLE h = marshal_as<RPC_AUTH_IDENTITY_HANDLE>(m_targetAuthHandle);
				DsFreePasswordCredentials(h);
				m_targetAuthHandle = IntPtr::Zero;
			}
			if (m_sourceAuthHandle != IntPtr::Zero)
			{
				RPC_AUTH_IDENTITY_HANDLE h = marshal_as<RPC_AUTH_IDENTITY_HANDLE>(m_sourceAuthHandle);
				DsFreePasswordCredentials(h);
				m_sourceAuthHandle = IntPtr::Zero;
			}

			//reset members
			m_TargetDc = nullptr;
			m_SourceDc = nullptr;
			m_SourceDomain = nullptr;
			m_TargetDomain = nullptr;

			m_initialized = false;
		}
#pragma endregion

	};
}