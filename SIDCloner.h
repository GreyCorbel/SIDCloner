// SIDCloner.h

#pragma once
using namespace System;
using namespace System::Collections::Generic;
using namespace System::Security;
using namespace System::Net;
using namespace msclr::interop;
using namespace System::Management::Automation;

namespace GreyCorbel {

	public ref class SidCloner
	{
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

#pragma region StaticInterface
		static void CloneSid(String^ sourceIdentity, String^ sourceDomain, String^ targetIdentity, String^ targetDomain);
		static void CloneSid(String^ sourceIdentity, String^ sourceDomain, String^ sourceDC, String^ targetIdentity, String^ targetDomain, String^ targetDC);
		static void CloneSid(String^ sourceIdentity, String^ sourceDomain, String^ sourceDC, String^ sourceUserName, SecureString^ sourcePassword, String^ targetIdentity, String^ targetDomain);
		static void CloneSid(String^ sourceIdentity, String^ sourceDomain, String^ sourceDC, String^ sourceUserName, SecureString^ sourcePassword, String^ targetIdentity, String^ targetDomain, String^ targetDC, String^ targetUserName, SecureString^ targetPassword);
#pragma endregion

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
	};
}