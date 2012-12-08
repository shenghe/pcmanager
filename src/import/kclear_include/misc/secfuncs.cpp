#include <windows.h>
#include "ntdll_lite.h"
#include <atlbase.h>

//////////////////////////////////////////////////////////////////////////

typedef struct tagMySafe {
	SECURITY_DESCRIPTOR securityDescriptor;
	ACL acl;
} MySafe;

//////////////////////////////////////////////////////////////////////////

PSECURITY_DESCRIPTOR CreateKeySecurityDescriptor(DWORD dwType) {
	PSECURITY_DESCRIPTOR retval = NULL;
	NTSTATUS status;
	SID_IDENTIFIER_AUTHORITY sid_id_auth = { 0, 0, 0, 0, 0, 5 };
	PSID pSid1 = NULL;
	PSID pSid2 = NULL;
	ULONG dwSid1;
	ULONG dwSid2;
	MySafe* pMySafe = NULL;
	ULONG dwLength;

	status = RtlAllocateAndInitializeSid(&sid_id_auth, 2, 32, 544, 0, 0, 0, 0, 0, 0, &pSid1);
	if (!NT_SUCCESS(status))
		goto clean0;

	status = RtlAllocateAndInitializeSid(&sid_id_auth, 1, 18, 0, 0, 0, 0, 0, 0, 0, &pSid2);
	if (!NT_SUCCESS(status))
		goto clean0;

	dwSid1 = RtlLengthSid(pSid1);
	dwSid2 = RtlLengthSid(pSid1);
	pMySafe = (MySafe*)malloc(dwSid1 * 2 + dwSid2 + 52);
	if (!pMySafe)
		goto clean0;

	memset(pMySafe, 1, dwSid1 * 2 + dwSid2 + 52);

	status = RtlCreateAcl(&pMySafe->acl, dwSid1 + dwSid2 + 32, 2);
	if (!NT_SUCCESS(status))
		goto clean0;

	status = RtlAddAccessAllowedAceEx(&pMySafe->acl, 2, 0, dwType, pSid1);
	if (!NT_SUCCESS(status))
		goto clean0;

	status = RtlAddAccessAllowedAceEx(&pMySafe->acl, 2, 0, 0xF003Fu, pSid2);
	if (!NT_SUCCESS(status))
		goto clean0;

	status = RtlCreateSecurityDescriptor(&pMySafe->securityDescriptor, 1);
	if (!NT_SUCCESS(status))
		goto clean0;

	status = RtlSetDaclSecurityDescriptor(
		&pMySafe->securityDescriptor, TRUE, &pMySafe->acl, FALSE);
	if (!NT_SUCCESS(status))
		goto clean0;

	dwLength = RtlLengthSecurityDescriptor(&pMySafe->securityDescriptor);
	if (!dwLength)
		goto clean0;

	memcpy((UCHAR*)pMySafe + dwLength, pSid1, RtlLengthSid(pSid1));

	status = RtlSetOwnerSecurityDescriptor(
		&pMySafe->securityDescriptor, (UCHAR*)pMySafe + dwLength, 0);
	if (!NT_SUCCESS(status))
		goto clean0;

	retval = &pMySafe->securityDescriptor;

clean0:
	if (pSid1) {
		RtlFreeSid(pSid1);
		pSid1 = NULL;
	}

	if (pSid2) {
		RtlFreeSid(pSid2);
		pSid2 = NULL;
	}

	if (!NT_SUCCESS(status)) {
		free(pMySafe);
		pMySafe = NULL;
	}

	return retval;
}

BOOL GrantRegkeyAccess(HKEY hKey) {
	BOOL retval = FALSE;
	PSECURITY_DESCRIPTOR pDescurityDescriptor = NULL;
	NTSTATUS status;
	
	if (!hKey)
		goto clean0;

	pDescurityDescriptor = CreateKeySecurityDescriptor(0xF003F);
	if (!pDescurityDescriptor)
		goto clean0;

	status = NtSetSecurityObject(hKey, SE_REGISTRY_KEY, pDescurityDescriptor);
	if (!NT_SUCCESS(status))
		goto clean0;

	retval = TRUE;

clean0:
	if (pDescurityDescriptor) {
		free(pDescurityDescriptor);
		pDescurityDescriptor = NULL;
	}

	return retval;
}

void GrantAllAccess(HKEY hKey) {
	ATL::CRegKey regKey;
	wchar_t* szName = new wchar_t[256];
	DWORD dwName = 256;
	LONG lRetCode = 0;
	PSECURITY_DESCRIPTOR pDescurityDescriptor = NULL;
	NTSTATUS status;
	int i = 0;

	regKey.Attach(hKey);

	pDescurityDescriptor = CreateKeySecurityDescriptor(0xF003F);
	if (!pDescurityDescriptor)
		goto clean0;

	for (;;) {

		dwName = 256;
		lRetCode = regKey.EnumKey(i++, szName, &dwName, NULL);
		if (lRetCode)
			break;

		CRegKey subReg;
		lRetCode = subReg.Open(regKey, szName, READ_CONTROL|WRITE_DAC);
		if (lRetCode)
			goto clean0;

		status = NtSetSecurityObject(subReg, SE_REGISTRY_KEY, pDescurityDescriptor);
		if (!NT_SUCCESS(status))
			goto clean0;

		subReg.Close();
		lRetCode = subReg.Open(regKey, szName, KEY_ALL_ACCESS);
		if (lRetCode)
			goto clean0;

		GrantAllAccess(subReg);
	}

	if (lRetCode)
		goto clean0;

clean0:
	if (szName) {
		delete[] szName;
		szName = NULL;
	}

	if (pDescurityDescriptor) {
		free(pDescurityDescriptor);
		pDescurityDescriptor = NULL;
	}

	regKey.Detach();
}
