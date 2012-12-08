/********************************************************************
	created:	2010/03/06
	created:	6:3:2010   10:29
	filename: 	kppfilepath.cpp
	author:		Jiang Fengbing
	
	purpose:	
*********************************************************************/

#include "kscfilepath.h"
#include <xhash>

//////////////////////////////////////////////////////////////////////////

namespace {

	template<typename Char> 
	struct CaseInsensitiveCompare 
	{
	public:
		bool operator()(Char x, Char y) const 
		{
			return tolower(x) == tolower(y);
		}
	};

	bool StartsWith(const std::wstring& str, const std::wstring& search, bool case_sensitive) 
	{
		if (case_sensitive)
		{
			return str.compare(0, search.length(), search) == 0;
		}
		else 
		{
			if (search.size() > str.size())
				return false;
			return std::equal(search.begin(), search.end(), str.begin(),
				CaseInsensitiveCompare<wchar_t>());
		}
	}

	std::wstring::size_type FindDriveLetter(const std::wstring& path) 
	{
		if (path.length() >= 2 && path[1] == L':' &&
			((path[0] >= L'A' && path[0] <= L'Z') ||
			(path[0] >= L'a' && path[0] <= L'z'))) 
		{
				return 1;
		}

		return std::wstring::npos;
	}

	bool EqualDriveLetterCaseInsensitive(const std::wstring a, const std::wstring b) 
	{
		size_t a_letter_pos = FindDriveLetter(a);
		size_t b_letter_pos = FindDriveLetter(b);

		if ((a_letter_pos == std::wstring::npos) ||
			(b_letter_pos == std::wstring::npos))
			return a == b;

		std::wstring a_letter(a.substr(0, a_letter_pos + 1));
		std::wstring b_letter(b.substr(0, b_letter_pos + 1));
		if (!StartsWith(a_letter, b_letter, false))
			return false;

		std::wstring a_rest(a.substr(a_letter_pos + 1));
		std::wstring b_rest(b.substr(b_letter_pos + 1));
		return a_rest == b_rest;
	}

	bool IsPathAbsolute(const std::wstring& path) 
	{
		std::wstring::size_type letter = FindDriveLetter(path);
		if (letter != std::wstring::npos) 
		{
			return path.length() > letter + 1 && 
				KFilePath::IsSeparator(path[letter + 1]);
		}

		return path.length() > 1 &&
			KFilePath::IsSeparator(path[0]) && KFilePath::IsSeparator(path[1]);
	}

	bool AreAllSeparators(const std::wstring& input) 
	{
		for (std::wstring::const_iterator it = input.begin(); it != input.end(); ++it) 
		{
			if (!KFilePath::IsSeparator(*it))
				return false;
		}

		return true;
	}
}

//////////////////////////////////////////////////////////////////////////

const wchar_t KFilePath::m_cSeparators[] = L"\\/";
const wchar_t KFilePath::m_cCurrentDirectory[] = L".";
const wchar_t KFilePath::m_cParentDirectory[] = L"..";
const wchar_t KFilePath::m_cExtensionSeparator = L'.';

//////////////////////////////////////////////////////////////////////////

bool KFilePath::IsAbsolute() const 
{
	return IsPathAbsolute(m_strPath);
}

//////////////////////////////////////////////////////////////////////////

bool KFilePath::IsSeparator(wchar_t character) 
{
	for (size_t i = 0; i < arraysize(m_cSeparators) - 1; ++i) 
	{
		if (character == m_cSeparators[i]) 
		{
			return true;
		}
	}

	return false;
}

