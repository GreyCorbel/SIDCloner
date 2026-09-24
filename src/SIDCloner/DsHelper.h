#pragma once
using namespace System;
using namespace System::Collections::Generic;
using namespace System::Security;
using namespace System::Net;
using namespace msclr::interop;

ref class DsHelper
{
public:
	static void GetRpcCredentials(const wchar_t* pUserName, const wchar_t* pDomain, const wchar_t* pPassword, RPC_AUTH_IDENTITY_HANDLE* pHandle)
	{
		DWORD dwRslt = DsMakePasswordCredentials(pUserName, pDomain, pPassword, pHandle);	//caller is responsible for calling DsFreePasswordCredentials()
		if (ERROR_SUCCESS != dwRslt) {
			throw gcnew System::ComponentModel::Win32Exception(dwRslt);
		}
	}

	static void GetRpcCredentials(NetworkCredential^ Credential, RPC_AUTH_IDENTITY_HANDLE* pHandle)
	{
		pin_ptr<const wchar_t> pUserName = PtrToStringChars(Credential->UserName);
		pin_ptr<const wchar_t> pUserDomain = PtrToStringChars(Credential->Domain);
		pin_ptr<const wchar_t> pPassword = PtrToStringChars(Credential->Password);

		DWORD dwRslt = DsMakePasswordCredentials(pUserName, pUserDomain, pPassword, pHandle);	//caller is responsible for calling DsFreePasswordCredentials()
		if (ERROR_SUCCESS != dwRslt) {
			throw gcnew System::ComponentModel::Win32Exception(dwRslt);
		}

	}
	static void GetDSHandle(const wchar_t* pDomain, const wchar_t* pDC, RPC_AUTH_IDENTITY_HANDLE pIdentityHandle, HANDLE* pDSHandle)
	{
		DWORD dwRslt;
		if (pIdentityHandle == NULL) {
			dwRslt = DsBind(pDC, pDomain, pDSHandle);
		}
		else {
			dwRslt = DsBindWithCred(pDC, pDomain, pIdentityHandle, pDSHandle);
		}
		//caller is responsible for calling DsUnBind()
		if (ERROR_SUCCESS != dwRslt) {
			throw gcnew System::ComponentModel::Win32Exception(dwRslt);
		}
	}
};

