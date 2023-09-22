#define _CRT_SECURE_NO_WARNINGS
#include<Windows.h>
#include<Richedit.h>
#include<cstdio>
#include"resource.h"
#include<CommCtrl.h>

CONST CHAR g_sz_WINDOW_CLASS[] = "My Text Editor";
HFONT g_hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
COLORREF g_RGB_Text = RGB(0, 0, 0);

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL LoadTextFileToEdit(HWND hEdit, LPSTR lpszFileName);
BOOL SaveTextFileFromEdit(HWND hEdit, LPSTR lpszFileName);
BOOL FileWasChanger(HWND hEdit, LPSTR lpszText, LPSTR lpszFileText);

VOID SelectFont(HWND hwnd);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
	//1) Регистрируем класс окна
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.cbSize = sizeof(wc);
	wc.style = 0;

	wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);
	wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
	//wc.hbrBackground = CreateSolidBrush(RGB(1, 96, 160));
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	wc.hInstance = hInstance;
	wc.lpszClassName = g_sz_WINDOW_CLASS;
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wc.lpfnWndProc = WndProc;

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Class registration faild", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	//2)Создаем окна:
	HWND hwnd = CreateWindowEx
	(
		WS_EX_ACCEPTFILES,
		g_sz_WINDOW_CLASS,
		g_sz_WINDOW_CLASS,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		0,
		hInstance,
		NULL
	);

	if (hwnd == NULL)
	{
		MessageBox(NULL, "Window creation failed", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	//3) Создаем цикл сообщений
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CHAR szFileName[MAX_PATH]{};
	static LPSTR lpszText = NULL;
	static LPSTR lpszFileText = NULL;
	switch (uMsg)
	{
	case WM_CREATE:
	{
		LoadLibrary("riched20.dll");
		RECT rect;
		GetClientRect(hwnd, &rect);
		HWND hEdit = CreateWindowEx
		(
			NULL, RICHEDIT_CLASS, "",
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL,
			rect.left, rect.top,
			rect.right - rect.left, rect.bottom - rect.top,
			hwnd,
			(HMENU)IDC_EDIT,
			GetModuleHandle(NULL),
			0
		);
		HWND hStatus = CreateWindowEx
		( 
			NULL, STATUSCLASSNAME, NULL,
			WS_CHILD | WS_VISIBLE,
			0,0,0,0,
			hwnd,
			(HMENU)ID_STATUS,
			GetModuleHandle(NULL),
			0
		);
	}
	break;
	case WM_MOVE: 
	{
		/*OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lpstrFile = szFileName;

		CONST INT SIZE = 256;
		CHAR sz_buffer[SIZE] = {};
		sprintf(sz_buffer, szFileName);
		SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)sz_buffer);*/
	}


	break;
	case WM_SIZE:
	{
		HWND hStatus = GetDlgItem(hwnd, ID_STATUS);
		RECT rcStatus;
		SendMessage(hStatus, WM_SIZE, 0, 0);
		GetWindowRect(hStatus, &rcStatus);
		int iStatusHeight = rcStatus.bottom - rcStatus.top;

		RECT rcClient;
		HWND hEdit = GetDlgItem(hwnd, IDC_EDIT);
		GetClientRect(hwnd, &rcClient);
		int iEditHeight = rcClient.bottom - iStatusHeight;

		SetWindowPos(hEdit, NULL, 0, 0, rcClient.right - rcClient.left, iEditHeight, SWP_NOZORDER);
	}
	break;
	case WM_DROPFILES:
	{
		HWND hEdit = GetDlgItem(hwnd, IDC_EDIT);
		DragQueryFile((HDROP)wParam, 0, szFileName, MAX_PATH);
		LoadTextFileToEdit(hEdit, szFileName);
		DWORD dwTextLength = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);
		lpszFileText = (LPSTR)GlobalAlloc(GPTR, dwTextLength++);
		SendMessage(hEdit, WM_GETTEXT, dwTextLength++, (LPARAM)lpszFileText);

		HWND hStatus = GetDlgItem(hwnd, ID_STATUS);
		SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)szFileName);
		CHAR szTitle[FILENAME_MAX]{};
		sprintf(szTitle, "%s - %s", g_sz_WINDOW_CLASS, strrchr(szFileName, '\\') + 1);
		SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)szTitle);
	}
		break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_FILE_NEW:
		{
			HWND hEdit = GetDlgItem(hwnd, IDC_EDIT);
			DWORD dwTextLength = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);
			LPSTR lpszText = (LPSTR)GlobalAlloc(GPTR, dwTextLength + 1);
			SendMessage(hEdit, WM_GETTEXT, dwTextLength + 1, (LPARAM)lpszText);
			if (lpszFileText && strcmp(lpszFileText, lpszText) || lpszFileText == 0 && lpszText[0])
			{
				switch (INT answer = MessageBox(hwnd, "Сохранить изменения в файле?", "Question", MB_YESNOCANCEL | MB_ICONQUESTION))
				{
				case IDYES: SendMessage(hwnd, WM_COMMAND, ID_FILE_SAVE, 0);
				case IDNO:
					lpszFileText = NULL;
					szFileName[0] = 0;
					SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)"");
				case IDCANCEL: break;
				}
			}
		}
		break;
		case ID_FILE_OPEN:
		{
			BOOL cansel = FALSE;
			HWND hEdit = GetDlgItem(hwnd, IDC_EDIT);
			DWORD dwTextLength = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);
			LPSTR lpszText = (LPSTR)GlobalAlloc(GPTR, dwTextLength + 1);
			SendMessage(hEdit, WM_GETTEXT, dwTextLength + 1, (LPARAM)lpszText);
			if (lpszFileText && strcmp(lpszFileText, lpszText) || lpszFileText == 0 && lpszText[0])
			{
				switch (INT answer = MessageBox(hwnd, "Сохранить изменения в файле?", "Question", MB_YESNOCANCEL | MB_ICONQUESTION))
				{
				case IDYES: SendMessage(hwnd, WM_COMMAND, ID_FILE_SAVE, 0);
				case IDCANCEL: cansel = TRUE; break;
				/*case IDNO:
					lpszFileText = NULL;
					szFileName[0] = 0;
					SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)"");*/
				}
				if (cansel)break;
			}
			//CHAR szFileName[MAX_PATH]{};

			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwnd;
			ofn.lpstrFilter = "Text files: (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.lpstrDefExt = "txt";

			if (GetOpenFileName(&ofn))
			{
				HWND hEdit = GetDlgItem(hwnd, IDC_EDIT);
				LoadTextFileToEdit(hEdit, szFileName);

				DWORD dwTextLength = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);
				lpszFileText = (LPSTR)GlobalAlloc(GPTR, dwTextLength + 1);
				SendMessage(hEdit, WM_GETTEXT, dwTextLength + 1, (LPARAM)lpszFileText);

				CHAR szTitle[FILENAME_MAX]{};
				sprintf(szTitle, "%s - %s", g_sz_WINDOW_CLASS, strrchr(szFileName, '\\')+1);
				SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)szTitle);
			}
			//CONST INT SIZE = 256;
			/*CHAR sz_buffer[FILENAME_MAX] = {};
			sprintf(sz_buffer, szFileName, g_sz_WINDOW_CLASS);*/
			HWND hStatusBar = GetDlgItem(hwnd, ID_STATUS);
			SendMessage(hStatusBar, WM_SETTEXT, 0, (LPARAM)szFileName);

		}break;

		case ID_FILE_SAVE:
		{
			if (szFileName[0])
			{
				
				SaveTextFileFromEdit(GetDlgItem(hwnd, IDC_EDIT), szFileName);
				HWND hEdit = GetDlgItem(hwnd, IDC_EDIT);
				DWORD dwTextLength = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);
				lpszFileText = (LPSTR)GlobalAlloc(GPTR, dwTextLength + 1);
				SendMessage(hEdit, WM_GETTEXT, dwTextLength + 1, (LPARAM)lpszFileText);
				break;
			}

		}
		case ID_FILE_SAVEAS:
		{
			//CHAR szFileName[MAX_PATH] = {};

			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwnd;
			ofn.lpstrFilter = "Text files (*.txt)\0*.txt\0All files (*.*)\0*.*\0";	//Double NULL-Terminated line
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
			ofn.lpstrDefExt = "txt";

			if (GetSaveFileName(&ofn))
			{
				HWND hEdit = GetDlgItem(hwnd, IDC_EDIT);
				SaveTextFileFromEdit(hEdit, szFileName);
			}
			CHAR szTitle[FILENAME_MAX]{};
			sprintf(szTitle, "%s - %s", g_sz_WINDOW_CLASS, strrchr(szFileName, '\\') + 1);
			SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)szTitle);
			HWND hStatusBar = GetDlgItem(hwnd, ID_STATUS);
			SendMessage(hStatusBar, WM_SETTEXT, 0, (LPARAM)szFileName);
		}
		break;
		case ID_FILE_EXIT: SendMessage(hwnd, WM_CLOSE, 0, 0);  break;
			///////////////////////////////////////////////
		case ID_FORMAT_FONT:SelectFont(hwnd);	break;
		}
	}
	break;
	case WM_DESTROY:	PostQuitMessage(0); break;
	case WM_CLOSE:
	{
		BOOL close = FALSE;
		HWND hEdit = GetDlgItem(hwnd, IDC_EDIT);
		DWORD dwTextLength = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);
		LPSTR lpszText = (LPSTR)GlobalAlloc(GPTR, dwTextLength + 1);

		if (lpszText != NULL)
		{
			SendMessage(hEdit, WM_GETTEXT, dwTextLength + 1, (LPARAM)lpszText);
			if (lpszFileText && strcmp(lpszFileText, lpszText) || lpszFileText == NULL && lpszText[0])
			{
				switch (INT answer = MessageBox(hwnd, "Сохранить изменения в файле?", "Question", MB_YESNOCANCEL | MB_ICONQUESTION))
				{
				case IDYES: SendMessage(hwnd, WM_COMMAND, ID_FILE_SAVE, 0);
				case IDNO: close = TRUE;
				case IDCANCEL: break;
				}
			}
			else
			{
				close = TRUE;
			}
		}
		if (close)DestroyWindow(hwnd);
	}
	break;
	default:			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return NULL;
}
BOOL LoadTextFileToEdit(HWND hEdit, LPSTR lpszFileName)
{
	BOOL bSuccess = FALSE;
	HANDLE hFile = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwFileSize = GetFileSize(hFile, NULL);
		if (dwFileSize != UINT_MAX)
		{
			LPSTR lpszText = (LPSTR)GlobalAlloc(GPTR, dwFileSize + 1);
			if (lpszText)
			{

				DWORD dwRead;
				if (ReadFile(hFile, lpszText, dwFileSize, &dwRead, NULL))
				{
					lpszText[dwFileSize] = 0;
					if (SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)lpszText))bSuccess = TRUE;
				}
				GlobalFree(lpszText);
			}
		}
		CloseHandle(hFile);
	}
	return bSuccess;
}
BOOL SaveTextFileFromEdit(HWND hEdit, LPSTR lpszFileName)
{
	BOOL bSuccess = FALSE;
	HANDLE hFile = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwTextLength = GetWindowTextLength(hEdit);
		if (dwTextLength > 0)
		{
			LPSTR lpszFileText = (LPSTR)GlobalAlloc(GPTR, dwTextLength + 1);
			if (lpszFileText != NULL)
			{
				if (GetWindowText(hEdit, lpszFileText, dwTextLength + 1))
				{
					DWORD dwWritten;
					if (WriteFile(hFile, lpszFileText, dwTextLength, &dwWritten, NULL))bSuccess = TRUE;
				}
				GlobalFree(lpszFileText);
			}
		}
		CloseHandle(hFile);
	}
	return bSuccess;
}
BOOL FileWasChanger(HWND hEdit, LPSTR lpszText, LPSTR lpszFileText)
{
	BOOL fileChange = FALSE;
	DWORD dwTextLength = GetWindowTextLength(hEdit);
	if (dwTextLength == 0 && lpszText == NULL && lpszFileText == NULL)return FALSE;
	if (dwTextLength > 0)
	{
		lpszText = (LPSTR)GlobalAlloc(GPTR, dwTextLength + 1);
		if (lpszFileText == NULL)lpszFileText = (LPSTR)GlobalAlloc(GPTR, dwTextLength + 1);

		if (lpszText != lpszFileText)
		{

			if (lpszFileText)
			{
				if (strcmp(lpszText, lpszFileText))
				{ 
				fileChange = TRUE;
				}
			}
		}
	}
	return fileChange;
}

VOID SelectFont(HWND hwnd)
{
	CHOOSEFONT cf;
	LOGFONT lf;
	HWND hEdit = GetDlgItem(hwnd, IDC_EDIT);

	ZeroMemory(&cf, sizeof(cf));
	ZeroMemory(&lf, sizeof(lf));

	GetObject(g_hFont, sizeof(LOGFONT), &lf);

	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = hwnd;

	cf.Flags = CF_EFFECTS | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
	cf.hInstance = GetModuleHandle(NULL);
	cf.lpLogFont = &lf;
	cf.rgbColors = g_RGB_Text;

	if (ChooseFont(&cf))
	{
		HFONT hf = CreateFontIndirect(&lf);
		if (hf)g_hFont = hf;
		else MessageBox(hwnd, "Font creation failde", "error", MB_OK | MB_ICONERROR);

	}
	SendMessage(hEdit, WM_SETFONT, (WPARAM)g_hFont, TRUE);
	SetFocus(hEdit);
}