bool KFilePath::IsDot(wchar_t character)
{
	if (L'.' == character)
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////
void KFilePath::GetComponents(std::vector<std::wstring>& vComponents) const 
{
	vComponents.clear();

	if (value().empty())
		return;

	std::vector<std::wstring> ret_val;
	KFilePath current = *this;
	KFilePath base;

	while (current != current.DirName()) 
	{
		base = current.BaseName();
		if (!AreAllSeparators(base.value()))
			ret_val.push_back(base.value());
		current = current.DirName();
	}

	base = current.BaseName();
	if (!base.value().empty() && base.value() != m_cCurrentDirectory)
		ret_val.push_back(current.BaseName().value());

	KFilePath dir = current.DirName();
	std::wstring::size_type letter = FindDriveLetter(dir.value());
	if (letter != std::wstring::npos) 
	{
		ret_val.push_back(std::wstring(dir.value(), 0, letter + 1));
	}

	vComponents = std::vector<std::wstring>(ret_val.rbegin(), ret_val.rend());
}

//////////////////////////////////////////////////////////////////////////

bool KFilePath::IsParent(const KFilePath& child) const 
{
	std::vector<std::wstring> parent_components;
	std::vector<std::wstring> child_components;
	GetComponents(parent_components);
	child.GetComponents(child_components);

	if (parent_components.size() >= child_components.size())
		return false;
	if (parent_components.size() == 0)
		return false;

	std::vector<std::wstring>::const_iterator parent_comp = parent_components.begin();
	std::vector<std::wstring>::const_iterator child_comp = child_components.begin();

	if ((FindDriveLetter(*parent_comp) != std::wstring::npos) &&
		(FindDriveLetter(*child_comp) != std::wstring::npos)) 
	{
		if (!StartsWith(*parent_comp, *child_comp, false))
			return false;
		++parent_comp;
		++child_comp;
	}

	while (parent_comp != parent_components.end()) 
	{
		if (*parent_comp != *child_comp)
			return false;
		++parent_comp;
		++child_comp;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

KFilePath KFilePath::BaseName() const 
{
	KFilePath new_path(m_strPath);
	new_path.StripTrailingSeparatorsInternal();

	std::wstring::size_type letter = FindDriveLetter(new_path.m_strPath);
	if (letter != std::wstring::npos) 
	{
		new_path.m_strPath.erase(0, letter + 1);
	}

	std::wstring::size_type last_separator =
		new_path.m_strPath.find_last_of(m_cSeparators, std::wstring::npos,
		arraysize(m_cSeparators) - 1);
	if (last_separator != std::wstring::npos &&
		last_separator < new_path.m_strPath.length() - 1) 
	{
		new_path.m_strPath.erase(0, last_separator + 1);
	}

	return new_path;
}

//////////////////////////////////////////////////////////////////////////

bool KFilePath::operator==(const KFilePath& path) const 
{
	return EqualDriveLetterCaseInsensitive(m_strPath, path.m_strPath);
}

//////////////////////////////////////////////////////////////////////////

bool KFilePath::operator!=(const KFilePath& path) const 
{
	return !EqualDriveLetterCaseInsensitive(m_strPath, path.m_strPath);
}

//////////////////////////////////////////////////////////////////////////

bool KFilePath::operator<(const KFilePath& path) const {

	return m_strPath < path.m_strPath;
}

//////////////////////////////////////////////////////////////////////////

KFilePath KFilePath::GetWindowsPath() 
{
	wchar_t szWindows[MAX_PATH] = { 0 };
	static KFilePath windowsPath;

	if (windowsPath.empty()) {
#pragma warning(push)
#pragma warning(disable: 6031)
		GetWindowsDirectoryW(szWindows, MAX_PATH);
#pragma warning(pop)
		windowsPath = KFilePath(szWindows);
	}

	return windowsPath;
}

//////////////////////////////////////////////////////////////////////////

KFilePath KFilePath::GetSystemPath() 
{
	wchar_t szSystem[MAX_PATH] = { 0 };
	static KFilePath systemPath;

	if (systemPath.empty())
	{
		GetSystemDirectoryW(szSystem, MAX_PATH);
		systemPath = KFilePath(szSystem);
	}
	
	return systemPath;
}

//////////////////////////////////////////////////////////////////////////

KFilePath KFilePath::StripTrailingSeparators() const 
{
	KFilePath new_path(m_strPath);
	new_path.StripTrailingSeparatorsInternal();

	return new_path;
}

//////////////////////////////////////////////////////////////////////////

std::wstring KFilePath::GetExtension() const 
{
	std::wstring base = BaseName().value();
	if (base == L"." || base == L"..")
		return std::wstring();

	const std::wstring::size_type last_dot = base.rfind(L".");
	if (last_dot == std::wstring::npos)
		return std::wstring();

	return std::wstring(base, last_dot);
}

void KFilePath::ReplaceExtension(const std::wstring& extension)
{
	if (m_strPath.empty())
		return;

	std::wstring base = BaseName().value();
	if (base.empty())
	{
		m_strPath = L"";
		return;
	}

	if (*(base.end() - 1) == m_cExtensionSeparator) 
	{
		if (base == m_cCurrentDirectory || base == m_cParentDirectory) 
		{
			m_strPath = L"";
			return;
		}
	}

	KFilePath no_ext(m_strPath);
	no_ext.RemoveExtension();
	if (extension.empty() || extension == std::wstring(1, m_cExtensionSeparator))
	{
		m_strPath = no_ext.value();
		return;
	}

	std::wstring str = no_ext.value();
	if (extension[0] != m_cExtensionSeparator)
		str.append(1, m_cExtensionSeparator);
	str.append(extension);
	m_strPath = str;
}

void KFilePath::RemoveExtension()
{
	std::wstring ext = GetExtension();
	if (ext.empty())
		return;

	const std::wstring::size_type last_dot = m_strPath.rfind(m_cExtensionSeparator);
	m_strPath = m_strPath.substr(0, last_dot);
}

void KFilePath::RemoveFileSpec()
{
	m_strPath = DirName().value();
}

//////////////////////////////////////////////////////////////////////////

void KFilePath::StripTrailingSeparatorsInternal() 
{
	std::wstring::size_type start = FindDriveLetter(m_strPath) + 2;

	std::wstring::size_type last_stripped = std::wstring::npos;
	for (std::wstring::size_type pos = m_strPath.length();
		pos > start && IsSeparator(m_strPath[pos - 1]);
		--pos) 
	{
		if (pos != start + 1 || last_stripped == start + 2 ||
			!IsSeparator(m_strPath[start - 1])) 
		{
			m_strPath.resize(pos - 1);
			last_stripped = pos;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

bool KFilePath::empty() const 
{ 
	return m_strPath.empty(); 
}

//////////////////////////////////////////////////////////////////////////

const std::wstring& KFilePath::value() const 
{ 
	return m_strPath; 
}

size_t KFilePath::hash_value() const
{
	return stdext::hash_value(m_strPath);
}

KFilePath::operator const wchar_t* () const
{
	return m_strPath.c_str();
}

//////////////////////////////////////////////////////////////////////////

bool KFilePath::IsDirectory() const 
{
	if (GetFileAttributesW(m_strPath.c_str()) & FILE_ATTRIBUTE_DIRECTORY)
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////

KFilePath KFilePath::DirName() const 
{
	KFilePath new_path(m_strPath);

	new_path.StripTrailingSeparatorsInternal();
	std::wstring::size_type letter = FindDriveLetter(new_path.m_strPath);

	std::wstring::size_type last_separator =
		new_path.m_strPath.find_last_of(m_cSeparators, std::wstring::npos,
		arraysize(m_cSeparators) - 1);
	if (last_separator == std::wstring::npos) 
	{
		new_path.m_strPath.resize(letter + 1);
	} 
	else if (last_separator == letter + 1) 
	{
		new_path.m_strPath.resize(letter + 2);
	} 
	else if (last_separator == letter + 2 &&
		IsSeparator(new_path.m_strPath[letter + 1])) 
	{
		new_path.m_strPath.resize(letter + 3);
	} 
	else if (last_separator != 0) 
	{
		new_path.m_strPath.resize(last_separator);
	}

	new_path.StripTrailingSeparatorsInternal();
	if (!new_path.m_strPath.length())
		new_path.m_strPath = m_cCurrentDirectory;

	return new_path;
}

//////////////////////////////////////////////////////////////////////////

void KFilePath::AddBackslash()
{
	if (m_strPath.size() < 1)
		return;

	if (IsSeparator(m_strPath[m_strPath.size() - 1]))
		return;

	m_strPath.append(L"\\");
}

void KFilePath::RemoveBackslash()
{
	if (m_strPath.size() < 1)
		return;

	if (IsSeparator(m_strPath[m_strPath.length() - 1]))
	{
		m_strPath.erase(m_strPath.length() - 1);
	}
}

void KFilePath::RemoveHeadBackslash()
{
	if (m_strPath.length() < 1)
		return;

	if (m_strPath[0] == L'\\')
		m_strPath = m_strPath.c_str() + 1;

	if (m_strPath.length() > 2)
	{
		if (m_strPath[0] == L'.' && m_strPath[1] == L'\\')
			m_strPath = m_strPath.c_str() + 1;
	}
}

void KFilePath::RemoveQuotes()
{
	if (m_strPath[0] == L'"')
	{
		m_strPath.erase(m_strPath.begin());
		m_strPath.erase(m_strPath.end());
	}
}

void KFilePath::AddExtension(const std::wstring& strExtension)
{
	if (m_strPath.size() < 1)
		return;

	if (strExtension.size() < 1)
		return;

	if (IsDot(m_strPath[m_strPath.size() - 1]))
	{
		if (IsDot(strExtension[0]))
		{
			m_strPath.append(strExtension.c_str() + 1);
		}
		else
		{
			m_strPath.append(strExtension);
		}
	}
	else
	{
		if (IsDot(strExtension[0]))
		{
			m_strPath.append(strExtension);
		}
		else
		{
			m_strPath.append(L".").append(strExtension);
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void KFilePath::Append(const std::wstring& strComponent)
{
	if (m_strPath.compare(m_cCurrentDirectory) == 0) 
	{
		m_strPath = strComponent;
		return;
	}

	KFilePath new_path(m_strPath);
	new_path.StripTrailingSeparatorsInternal();

	if (strComponent.length() > 0 && new_path.m_strPath.length() > 0) 
	{
		if (!IsSeparator(new_path.m_strPath[new_path.m_strPath.length() - 1])) 
		{
			if (FindDriveLetter(new_path.m_strPath) + 1 != new_path.m_strPath.length()) 
			{
				new_path.m_strPath.append(1, m_cSeparators[0]);
			}
		}
	}

	new_path.AddBackslash();
	KFilePath pathAppend(strComponent);
	pathAppend.RemoveHeadBackslash();
	new_path.m_strPath.append(pathAppend);
	m_strPath = new_path.value();
}

void KFilePath::Append(const KFilePath& component)
{
 	Append(component.value());
}

//////////////////////////////////////////////////////////////////////////

bool KFilePath::FileExists()
{
	if (GetFileAttributes(m_strPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////

void KFilePath::Attach(std::wstring strFileName)
{
	m_strPath = strFileName;
}

//////////////////////////////////////////////////////////////////////////

std::wstring KFilePath::Detach()
{
	std::wstring retval = m_strPath;
	m_strPath = L"";

	return retval;
}

//////////////////////////////////////////////////////////////////////////

KFilePath KFilePath::GetFilePath(HMODULE hModule)
{
	bool retval = false;
	wchar_t szFileName[MAX_PATH] = { 0 };
	DWORD dwRetCode;

	dwRetCode = ::GetModuleFileNameW(hModule, szFileName, MAX_PATH);
	if (dwRetCode)
		return KFilePath(szFileName);

	return KFilePath();
}

//////////////////////////////////////////////////////////////////////////

bool KFilePath::MatchesExtension(const std::wstring& extension) const 
{
	std::wstring current_extension = GetExtension();

	if (current_extension.length() != extension.length())
		return false;

	return std::equal(extension.begin(),
		extension.end(),
		current_extension.begin(),
		CaseInsensitiveCompare<wchar_t>());
}

//////////////////////////////////////////////////////////////////////////

namespace
{
	typedef HRESULT (__stdcall* GetUserProfileDirW_t)(HWND, int, HANDLE, DWORD, LPWSTR);
}

KFilePath KFilePath::GetFolderPath(int nFolder)
{
	HRESULT hr = E_FAIL;
	wchar_t szFolderPath[MAX_PATH] = {0};
	DWORD dwSize = MAX_PATH;
	GetUserProfileDirW_t SHGetFolderPathW = NULL;
	KFilePath retval;
	HINSTANCE hShFolder = NULL;

	hShFolder = ::LoadLibraryW(L"ShFolder.dll");
	if (!hShFolder)
		goto clean0;

	SHGetFolderPathW = (GetUserProfileDirW_t)::GetProcAddress(hShFolder, "SHGetFolderPathW");
	if (!SHGetFolderPathW)
		goto clean0;
	
	hr = SHGetFolderPathW(NULL, nFolder, NULL, 0, szFolderPath);
	if (FAILED(hr))
		goto clean0;

	retval = szFolderPath;

clean0:
	if (hShFolder)
	{
		::FreeLibrary(hShFolder);
		hShFolder = NULL;
	}

	return retval;
}

//////////////////////////////////////////////////////////////////////////
