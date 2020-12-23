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
	public ref class SidCloner:public PSCmdlet
	{
	public:
		[Parameter(Mandatory = true, ValueFromPipeline = true, ParameterSetName = "MappingObject")]
		property PSObject^ Identities;

		[Parameter(Mandatory = true, ParameterSetName = "SeparateNames")]
		property String^ SourcePrincipal;

		[Parameter(Mandatory = true, ParameterSetName = "SeparateNames")]
		property String^ TargetPrincipal;

		[Parameter(Position = 0, Mandatory = true)]
		property String^ SourceDomain;

		[Parameter(Position = 1, Mandatory = true)]
		property String^ TargetDomain;

		[Parameter(Position = 2 )]
		property String^ SourceDC;

		[Parameter(Position = 3 )]
		property String^ TargetDC;

		[Parameter(Position = 4)]
		property PSCredential^ SourceCredential;

		[Parameter(Position = 5)]
		property PSCredential^ TargetCredential;

	public:
		virtual void BeginProcessing() override
		{
			Initialize(SourceDomain, TargetDomain, SourceDC, TargetDC, SourceCredential, TargetCredential);
		}

		virtual void ProcessRecord() override
		{
			if (this->ParameterSetName == "MappingObject")
			{
				for each (auto prop in Identities->Properties)
				{
					if (prop->Name->Equals("SourcePrincipal", StringComparison::CurrentCultureIgnoreCase))
						SourcePrincipal = dynamic_cast<String^>(prop->Value);
					if (prop->Name->Equals("TargetPrincipal", StringComparison::CurrentCultureIgnoreCase))
						TargetPrincipal = dynamic_cast<String^>(prop->Value);
				}
				if (String::IsNullOrWhiteSpace(SourcePrincipal) || String::IsNullOrWhiteSpace(TargetPrincipal))
					throw gcnew System::ArgumentException("Identities parameter must contain SourcePrincipal and TargetPrincipal properties.");
			}

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