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
		ParseCredentialName(userName, defaultDomain, userPart, domainPart);
	}

	static void NormalizeCredential(NetworkCredential^ cred, String^ defaultDomain)
	{
		if (cred == nullptr)
			throw gcnew ArgumentNullException("cred");

		String^ userName = cred->UserName;
		if (String::IsNullOrWhiteSpace(userName))
			throw gcnew ArgumentException("A credential user name is required.", "cred");

		if (userName->Contains("\\") || userName->Contains("@"))
		{
			String^ parsedUserName;
			String^ parsedDomain;
			ParseCredentialName(userName, defaultDomain, &parsedUserName, &parsedDomain);
			cred->UserName = parsedUserName;
			cred->Domain = parsedDomain;
		}
		else
		{
			cred->UserName = userName->Trim();
			if (String::IsNullOrWhiteSpace(cred->Domain))
				cred->Domain = defaultDomain;

			if (String::IsNullOrWhiteSpace(cred->Domain))
				throw gcnew ArgumentException("A credential domain is required when no default domain is available.", "defaultDomain");
		}
	}

private:
	static void ParseCredentialName(String^ value, String^ defaultDomain, String^* userPart, String^* domainPart)
	{
		if (String::IsNullOrWhiteSpace(value))
			throw gcnew ArgumentException("A credential user name is required.", "value");

		String^ normalized = value->Trim();
		bool hasBackslash = normalized->Contains("\\");
		bool hasAtSign = normalized->Contains("@");

		if (hasBackslash && hasAtSign)
			throw gcnew ArgumentException("Credential user names must use either DOMAIN\\user or user@domain format.", "value");

		if (hasBackslash || hasAtSign)
		{
			wchar_t delimiter = hasBackslash ? '\\' : '@';
			int delimiterIndex = normalized->IndexOf(delimiter);
			if (delimiterIndex != normalized->LastIndexOf(delimiter))
				throw gcnew ArgumentException("Credential user names must contain only one domain delimiter.", "value");

			String^ firstPart = normalized->Substring(0, delimiterIndex);
			String^ secondPart = normalized->Substring(delimiterIndex + 1);

			if (hasBackslash)
			{
				*domainPart = String::IsNullOrWhiteSpace(firstPart) ? defaultDomain : firstPart;
				*userPart = secondPart;
			}
			else
			{
				*userPart = firstPart;
				*domainPart = String::IsNullOrWhiteSpace(secondPart) ? defaultDomain : secondPart;
			}
		}
		else
		{
			*userPart = normalized;
			*domainPart = defaultDomain;
		}

		if (String::IsNullOrWhiteSpace(*userPart))
			throw gcnew ArgumentException("Credential user names must include a user name.", "value");

		if (String::IsNullOrWhiteSpace(*domainPart))
			throw gcnew ArgumentException("A credential domain is required when no default domain is available.", "defaultDomain");
	}

};
