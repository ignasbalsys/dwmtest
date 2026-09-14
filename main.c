#define _CRT_SECURE_NO_WARNINGS 1
#include <Windows.h>
#include <dwmapi.h>
#include <uxtheme.h>

#include <phnt_windows.h>
#include <phnt.h>

#include <stdio.h>

#include "controls.h"

typedef struct _WINDOW_DATA
{
	HWND main;

	HWND headerListView;
	HWND dropNotice;
	HWND tabs;
	HWND tree;
	HTREEITEM treeRoot;

	BOOL hasBackup;
	DWORD oldColors[4];
} WINDOW_DATA, *PWINDOW_DATA;

typedef enum TreeItemType {
	Import = 1,
	Thunk32,
	Thunk64
} TreeItemType;

typedef struct _TREE_ITEM
{
	TreeItemType Type;
	void* Data;
} TREE_ITEM;


DWORD RvaToFileOffset(IMAGE_SECTION_HEADER *s, DWORD sectionCount, DWORD rva)
{
	for (int i = 0; i < sectionCount; i++)
	{
		if (s[i].VirtualAddress <= rva && rva < s[i].VirtualAddress + s[i].Misc.VirtualSize)
		{
			DWORD sectionRelAddress = rva - s[i].VirtualAddress;
			return sectionRelAddress + s[i].PointerToRawData;
		}
	}

	return 0;
}


#define ST_SUCCESS 0
#define ST_OPEN_ERROR 1
#define ST_NOT_IMAGE_ERROR 2
#define ST_PARSE_ERROR 3
#define ST_MEMORY_ERROR 4

void ListViewAddItem(HWND list, int row, int col, wchar_t* text)
{
	if (col == 0)
	{
		LVITEM lvi = { 0 };

		lvi.mask = LVIF_TEXT;
		lvi.iItem = row;
		lvi.iSubItem = 0;
		lvi.pszText = text;

		ListView_InsertItem(list, &lvi);
	}
	else 
	{
		ListView_SetItemText(list, row, col, text);
	}
}

int FillHeaderListView(HWND l, DWORD signature, IMAGE_FILE_HEADER *file, VOID *optional, DWORD magic)
{
	wchar_t str[1024] = { 0 };

	ListViewAddItem(l, 0, 0, L"Signature");

	ListViewAddItem(l, 1, 0, L"Machine");
	ListViewAddItem(l, 2, 0, L"NumberOfSections");
	ListViewAddItem(l, 3, 0, L"TimeDateStamp");
	ListViewAddItem(l, 4, 0, L"PointerToSymbolTable");
	ListViewAddItem(l, 5, 0, L"NumberOfSymbols");
	ListViewAddItem(l, 6, 0, L"SizeOfOptionalHeader");
	ListViewAddItem(l, 7, 0, L"Characteristics");

	int wr = 0;
	wr = swprintf(str, 1024, L"0x%x", signature);
	ListViewAddItem(l, 0, 1, str);

	memset(str, 0, wr);
	wr = swprintf(str, 1024, L"0x%x", file->Machine);
	ListViewAddItem(l, 1, 1, str);

	memset(str, 0, wr);
	wr = swprintf(str, 1024, L"%d", file->NumberOfSections);
	ListViewAddItem(l, 2, 1, str);
	
	memset(str, 0, wr);
	wr = swprintf(str, 1024, L"0x%x", file->TimeDateStamp);
	ListViewAddItem(l, 3, 1, str);

	memset(str, 0, wr);
	wr = swprintf(str, 1024, L"0x%x", file->PointerToSymbolTable);
	ListViewAddItem(l, 4, 1, str);

	memset(str, 0, wr);
	wr = swprintf(str, 1024, L"%d", file->NumberOfSymbols);
	ListViewAddItem(l, 5, 1, str);

	memset(str, 0, wr);
	wr = swprintf(str, 1024, L"0x%x", file->SizeOfOptionalHeader);
	ListViewAddItem(l, 6, 1, str);

	memset(str, 0, wr);
	wr = swprintf(str, 1024, L"0x%x", file->Characteristics);
	ListViewAddItem(l, 7, 1, str);
	
	return 0;
}

