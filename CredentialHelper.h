#pragma once
using namespace System;
using namespace System::Collections::Generic;
using namespace System::Security;
using namespace System::Net;
using namespace msclr::interop;

ref class CredentialHelper
{
public:
	static void CrackName(String^ userName, String^* userPart, String^* domainPart, String^ defaultDomain)
	{
		String^ u = userName->Replace(" ", "");

		//domain\username was passed
		if (u->IndexOf("\\") > -1) {
			array<String^>^ arr = u->Split('\\');
			*userPart = arr[1];
			if (!String::IsNullOrEmpty(arr[0])) {
				*domainPart = arr[0];
			}
			else {
				*domainPart = defaultDomain;
			}
			return;
		}
		//username@domain was passed
		if (u->IndexOf("@") > -1) {
			array<String^>^ arr = u->Split('@');
			*userPart = arr[0];
			if (!String::IsNullOrEmpty(arr[1])) {
				*domainPart = arr[1];
			}
			else {
				*domainPart = defaultDomain;
			}
			return;
		}
		//username without domain was passed
		*userPart = u;
		*domainPart = defaultDomain;
	}

	static void NormalizeCredential(NetworkCredential^ cred, String^ defaultDomain)
	{
		if (!String::IsNullOrEmpty(cred->Domain) && cred->UserName->IndexOf('\\') == -1 && cred->UserName->IndexOf('@') == -1)
			return;

		//domain\username was passed
		if (cred->UserName->IndexOf('\\') > -1) {
			array<String^>^ arr = cred->UserName->Split('\\');
			cred->UserName = arr[1];
			if (!String::IsNullOrEmpty(arr[0])) {
				cred->Domain = arr[0];
			}
			else {
				cred->Domain = defaultDomain;
			}
			return;
		}

		//username@domain was passed
		if (cred->UserName->IndexOf('@') > -1) {
			array<String^>^ arr = cred->UserName->Split('@');
			cred->UserName = arr[0];
			if (!String::IsNullOrEmpty(arr[1])) {
				cred->Domain = arr[1];
			}
			else {
				cred->Domain = defaultDomain;
			}
			return;
		}
	}

};

