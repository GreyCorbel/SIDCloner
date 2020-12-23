#include "StdAfx.h"
#include "CloneResult.h"
#include "SidCloner.h"
#include "DsHelper.h"
#include "CredentialHelper.h"

namespace GreyCorbel {

	void SidCloner::CloneSid(String^ sourceIdentity, String^ sourceDomain, String^ sourceDC, String^ sourceUserName, SecureString^ sourcePassword, String^ targetIdentity, String^ targetDomain) {
		IntPtr pSourcePwdPtr=IntPtr::Zero;
		try {
			String ^su;
			String ^sd;
			CredentialHelper::CrackName(sourceUserName,&su,&sd,sourceDomain);

			pSourcePwdPtr=System::Runtime::InteropServices::Marshal::SecureStringToGlobalAllocUnicode(sourcePassword);
			CloneSid(sourceIdentity, sourceDomain, sourceDC, su, sd, (const wchar_t*)(pSourcePwdPtr.ToPointer()), targetIdentity, targetDomain, nullptr, nullptr, nullptr, (const wchar_t*)nullptr);
		}
		catch(Exception^ e) {
			throw e;
		}
		finally {
			if(pSourcePwdPtr!=IntPtr::Zero)
				System::Runtime::InteropServices::Marshal::ZeroFreeGlobalAllocUnicode(pSourcePwdPtr);
		}
	}


	void SidCloner::CloneSid(String^ sourceIdentity, String^ sourceDomain, String^ targetIdentity, String^ targetDomain) {
		CloneSid(sourceIdentity, sourceDomain, nullptr, nullptr, nullptr, (const wchar_t*)nullptr, targetIdentity, targetDomain, nullptr, nullptr, nullptr, (const wchar_t*)nullptr);
	}
		
	void SidCloner::CloneSid(String^ sourceIdentity, String^ sourceDomain, String^ sourceDC, String^ targetIdentity, String^ targetDomain, String^ targetDC) {
		CloneSid(sourceIdentity, sourceDomain, sourceDC, nullptr, nullptr, (const wchar_t*)nullptr, targetIdentity, targetDomain, targetDC, nullptr, nullptr, (const wchar_t*)nullptr);
	}

	void SidCloner::CloneSid(String^ sourceIdentity, String^ sourceDomain, String^ sourceDC, String^ sourceUserName, SecureString^ sourcePassword, String^ targetIdentity, String^ targetDomain, String^ targetDC, String^ targetUserName, SecureString^ targetPassword) {
		IntPtr pSourcePwdPtr=IntPtr::Zero;
		IntPtr pTargetPwdPtr=IntPtr::Zero;
		try {
			String ^su;
			String ^sd;
			CredentialHelper::CrackName(sourceUserName,&su,&sd,sourceDomain);
			String ^tu;
			String ^td;
			CredentialHelper::CrackName(targetUserName,&tu,&td,targetDomain);

			pSourcePwdPtr=System::Runtime::InteropServices::Marshal::SecureStringToGlobalAllocUnicode(sourcePassword);
			pTargetPwdPtr=System::Runtime::InteropServices::Marshal::SecureStringToGlobalAllocUnicode(targetPassword);
			CloneSid(sourceIdentity, sourceDomain, sourceDC, su, sd, (const wchar_t*)(pSourcePwdPtr.ToPointer()), targetIdentity, targetDomain, targetDC, tu, td,  (const wchar_t*)(pTargetPwdPtr.ToPointer()));
		}
		catch(Exception^ e) {
			throw e;
		}
		finally {
			if(pSourcePwdPtr!=IntPtr::Zero)
				System::Runtime::InteropServices::Marshal::ZeroFreeGlobalAllocUnicode(pSourcePwdPtr);
			if(pTargetPwdPtr!=IntPtr::Zero)
				System::Runtime::InteropServices::Marshal::ZeroFreeGlobalAllocUnicode(pTargetPwdPtr);
		}
	}

	void SidCloner::CloneSid(
		String^ sourceIdentity, 
		String^ sourceDomain, 
		String^ sourceDC, 
		String^ sourceUserName, 
		String^ sourceUserDomain, 
		const wchar_t* pSourcePassword, 
		String^ targetIdentity, 
		String^ targetDomain, 
		String^ targetDC, 
		String^ targetUserName, 
		String^ targetUserDomain, 
		const wchar_t* pTargetPassword)
	{
		if(String::IsNullOrEmpty(sourceIdentity) || String::IsNullOrEmpty(sourceDomain) || String::IsNullOrEmpty(targetIdentity) || String::IsNullOrEmpty(targetDomain))
			throw gcnew System::ComponentModel::Win32Exception(ERROR_INVALID_PARAMETER);
		
		pin_ptr<const wchar_t> pSourceIdentity=PtrToStringChars(sourceIdentity);
		pin_ptr<const wchar_t> pSourceDomain=PtrToStringChars(sourceDomain);
		pin_ptr<const wchar_t> pSourceDC=nullptr;
		pin_ptr<const wchar_t> pTargetIdentity=PtrToStringChars(targetIdentity);
		pin_ptr<const wchar_t> pTargetDomain=PtrToStringChars(targetDomain);
		pin_ptr<const wchar_t> pTargetDC=nullptr;


		if(!String::IsNullOrEmpty(sourceDC))
			pSourceDC=PtrToStringChars(sourceDC);
		if(!String::IsNullOrEmpty(targetDC))
			pTargetDC=PtrToStringChars(targetDC);

		RPC_AUTH_IDENTITY_HANDLE sourceAuthHandle=NULL;
		RPC_AUTH_IDENTITY_HANDLE targetAuthHandle=NULL;
		HANDLE targetDsHandle=NULL;
		

		DWORD dwRslt=ERROR_SUCCESS;
		try {
			if(!String::IsNullOrEmpty(sourceUserName)) {
				pin_ptr<const wchar_t> pSourceUserName=PtrToStringChars(sourceUserName);
				pin_ptr<const wchar_t> pSourceUserDomain=PtrToStringChars(sourceUserDomain);
				//pass both username and domain - see my comment at DsMakePasswordCredentials() description in MSDN
				DsHelper::GetRpcCredentials(pSourceUserName,pSourceUserDomain,pSourcePassword,&sourceAuthHandle);
			}
			if(!String::IsNullOrEmpty(targetUserName)) {
				pin_ptr<const wchar_t> pTargetUserName=PtrToStringChars(targetUserName);
				pin_ptr<const wchar_t> pTargetUserDomain=PtrToStringChars(targetUserDomain);
				//pass both username and domain - see my comment at DsMakePasswordCredentials() description in MSDN
				DsHelper::GetRpcCredentials(pTargetUserName,pTargetUserDomain,pTargetPassword,&targetAuthHandle);
			}
			DsHelper::GetDSHandle(pTargetDomain, pTargetDC, targetAuthHandle,&targetDsHandle);

			dwRslt=DsAddSidHistory(targetDsHandle,NULL,pSourceDomain,pSourceIdentity,pSourceDC,sourceAuthHandle,pTargetDomain,pTargetIdentity);
		}
		catch(Exception^ e) {
			throw e;
		}
		finally {
			if(targetDsHandle != NULL)
				DsUnBind(&targetDsHandle);
			if(targetAuthHandle !=NULL)
				DsFreePasswordCredentials(targetAuthHandle);
			if(sourceAuthHandle !=NULL)
				DsFreePasswordCredentials(sourceAuthHandle);
		}
		if(dwRslt!=ERROR_SUCCESS)
			throw gcnew System::ComponentModel::Win32Exception(dwRslt);
	}
}