int ReadImports(char* filename, WINDOW_DATA *data)
{
	int status = ST_SUCCESS;
	FILE* f = NULL;
	IMAGE_DOS_HEADER dosHdr = { 0 };

	DWORD Signature = 0;
	IMAGE_FILE_HEADER FileHeader = { 0 };
	VOID* OptionalHeader = NULL;

	size_t result = 0;
	errno_t err;
	IMAGE_SECTION_HEADER* sections = NULL;


	f = fopen(filename, "rb");
	if (!f)
	{
		status = ST_OPEN_ERROR;
		goto cleanup1;
	}

	result = fread(&dosHdr, sizeof(IMAGE_DOS_HEADER), 1, f);
	if (result < 1)
	{
		status = ST_OPEN_ERROR;
		goto cleanup2;
	}

	if (dosHdr.e_magic != IMAGE_DOS_SIGNATURE)
	{
		/* Not an image */
		status = ST_NOT_IMAGE_ERROR;
		goto cleanup2;
	}

	if (fseek(f, dosHdr.e_lfanew, SEEK_SET))
	{
		/* No PE header */
		status = ST_NOT_IMAGE_ERROR;
		goto cleanup2;
	}

	if (fread(&Signature, 1, 4, f) < 4)
	{
		/* No PE header */
		status = ST_NOT_IMAGE_ERROR;
		goto cleanup2;
	}

	if (Signature != IMAGE_NT_SIGNATURE)
	{
		/* Not a PE */
		status = ST_NOT_IMAGE_ERROR;
		goto cleanup2;
	}


	if (fread(&FileHeader, sizeof(IMAGE_FILE_HEADER), 1, f) < 1)
	{
		/* No PE header */
		status = ST_NOT_IMAGE_ERROR;
		goto cleanup2;
	}

	DWORD Magic = 0;
	if (fread(&Magic, 2, 1, f) < 1)
	{
		/* No PE header */
		status = ST_NOT_IMAGE_ERROR;
		goto cleanup2;
	}

	if (fseek(f, dosHdr.e_lfanew + 4 + sizeof(IMAGE_FILE_HEADER), SEEK_SET))
	{
		/* No PE header */
		status = ST_NOT_IMAGE_ERROR;
		goto cleanup2;
	}

	if (Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
	{
		IMAGE_OPTIONAL_HEADER32 *OptionalHeader32 = malloc(sizeof(IMAGE_OPTIONAL_HEADER32));
		if (!OptionalHeader32)
		{
			status = ST_MEMORY_ERROR;
			goto cleanup2;
		}

		OptionalHeader = OptionalHeader32;

		if (fread(OptionalHeader, sizeof(IMAGE_OPTIONAL_HEADER32), 1, f) < 1)
		{
			/* No PE header */
			status = ST_NOT_IMAGE_ERROR;
			goto cleanup3;
		}


		if (fseek(f, dosHdr.e_lfanew + sizeof(IMAGE_NT_HEADERS32), SEEK_SET))
		{
			/* No PE header */
			status = ST_PARSE_ERROR;
			goto cleanup3;
		}

	}
	else if (Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		IMAGE_OPTIONAL_HEADER64 *OptionalHeader64 = malloc(sizeof(IMAGE_OPTIONAL_HEADER64));
		if (!OptionalHeader64)
		{
			status = ST_MEMORY_ERROR;
			goto cleanup2;
		}

		OptionalHeader = OptionalHeader64;

		if (fread(OptionalHeader, sizeof(IMAGE_OPTIONAL_HEADER64), 1, f) < 1)
		{
			/* No PE header */
			status = ST_NOT_IMAGE_ERROR;
			goto cleanup3;
		}

		if (fseek(f, dosHdr.e_lfanew + sizeof(IMAGE_NT_HEADERS64), SEEK_SET))
		{
			/* No PE header */
			status = ST_PARSE_ERROR;
			goto cleanup3;
		}
	}
	else
	{
		/* IMAGE_ROM_OPTIONAL_HDR_MAGIC */

		status = ST_PARSE_ERROR;
		goto cleanup2;
	}


	FillHeaderListView(data->headerListView, Signature, &FileHeader, OptionalHeader, Magic);

	// ****

	size_t sectionTableSize = FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);

	sections = malloc(sectionTableSize);
	if (!sections)
	{
		status = ST_MEMORY_ERROR;
		goto cleanup3;
	}

	DWORD importVa = 0;
	DWORD importSize = 0;
	DWORD importFileOff = 0;

	if (fread(sections, sectionTableSize, 1, f) < 1)
	{
		/* Could not read sections */
		status = ST_PARSE_ERROR;
		goto cleanup4;
	}

	if (Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
	{
		IMAGE_OPTIONAL_HEADER32* OptionalHeader32 = (IMAGE_OPTIONAL_HEADER32*)OptionalHeader;
		importVa = OptionalHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		importSize = OptionalHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
	}
	else if (Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		IMAGE_OPTIONAL_HEADER64* OptionalHeader64 = (IMAGE_OPTIONAL_HEADER64*)OptionalHeader;
		importVa = OptionalHeader64->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		importSize = OptionalHeader64->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
	}
	else
	{
		status = ST_PARSE_ERROR;
		goto cleanup4;
	}

	/* Find import table */
	importFileOff = RvaToFileOffset(sections, FileHeader.NumberOfSections, importVa);

	if (fseek(f, importFileOff, SEEK_SET))
	{
		/* No import table */
		status = ST_PARSE_ERROR;
		goto cleanup4;
	}

	IMAGE_IMPORT_DESCRIPTOR *descriptors = malloc(importSize);
	if (!descriptors)
	{
		status = ST_MEMORY_ERROR;
		goto cleanup4;
	}

	if (fread(descriptors, importSize, 1, f) < 1)
	{
		/* Could not read import descriptors */
		status = ST_PARSE_ERROR;
		goto cleanup5;
	}

	for (int i = 0; i < (importSize / sizeof(IMAGE_IMPORT_DESCRIPTOR)); i++)
	{
		if (descriptors[i].Characteristics == 0)
		{
			break;
		}

		DWORD offset = RvaToFileOffset(sections, FileHeader.NumberOfSections, descriptors[i].Name);

		char name[256] = { 0 };
		wchar_t namew[256] = { 0 };
		int ii = 0;

		if (fseek(f, offset, SEEK_SET))
		{
			status = ST_PARSE_ERROR;
			goto cleanup5;
		}

		while (ii < 256 && fread(&name[ii], 1, 1, f) == 1 && name[ii] != 0)
		{
			ii++;
		}
		
		MultiByteToWideChar(
			CP_ACP,          
			0,               
			name,
			strlen(name),              
			namew,         
			256                
		);

		IMAGE_IMPORT_DESCRIPTOR* copy = malloc(sizeof(IMAGE_IMPORT_DESCRIPTOR));
		if (!copy)
		{
			status = ST_MEMORY_ERROR;
			goto cleanup5;
		}
		memcpy(copy, &descriptors[i], sizeof(IMAGE_IMPORT_DESCRIPTOR));

		TREE_ITEM* tItem = malloc(sizeof(TREE_ITEM));
		if (!tItem)
		{
			status = ST_MEMORY_ERROR;
			free(copy);
			goto cleanup5;
		}

		tItem->Type = Import;
		tItem->Data = copy;

		TVINSERTSTRUCT imp = {0};
		imp.hParent = data->treeRoot;
		imp.hInsertAfter = TVI_FIRST;
		imp.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
		imp.item.pszText = namew;
		imp.item.cchTextMax = lstrlenW(namew);
		imp.item.state = TVIS_EXPANDED;
		imp.item.stateMask = TVIS_EXPANDED;
		imp.item.lParam = tItem;

		HTREEITEM himp = SendMessage(data->tree, TVM_INSERTITEM, 0, &imp);

		//SendMessage(list, LB_ADDSTRING, 0, (LPARAM)namew);


		DWORD iatOffset = RvaToFileOffset(sections, FileHeader.NumberOfSections, descriptors[i].OriginalFirstThunk);
		if (fseek(f, iatOffset, SEEK_SET))
		{
			status = ST_PARSE_ERROR;
			free(copy);
			free(tItem);
			goto cleanup5;
		}


		void *thunks = NULL;
		int thunkIndex = 0;
		if (Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		{
			thunks = malloc(sizeof(IMAGE_THUNK_DATA32) * 256);
			if (!thunks)
			{
				status = ST_MEMORY_ERROR;
				goto cleanup5;
			}

			while (
				thunkIndex < 256 &&
				fread(&((IMAGE_THUNK_DATA32*)thunks)[thunkIndex], sizeof(IMAGE_THUNK_DATA32), 1, f) == 1 &&
				((IMAGE_THUNK_DATA32*)thunks)[thunkIndex].u1.AddressOfData != 0
				)
			{
				IMAGE_THUNK_DATA32* copy = malloc(sizeof(IMAGE_THUNK_DATA32));
				if (!copy)
				{
					status = ST_MEMORY_ERROR;
					free(thunks);
					goto cleanup5;
				}
				memcpy(copy, &((IMAGE_THUNK_DATA32*)thunks)[thunkIndex], sizeof(IMAGE_THUNK_DATA32));

				TREE_ITEM* tItem = malloc(sizeof(TREE_ITEM));
				if (!tItem)
				{
					free(copy);
					free(thunks);
					status = ST_MEMORY_ERROR;
					goto cleanup5;
				}

				tItem->Type = Thunk32;
				tItem->Data = copy;

				if ((((IMAGE_THUNK_DATA32*)thunks)[thunkIndex].u1.Ordinal >> 31) == 1)
				{
					/* Imported by ordinal */
					int ordinal = ((IMAGE_THUNK_DATA32*)thunks)[thunkIndex].u1.AddressOfData & 0xFFFF;
					wchar_t importNameW[256] = { 0 };

					swprintf(importNameW, 256, L"- #%u", ordinal);

					TVINSERTSTRUCT name = { 0 };
					name.hParent = himp;
					name.hInsertAfter = TVI_FIRST;
					name.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
					name.item.pszText = importNameW;
					name.item.cchTextMax = lstrlenW(importNameW);
					name.item.state = TVIS_EXPANDED;
					name.item.stateMask = TVIS_EXPANDED;
					name.item.lParam = tItem;

					SendMessage(data->tree, TVM_INSERTITEM, 0, &name);

				}
				else
				{
					/* Imported by name */
					int nameRva = ((IMAGE_THUNK_DATA32*)thunks)[thunkIndex].u1.AddressOfData & 0x3FFFFFFF;
					DWORD importNameOffset = RvaToFileOffset(sections, FileHeader.NumberOfSections, nameRva) + 2; // + 2 Hint

					char importName[256] = { 0 };
					wchar_t importNameW[256] = { 0 };
					int importNameIndex = 0;

					if (fseek(f, importNameOffset, SEEK_SET))
					{
						status = ST_PARSE_ERROR;
						free(copy);
						free(thunks);
						free(tItem);
						goto cleanup5;
					}

					while (importNameIndex < 256 && fread(&importName[importNameIndex], 1, 1, f) == 1 && importName[importNameIndex] != 0)
					{
						importNameIndex++;
					}

					/* Reset file pointer for next thunk */
					if (fseek(f, iatOffset + thunkIndex * sizeof(IMAGE_THUNK_DATA32), SEEK_SET))
					{
						status = ST_PARSE_ERROR;
						free(copy);
						free(thunks);
						free(tItem);
						goto cleanup5;
					}


					MultiByteToWideChar(
						CP_ACP,
						0,
						importName,
						strlen(importName),
						importNameW,
						256
					);

					wchar_t res[256] = { 0 };
					swprintf(res, 256, L"- %s", importNameW);

					TVINSERTSTRUCT name = { 0 };
					name.hParent = himp;
					name.hInsertAfter = TVI_FIRST;
					name.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
					name.item.pszText = importNameW;
					name.item.cchTextMax = lstrlenW(importNameW);
					name.item.state = TVIS_EXPANDED;
					name.item.stateMask = TVIS_EXPANDED;
					name.item.lParam = tItem;

					SendMessage(data->tree, TVM_INSERTITEM, 0, &name);

				}

				thunkIndex++;
			}





		}
		else if (Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		{
			thunks = malloc(sizeof(IMAGE_THUNK_DATA64) * 256);
			if (!thunks)
			{
				status = ST_MEMORY_ERROR;
				goto cleanup5;
			}

			while (
				thunkIndex < 256 &&
				fread(&((IMAGE_THUNK_DATA64*)thunks)[thunkIndex], sizeof(IMAGE_THUNK_DATA64), 1, f) == 1 &&
				((IMAGE_THUNK_DATA64*)thunks)[thunkIndex].u1.AddressOfData != 0
				)
			{
				IMAGE_THUNK_DATA64* copy = malloc(sizeof(IMAGE_THUNK_DATA64));
				if (!copy)
				{
					status = ST_MEMORY_ERROR;
					free(thunks);
					goto cleanup5;
				}
				memcpy(copy, &((IMAGE_THUNK_DATA64*)thunks)[thunkIndex], sizeof(IMAGE_THUNK_DATA64));

				TREE_ITEM* tItem = malloc(sizeof(TREE_ITEM));
				if (!tItem)
				{
					free(copy);
					free(thunks);
					status = ST_MEMORY_ERROR;
					goto cleanup5;
				}

				tItem->Type = Thunk64;
				tItem->Data = copy;

				if ((((IMAGE_THUNK_DATA64*)thunks)[thunkIndex].u1.Ordinal >> 63) == 1)
				{
					/* Imported by ordinal */
					int ordinal = ((IMAGE_THUNK_DATA64*)thunks)[thunkIndex].u1.AddressOfData & 0xFFFF;
					wchar_t importNameW[256] = { 0 };

					swprintf(importNameW, 256, L"- #%u", ordinal);

					TVINSERTSTRUCT name = { 0 };
					name.hParent = himp;
					name.hInsertAfter = TVI_FIRST;
					name.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
					name.item.pszText = importNameW;
					name.item.cchTextMax = lstrlenW(importNameW);
					name.item.state = TVIS_EXPANDED;
					name.item.stateMask = TVIS_EXPANDED;
					name.item.lParam = tItem;

					SendMessage(data->tree, TVM_INSERTITEM, 0, &name);

				}
				else
				{
					/* Imported by name */
					int nameRva = ((IMAGE_THUNK_DATA64*)thunks)[thunkIndex].u1.AddressOfData & 0x3FFFFFFF;
					DWORD importNameOffset = RvaToFileOffset(sections, FileHeader.NumberOfSections, nameRva) + 2; // + 2 Hint

					char importName[256] = { 0 };
					wchar_t importNameW[256] = { 0 };
					int importNameIndex = 0;

					if (fseek(f, importNameOffset, SEEK_SET))
					{
						status = ST_PARSE_ERROR;
						free(copy);
						free(thunks);
						free(tItem);
						goto cleanup5;
					}

					while (importNameIndex < 256 && fread(&importName[importNameIndex], 1, 1, f) == 1 && importName[importNameIndex] != 0)
					{
						importNameIndex++;
					}

					/* Reset file pointer for next thunk */
					if (fseek(f, iatOffset + thunkIndex * sizeof(IMAGE_THUNK_DATA64), SEEK_SET))
					{
						status = ST_PARSE_ERROR;
						free(copy);
						free(thunks);
						free(tItem);
						goto cleanup5;
					}


					MultiByteToWideChar(
						CP_ACP,
						0,
						importName,
						strlen(importName),
						importNameW,
						256
					);

					wchar_t res[256] = { 0 };
					swprintf(res, 256, L"- %s", importNameW);

					TVINSERTSTRUCT name = { 0 };
					name.hParent = himp;
					name.hInsertAfter = TVI_FIRST;
					name.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
					name.item.pszText = importNameW;
					name.item.cchTextMax = lstrlenW(importNameW);
					name.item.state = TVIS_EXPANDED;
					name.item.stateMask = TVIS_EXPANDED;
					name.item.lParam = tItem;

					SendMessage(data->tree, TVM_INSERTITEM, 0, &name);

				}

				thunkIndex++;
			}
		}
		else
		{
			free(copy);
			free(tItem);
			goto cleanup5;
		}

		free(thunks);
	}

cleanup5:
	free(descriptors);

cleanup4:
	free(sections);

cleanup3:
	free(OptionalHeader);
cleanup2:

	fclose(f);

cleanup1:
	return status;

}

void FreeTreeViewUserData(HWND tree, HTREEITEM root)
{
	if (!root)
	{
		return;
	}

	TVITEM tvi = { 0 };
	tvi.hItem = root;
	tvi.mask = TVIF_PARAM;

	SendMessage(tree, TVM_GETITEM, 0, &tvi);

	TREE_ITEM* ti = (TREE_ITEM *)tvi.lParam;
	if (ti && ti->Data)
	{
		free(ti->Data);
	}

	if (ti)
	{
		free(ti);
	}

	while (root)
	{
		HTREEITEM child = SendMessage(tree, TVM_GETNEXTITEM, TVGN_CHILD, root);
		if (child)
		{
			FreeTreeViewUserData(tree, child);
		}

		root = SendMessage(tree, TVM_GETNEXTITEM, TVGN_NEXT, root);

	}
}

int ApplyWindowStyle(WINDOW_DATA *data)
{

	enum DWMNCRENDERINGPOLICY renderingPolicy = DWMNCRP_DISABLED;
	int result = DwmSetWindowAttribute(data->main, DWMWA_NCRENDERING_POLICY, &renderingPolicy, sizeof(enum DWMNCRENDERINGPOLICY));
	if (result)
	{
		return 1;
	}

	COLORREF colors[4] = {
			RGB(255, 0, 0),
			RGB(135, 0, 0),
			RGB(0, 255, 0),
			RGB(255,255,255)
	};

	if (!data->hasBackup)
	{
		data->oldColors[0] = GetSysColor(COLOR_ACTIVECAPTION);
		data->oldColors[1] = GetSysColor(COLOR_GRADIENTACTIVECAPTION);
		data->oldColors[2] = GetSysColor(COLOR_CAPTIONTEXT);
		data->oldColors[3] = GetSysColor(COLOR_WINDOW);
		data->hasBackup = 1;
	}


	int elements[4] = { COLOR_ACTIVECAPTION, COLOR_GRADIENTACTIVECAPTION, COLOR_CAPTIONTEXT, COLOR_WINDOW };
	if (!SetSysColors(4, elements, colors))
	{
		return 1;
	}



	RECT rect;
	GetWindowRect(data->main, &rect);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	HRGN rgn = CreateRectRgn(0, 0, width, height);

	DWM_BLURBEHIND blur = { 0 };
	blur.dwFlags = DWM_BB_ENABLE;
	blur.fEnable = 1;
	blur.hRgnBlur = NULL;
	blur.fTransitionOnMaximized = 0;

	result = DwmEnableBlurBehindWindow(data->main, &blur);
	if (result)
	{
		return 1;
	}

	SetWindowTheme(data->main, L"", L"");
	SetThemeAppProperties(0);

	return 0;
}

int ShowContextMenu(HWND hwnd, POINT p)
{
	HMENU menu = CreatePopupMenu();
	int option = 0;

	if (menu)
	{
		AppendMenu(menu, MF_STRING, 50, L"Properties");

		option = TrackPopupMenu(
			menu, 
			TPM_LEFTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RETURNCMD,
			p.x,
			p.y,
			0, 
			hwnd,
			NULL);

		DestroyMenu(menu);
	}

	return option;
}

LRESULT Wndproc(
	HWND unnamedParam1,
	UINT unnamedParam2,
	WPARAM unnamedParam3,
	LPARAM unnamedParam4
)
{
	switch (unnamedParam2)
	{
	case WM_CREATE:
	{
		WINDOW_DATA* data = malloc(sizeof(WINDOW_DATA));
		if (!data)
		{
			MessageBoxW(unnamedParam1, L"Out of system memory.", L"Error", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
			exit(1);
		}
		memset(data, 0, sizeof(WINDOW_DATA));

		data->main = unnamedParam1;

		if (ApplyWindowStyle(data))
		{
			MessageBoxW(unnamedParam1, L"Could not apply window style.", L"Error", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
			exit(1);
		}

		INITCOMMONCONTROLSEX comctl;
		comctl.dwICC = ICC_TAB_CLASSES | ICC_TREEVIEW_CLASSES | ICC_LISTVIEW_CLASSES;
		comctl.dwSize = sizeof(INITCOMMONCONTROLSEX);
		if (!InitCommonControlsEx(&comctl))
		{
			MessageBoxW(unnamedParam1, L"Cannot load common controls.", L"Error", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
			exit(1);
		}

		RECT clientRect;
		GetClientRect(unnamedParam1, &clientRect);
		data->tabs = CreateWindow(
			WC_TABCONTROL,
			NULL,
			WS_CHILD | WS_CLIPSIBLINGS,
			0,
			0,
			clientRect.right,
			clientRect.bottom,
			unnamedParam1,
			IDC_MAIN_TABS,
			GetWindowLongPtr(unnamedParam1, GWLP_HINSTANCE),
			NULL);


		TCITEM tab = { 0 };
		tab.mask = TCIF_TEXT;
		tab.pszText = L"PE header";

		SendMessage(data->tabs, TCM_INSERTITEM, 0, &tab);

		tab.pszText = L".idata";
		SendMessage(data->tabs, TCM_INSERTITEM, 1, &tab);
		
		SendMessage(data->tabs, TCM_ADJUSTRECT, FALSE, &clientRect);

		data->headerListView = CreateWindow(
			WC_LISTVIEW,
			NULL,
			WS_CHILD | WS_BORDER | LVS_REPORT, //| LVS_NOCOLUMNHEADER,
			clientRect.left,
			clientRect.top,
			clientRect.right - clientRect.left,
			clientRect.bottom - clientRect.top,
			unnamedParam1,
			IDC_HEADER_LISTVIEW,
			GetWindowLongPtr(unnamedParam1, GWLP_HINSTANCE),
			NULL);


		LVCOLUMN nameColumn = { 0 };
		nameColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		nameColumn.pszText = L"Property";
		nameColumn.cx = 150;
		ListView_InsertColumn(data->headerListView, 0, &nameColumn);

		LVCOLUMN valueColumn = { 0 };
		valueColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		valueColumn.pszText = L"Value";
		valueColumn.cx = 150;
		ListView_InsertColumn(data->headerListView, 1, &valueColumn);

		ListView_SetExtendedListViewStyle(data->headerListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

		data->dropNotice = CreateWindow(
			L"STATIC",
			L"Drop an executable file on to the window.",
			WS_CHILD | WS_VISIBLE | SS_CENTER,
			100,
			200,
			300,
			30,
			unnamedParam1,
			IDC_MAIN_LABEL,
			GetWindowLongPtr(unnamedParam1, GWLP_HINSTANCE),
			NULL);


		data->tree = CreateWindow(
			WC_TREEVIEW,
			NULL,
			WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,
			clientRect.left,
			clientRect.top,
			clientRect.right - clientRect.left,  
			clientRect.bottom - clientRect.top, 
			unnamedParam1,
			IDC_IMPORT_TREEVIEW,
			GetWindowLongPtr(unnamedParam1, GWLP_HINSTANCE),
			NULL);

		SetWindowTheme(data->tree, L"", L"");



		SetWindowLongPtr(unnamedParam1, GWLP_USERDATA, data);


		DragAcceptFiles(unnamedParam1, TRUE);

		break;
	}

	case WM_DESTROY:
	{
		WINDOW_DATA* data = GetWindowLongPtr(unnamedParam1, GWLP_USERDATA);

		if (data->hasBackup)
		{
			INT elements[4] = { COLOR_ACTIVECAPTION, COLOR_GRADIENTACTIVECAPTION, COLOR_CAPTIONTEXT, COLOR_WINDOW };
			SetSysColors(4, elements, data->oldColors);
		}

		FreeTreeViewUserData(data->tree, data->treeRoot);
		free(data);

		PostQuitMessage(0);
		return 0;
		break;
	}

	case WM_NOTIFY:
	{
		if (((LPNMHDR)unnamedParam4)->idFrom == IDC_MAIN_TABS && ((LPNMHDR)unnamedParam4)->code == TCN_SELCHANGE) {
			int tab = SendMessage(((LPNMHDR)unnamedParam4)->hwndFrom, TCM_GETCURSEL, 0, 0);

			switch (tab)
			{
			case 0:
			{
				ShowWindow(GetDlgItem(unnamedParam1, IDC_HEADER_LISTVIEW), SW_SHOW);
				ShowWindow(GetDlgItem(unnamedParam1, IDC_IMPORT_TREEVIEW), SW_HIDE);
				break;
			}
			case 1:
			{
				ShowWindow(GetDlgItem(unnamedParam1, IDC_IMPORT_TREEVIEW), SW_SHOW);
				ShowWindow(GetDlgItem(unnamedParam1, IDC_HEADER_LISTVIEW), SW_HIDE);
				break;
			}

			}
		}


		if (((LPNMHDR)unnamedParam4)->idFrom == IDC_IMPORT_TREEVIEW && ((LPNMHDR)unnamedParam4)->code == NM_RCLICK)
		{

			POINT p;
			GetCursorPos(&p);

			POINT pClient = p;
			ScreenToClient(((LPNMHDR)unnamedParam4)->hwndFrom, &pClient);


			TVHITTESTINFO hit = { 0 };
			hit.pt = pClient;
			
			HTREEITEM item = SendMessage(((LPNMHDR)unnamedParam4)->hwndFrom, TVM_HITTEST, 0, &hit);
			if (item)
			{
				SendMessage(((LPNMHDR)unnamedParam4)->hwndFrom, TVM_SELECTITEM, TVGN_CARET, item);
				int option = ShowContextMenu(unnamedParam1, p);

				if (option == 50)
				{
					/* Properties */

					wchar_t propertyName[256] = { 0 };
					TVITEM tvi = { 0 };
					tvi.mask = TVIF_PARAM | TVIF_TEXT;
					tvi.hItem = item;
					tvi.pszText = propertyName;
					tvi.cchTextMax = 256;

					if (SendMessage(((LPNMHDR)unnamedParam4)->hwndFrom, TVM_GETITEM, 0, &tvi))
					{
						TREE_ITEM* it = (TREE_ITEM*)tvi.lParam;
						if (it && it->Type == Import)
						{
							IMAGE_IMPORT_DESCRIPTOR* data = (IMAGE_IMPORT_DESCRIPTOR*)it->Data;

							wchar_t msg[1024] = { 0 };
							swprintf(msg, 1024, 
								L"Name: %x\n"
								L"Characteristics\\OriginalFirstThunk: % x\n"
								L"ForwarderChain: % x\n"
								L"FirstThunk: %x\n"
								L"TimeDateStamp: %x\n",
								data->Name, 
								data->Characteristics, 
								data->ForwarderChain, 
								data->FirstThunk,
								data->TimeDateStamp);

							wchar_t msg1[256] = { 0 };
							swprintf(msg1, 256, L"Properties for %s", propertyName);


							MessageBox(unnamedParam1, msg, msg1, MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
						}
						else if (it && it->Type == Thunk32)
						{
							IMAGE_THUNK_DATA32* data = (IMAGE_THUNK_DATA32*)it->Data;

							wchar_t msg[1024] = { 0 };
							swprintf(msg, 1024,
								L"IMAGE_THUNK_DATA32: %x\n",
								data->u1.AddressOfData);

							wchar_t msg1[256] = { 0 };
							swprintf(msg1, 256, L"Properties for %s", propertyName);

							MessageBox(unnamedParam1, msg, msg1, MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);

						}
						else if (it && it->Type == Thunk64)
						{
							IMAGE_THUNK_DATA64* data = (IMAGE_THUNK_DATA64*)it->Data;

							wchar_t msg[1024] = { 0 };
							swprintf(msg, 1024,
								L"IMAGE_THUNK_DATA64: %llx\n",
								data->u1.AddressOfData);

							wchar_t msg1[256] = { 0 };
							swprintf(msg1, 256, L"Properties for %s", propertyName);

							MessageBox(unnamedParam1, msg, msg1, MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
						}
					}


				}

			}



		}


		break;
	}

	case WM_COMMAND:
	{
#if 0
		WINDOW_DATA* data = GetWindowLongPtr(unnamedParam1, GWLP_USERDATA);

		if (HIWORD(unnamedParam3) == BN_CLICKED)
		{
			switch (LOWORD(unnamedParam3))
			{
			case 100:
				
				// future buttons
			}
		}
#endif
		break;
	}

	case WM_DROPFILES:
	{
		WINDOW_DATA* data = GetWindowLongPtr(unnamedParam1, GWLP_USERDATA);

		DWORD fileCount = DragQueryFileW((HDROP)unnamedParam3, 0xFFFFFFFF, NULL, 0);
		char file[MAX_PATH] = { 0 };

		if (fileCount > 1)
		{
			MessageBoxW(unnamedParam1, L"Only a single file is accepted.", L"Error", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);

			DragFinish((HDROP)unnamedParam3);
			break;
		}

		DWORD bytesRead = DragQueryFileA((HDROP)unnamedParam3, 0, file, MAX_PATH);
		if (!bytesRead)
		{
			MessageBoxW(unnamedParam1, L"No dropped file received", L"Error", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);

			DragFinish((HDROP)unnamedParam3);
			break;
		}


		DragFinish((HDROP)unnamedParam3);



		if (data->treeRoot)
		{
			FreeTreeViewUserData(data->tree, data->treeRoot);
			SendMessage(data->tree, TVM_DELETEITEM, 0, data->treeRoot);
		}

		TVINSERTSTRUCT root = { 0 };
		root.hParent = TVI_ROOT;
		root.hInsertAfter = TVI_FIRST;
		root.item.mask = TVIF_TEXT | TVIF_STATE;
		root.item.pszText = L".idata";
		root.item.cchTextMax = lstrlenW(L".idata");
		root.item.state = TVIS_EXPANDED;
		root.item.stateMask = TVIS_EXPANDED;

		data->treeRoot = SendMessage(data->tree, TVM_INSERTITEM, 0, &root);

		int status = ReadImports(file, data);


		if (status)
		{
			FreeTreeViewUserData(data->tree, data->treeRoot);
			SendMessage(data->tree, TVM_DELETEITEM, 0, data->treeRoot);

			switch (status)
			{
			case ST_OPEN_ERROR:
			{
				MessageBoxW(unnamedParam1, L"Error while opening the executable file.", L"Error", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
				break;
			}

			case ST_NOT_IMAGE_ERROR:
			{
				MessageBoxW(unnamedParam1, L"File is not a PE executable image.", L"Error", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
				break;
			}

			case ST_PARSE_ERROR:
			{
				MessageBoxW(unnamedParam1, L"Error while parsing PE executable image.", L"Error", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
				break;
			}
			case ST_MEMORY_ERROR:
			{
				MessageBoxW(unnamedParam1, L"Out of system memory", L"Error", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
				break;
			}
			default:
			{
				MessageBoxW(unnamedParam1, L"Invalid file parsing error code", L"Error", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
				break;
			}
			}
		} 
		else
		{
			ShowWindow(GetDlgItem(unnamedParam1, IDC_MAIN_LABEL), SW_HIDE);
			//ShowWindow(GetDlgItem(unnamedParam1, IDC_IMPORT_TREEVIEW), SW_SHOW);
			ShowWindow(GetDlgItem(unnamedParam1, IDC_HEADER_LISTVIEW), SW_SHOW);
			ShowWindow(GetDlgItem(unnamedParam1, IDC_MAIN_TABS), SW_SHOW);
		}


		break;

	}

	default:
		return DefWindowProcW(unnamedParam1, unnamedParam2, unnamedParam3, unnamedParam4);

	}

	return 0;

}


int WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd
)
{
	WNDCLASSEXW wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = Wndproc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(500));
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszMenuName = L"";
	wc.lpszClassName = L"winclass";
	wc.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(500));

	RegisterClassExW(&wc);

	int style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;

	HWND hwnd = CreateWindowW(
		L"winclass",
		L"DWM test",
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		500,
		500,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return (int)msg.wParam;

}