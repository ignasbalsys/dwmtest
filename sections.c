#define _CRT_SECURE_NO_WARNINGS 1
#include "dwmtest.h"



int ShowSectionContextMenu(HWND hwnd, POINT p)
{
	HMENU menu = CreatePopupMenu();
	int option = 0;

	if (menu)
	{
		AppendMenu(menu, MF_STRING, 50, L"Copy property name");
		AppendMenu(menu, MF_STRING, 51, L"Copy value as hexedecimal		(base 16)");
		AppendMenu(menu, MF_STRING, 52, L"Copy value as signed decimal	(base 10)");
		AppendMenu(menu, MF_STRING, 53, L"Copy value as decimal			(base 10)");
		AppendMenu(menu, MF_STRING, 54, L"Copy value as octal			(base 8)");

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

SECTION_TREE_ITEM* MakeSectionTreeUserDataCopy(wchar_t* name, ULONGLONG value)
{
	SECTION_TREE_ITEM* item = malloc(sizeof(SECTION_TREE_ITEM));
	if (!item)
	{
		return NULL;
	}

	int stringLen = lstrlenW(name) + 1;

	wchar_t* copy = malloc(stringLen * sizeof(wchar_t));
	if (!copy)
	{
		return NULL;
	}

	memset(copy, 0, stringLen * sizeof(wchar_t));

	lstrcpyW(copy, name);

	item->PropertyName = copy;
	item->PropertyValue = value;

	return item;
}

void FreeSectionTVUserData(HWND tree, HTREEITEM root)
{
	if (!root)
	{
		return;
	}

	TVITEM tvi = { 0 };
	tvi.hItem = root;
	tvi.mask = TVIF_PARAM;

	SendMessage(tree, TVM_GETITEM, 0, &tvi);

	SECTION_TREE_ITEM* ti = (SECTION_TREE_ITEM*)tvi.lParam;
	if (ti && ti->PropertyName)
	{
		free(ti->PropertyName);
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
			FreeSectionTVUserData(tree, child);
		}

		root = SendMessage(tree, TVM_GETNEXTITEM, TVGN_NEXT, root);

	}
}

void FillSectionTree(HWND tree, HTREEITEM root, IMAGE_SECTION_HEADER* sections, int numberOfSections)
{

	for (int i = 0; i < numberOfSections; i++)
	{
		wchar_t namew[9] = { 0 };
		char name[9] = { 0 };

		memcpy(name, sections[i].Name, 8);

		MultiByteToWideChar(
			CP_ACP,
			0,
			name,
			strlen(name),
			namew,
			8
		);

		TVINSERTSTRUCT sec = { 0 };
		sec.hParent = root;
		sec.hInsertAfter = TVI_LAST;
		sec.item.mask = TVIF_TEXT; //| TVIF_PARAM;
		sec.item.pszText = namew;
		sec.item.cchTextMax = lstrlenW(namew);

		HTREEITEM hsec = SendMessage(tree, TVM_INSERTITEM, 0, &sec);



		int wr = 0;
		wchar_t str[256] = { 0 };
		wr = swprintf(str, 256, L"VirtualSize: 0x%x", sections[i].Misc.VirtualSize);

		sec.hParent = hsec;
		sec.item.mask = TVIF_TEXT | TVIF_PARAM;

		sec.item.pszText = str;
		sec.item.cchTextMax = lstrlenW(str);
		sec.item.lParam = MakeSectionTreeUserDataCopy(L"VirtualSize", sections[i].Misc.VirtualSize);
		SendMessage(tree, TVM_INSERTITEM, 0, &sec);

		memset(str, 0, wr * sizeof(wchar_t));
		wr = swprintf(str, 256, L"VirtualAddress: 0x%x", sections[i].VirtualAddress);
		sec.item.pszText = str;
		sec.item.cchTextMax = lstrlenW(str);
		sec.item.lParam = MakeSectionTreeUserDataCopy(L"VirtualAddress", sections[i].VirtualAddress);
		SendMessage(tree, TVM_INSERTITEM, 0, &sec);

		memset(str, 0, wr * sizeof(wchar_t));
		wr = swprintf(str, 256, L"SizeOfRawData: 0x%x", sections[i].SizeOfRawData);
		sec.item.pszText = str;
		sec.item.cchTextMax = lstrlenW(str);
		sec.item.lParam = MakeSectionTreeUserDataCopy(L"SizeOfRawData", sections[i].SizeOfRawData);
		SendMessage(tree, TVM_INSERTITEM, 0, &sec);

		memset(str, 0, wr * sizeof(wchar_t));
		wr = swprintf(str, 256, L"PointerToRawData: 0x%x", sections[i].PointerToRawData);
		sec.item.pszText = str;
		sec.item.cchTextMax = lstrlenW(str);
		sec.item.lParam = MakeSectionTreeUserDataCopy(L"PointerToRawData", sections[i].PointerToRawData);
		SendMessage(tree, TVM_INSERTITEM, 0, &sec);

		memset(str, 0, wr * sizeof(wchar_t));
		wr = swprintf(str, 256, L"PointerToRelocations: 0x%x", sections[i].PointerToRelocations);
		sec.item.pszText = str;
		sec.item.cchTextMax = lstrlenW(str);
		sec.item.lParam = MakeSectionTreeUserDataCopy(L"PointerToRelocations", sections[i].PointerToRelocations);
		SendMessage(tree, TVM_INSERTITEM, 0, &sec);

		memset(str, 0, wr * sizeof(wchar_t));
		wr = swprintf(str, 256, L"PointerToLinenumbers: 0x%x", sections[i].PointerToLinenumbers);
		sec.item.pszText = str;
		sec.item.cchTextMax = lstrlenW(str);
		sec.item.lParam = MakeSectionTreeUserDataCopy(L"PointerToLinenumbers", sections[i].PointerToLinenumbers);
		SendMessage(tree, TVM_INSERTITEM, 0, &sec);

		memset(str, 0, wr * sizeof(wchar_t));
		wr = swprintf(str, 256, L"NumberOfRelocations: 0x%x", sections[i].NumberOfRelocations);
		sec.item.pszText = str;
		sec.item.cchTextMax = lstrlenW(str);
		sec.item.lParam = MakeSectionTreeUserDataCopy(L"NumberOfRelocations", sections[i].NumberOfRelocations);
		SendMessage(tree, TVM_INSERTITEM, 0, &sec);

		memset(str, 0, wr * sizeof(wchar_t));
		wr = swprintf(str, 256, L"NumberOfLinenumbers: %d", sections[i].NumberOfLinenumbers);
		sec.item.pszText = str;
		sec.item.cchTextMax = lstrlenW(str);
		sec.item.lParam = MakeSectionTreeUserDataCopy(L"NumberOfLinenumbers", sections[i].NumberOfLinenumbers);
		SendMessage(tree, TVM_INSERTITEM, 0, &sec);

		memset(str, 0, wr * sizeof(wchar_t));
		wr = swprintf(str, 256, L"Characteristics: 0x%x", sections[i].Characteristics);
		sec.item.pszText = str;
		sec.item.cchTextMax = lstrlenW(str);
		sec.item.lParam = MakeSectionTreeUserDataCopy(L"Characteristics", sections[i].Characteristics);
		HTREEITEM ch = SendMessage(tree, TVM_INSERTITEM, 0, &sec);

		sec.hParent = ch;

		for (int ii = 0; ii < 20; ii++)
		{
			unsigned int mask = 1;

			if (sections[i].Characteristics & (mask << ii))
			{
				sec.item.pszText = headerTabSectionCharacteristicFlagNames[ii];
				sec.item.cchTextMax = lstrlenW(headerTabSectionCharacteristicFlagNames[ii]);

				SendMessage(tree, TVM_INSERTITEM, 0, &sec);
			}

		}

		for (int ii = 0; ii < 14; ii++)
		{
			unsigned int mask = 0x00100000;

			if (sections[i].Characteristics & (mask + 0x100000 * ii))
			{
				sec.item.pszText = headerTabSectionCharacteristicFlagNames[ii + 20];
				sec.item.cchTextMax = lstrlenW(headerTabSectionCharacteristicFlagNames[ii + 20]);
				SendMessage(tree, TVM_INSERTITEM, 0, &sec);
			}

		}

		for (int ii = 24; ii < 32; ii++)
		{
			unsigned int mask = 1;

			if (sections[i].Characteristics & (mask << ii))
			{
				sec.item.pszText = headerTabSectionCharacteristicFlagNames[ii + 10];
				sec.item.cchTextMax = lstrlenW(headerTabSectionCharacteristicFlagNames[ii + 10]);
				SendMessage(tree, TVM_INSERTITEM, 0, &sec);
			}

		}

	}
}

void SectionsNotify(POINT cpt, POINT pt, WINDOW_DATA* data, HWND from)
{
	TVHITTESTINFO hit = { 0 };
	hit.pt = cpt;

	HTREEITEM item = SendMessage(from, TVM_HITTEST, 0, &hit);
	if (item)
	{
		SendMessage(from, TVM_SELECTITEM, TVGN_CARET, item);
		int option = ShowSectionContextMenu(data->main, pt);
		switch (option)
		{
		case 50:
		{
			/* Copy property name */

			TVITEM tvi = { 0 };
			tvi.mask = TVIF_PARAM;
			tvi.hItem = item;

			if (SendMessage(from, TVM_GETITEM, 0, &tvi))
			{
				SECTION_TREE_ITEM* it = (SECTION_TREE_ITEM*)tvi.lParam;
				if (it && it->PropertyName)
				{
					if (!OpenClipboard(data->main))
					{
						return 0;
					}

					if (!EmptyClipboard())
					{
						CloseClipboard();
						return 0;
					}

					int sz = lstrlenW(it->PropertyName) + 1;
					HGLOBAL hclipboardData = GlobalAlloc(GMEM_MOVEABLE, sz * sizeof(wchar_t));
					if (!hclipboardData)
					{
						CloseClipboard();
						return 0;
					}

					wchar_t* clipboardData = GlobalLock(hclipboardData);
					memcpy(clipboardData, it->PropertyName, sz * sizeof(wchar_t));
					GlobalUnlock(hclipboardData);

					SetClipboardData(CF_UNICODETEXT, hclipboardData);

					CloseClipboard();

				}
			}

			break;
		}

		case 51:
		case 52:
		case 53:
		{
			/* Copy value */

			wchar_t value[256] = { 0 };
			TVITEM tvi = { 0 };
			tvi.mask = TVIF_PARAM;
			tvi.hItem = item;

			if (SendMessage(from, TVM_GETITEM, 0, &tvi))
			{
				SECTION_TREE_ITEM* it = (SECTION_TREE_ITEM*)tvi.lParam;
				if (it)
				{
					if (option == 51)
					{
						swprintf(value, 256, L"%llx", it->PropertyValue);
					}
					else if (option == 52)
					{
						swprintf(value, 256, L"%lld", it->PropertyValue);
					}
					else if (option == 53)
					{
						swprintf(value, 256, L"%llu", it->PropertyValue);
					}
					else if (option == 54)
					{
						swprintf(value, 256, L"%llo", it->PropertyValue);
					}

					if (!OpenClipboard(data->main))
					{
						return 0;
					}

					if (!EmptyClipboard())
					{
						CloseClipboard();
						return 0;
					}

					int sz = lstrlenW(value) + 1;
					HGLOBAL hclipboardData = GlobalAlloc(GMEM_MOVEABLE, sz * sizeof(wchar_t));
					if (!hclipboardData)
					{
						CloseClipboard();
						return 0;
					}

					wchar_t* clipboardData = GlobalLock(hclipboardData);
					memcpy(clipboardData, value, sz * sizeof(wchar_t));
					GlobalUnlock(hclipboardData);

					SetClipboardData(CF_UNICODETEXT, hclipboardData);

					CloseClipboard();

				}
			}

			break;
		}
		}


	}
}