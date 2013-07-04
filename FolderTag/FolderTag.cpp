#include "stdafx.h"
#include "resource.h"

#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:wmainCRTStartup")
#pragma comment(lib, "Shlwapi.lib")

#define MAX_TAG_LENGTH (64)

_TCHAR tagName[MAX_TAG_LENGTH + 1];

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hwndOwner = GetParent(hwndDlg);
			if (hwndOwner == NULL)
			{
				hwndOwner = GetDesktopWindow();
			}

			RECT rc, rcDlg, rcOwner;
			GetWindowRect(hwndOwner, &rcOwner);
			GetWindowRect(hwndDlg, &rcDlg);
			CopyRect(&rc, &rcOwner);

			OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
			OffsetRect(&rc, -rc.left, -rc.top);
			OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

			SetWindowPos(hwndDlg, HWND_TOP, rcOwner.left + (rc.right / 2), rcOwner.top + (rc.bottom / 2), 0, 0, SWP_NOSIZE);

			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM) LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION)));
			SendMessage(GetDlgItem(hwndDlg, IDC_EDITTAG), EM_LIMITTEXT, MAX_TAG_LENGTH, 0);
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				SendMessage(GetDlgItem(hwndDlg, IDC_EDITTAG), WM_GETTEXT, MAX_TAG_LENGTH + 1, (LPARAM) tagName);

				EndDialog(hwndDlg, IDOK);
			}
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

LPTSTR GetFormattedMessage(LPTSTR message, ...)
{
	va_list args = NULL;
	va_start(args, message);

	LPTSTR buffer;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
		message,
		0,
		0,
		(LPTSTR) &buffer,
		0,
		&args);

	va_end(args);
	return buffer;
}

void Install()
{
	_TCHAR moduleFileName[MAX_PATH];
	GetModuleFileName(NULL, moduleFileName, MAX_PATH);

	_TCHAR *cmd = GetFormattedMessage(_T("\"%1\" \"%%1\""), moduleFileName);
	DWORD cmdLength = _tcslen(cmd) * sizeof(_TCHAR);
	SHSetValue(HKEY_CURRENT_USER, _T("Software\\Classes\\Folder\\shell\\Tag\\command"), NULL, REG_SZ, cmd, cmdLength);
	LocalFree(cmd);
}

void Uninstall()
{
	SHDeleteKey(HKEY_CURRENT_USER, _T("Software\\Classes\\Folder\\shell\\Tag"));
}

int _tmain(int argc, _TCHAR* argv [])
{
	if (argc < 2)
	{
		return 0;
	}

	if (_tcscmp(_T("/install"), argv[1]) == 0)
	{
		Install();
		return 0;
	}

	if (_tcscmp(_T("/uninstall"), argv[1]) == 0)
	{
		Uninstall();
		return 0;
	}

	if (IDOK != DialogBox(NULL, MAKEINTRESOURCE(IDD_TAGPROMPT), NULL, DialogProc))
	{
		return 0;
	}

	SHFOLDERCUSTOMSETTINGS settings = { 0 };
	settings.dwSize = sizeof(SHFOLDERCUSTOMSETTINGS);
	settings.dwMask = FCSM_INFOTIP;
	settings.pszInfoTip = tagName;

	for (int i = 1; i < argc; i++)
	{
		HRESULT hr = SHGetSetFolderCustomSettings(&settings, argv[i], FCS_FORCEWRITE);
	}

	return 0;
}