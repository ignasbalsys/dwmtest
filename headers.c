#define _CRT_SECURE_NO_WARNINGS 1
#include "dwmtest.h"

void PrintDate(wchar_t* str, int strLen, time_t datestamp)
{
	struct tm result;
	gmtime_s(&result, &datestamp);

	wchar_t* weekdays[] =
	{
		L"Monday",
		L"Tuesday",
		L"Wednesday",
		L"Thursday",
		L"Friday",
		L"Saturday",
		L"Sunday"
	};

	swprintf(str, strLen, L"%04d-%02d-%02d %02d:%02d:%02d (%s)", 
		result.tm_year + 1900, 
		result.tm_mon + 1, 
		result.tm_mday, 
		result.tm_hour, 
		result.tm_min, 
		result.tm_sec, 
		weekdays[result.tm_wday]);
}



void PrintMachine(wchar_t* str, int strLen, DWORD machine)
{
	for (int i = 0; i < sizeof(headerTabMachineValues) / sizeof(int); i++)
	{
		if (machine == headerTabMachineValues[i])
		{
			swprintf(str, strLen, L"%s", headerTabMachineNames[i]);

			return;
		}
	}

	swprintf(str, strLen, L"%x", machine);
}

void PrintMagic(wchar_t* str, int strLen, DWORD magic)
{
	for (int i = 0; i < sizeof(headerTabMagicValues) / sizeof(int); i++)
	{
		if (magic == headerTabMagicValues[i])
		{
			swprintf(str, strLen, L"%s", headerTabMagicNames[i]);

			return;
		}
	}

	swprintf(str, strLen, L"%x", magic);
}

void PrintSubsystem(wchar_t* str, int strLen, DWORD subsystem)
{
	for (int i = 0; i < 17; i++)
	{
		if (subsystem == i)
		{
			swprintf(str, strLen, L"%s", headerTabSubsystemNames[i]);

			return;
		}
	}

	swprintf(str, strLen, L"%u", subsystem);
}

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

int CreateCharacteristicsContextBox(wchar_t **text, int numberOfLines, HWND hwnd, POINT p)
{
	HMENU menu = CreatePopupMenu();
	HMENU* subMenus = malloc(sizeof(HMENU) * numberOfLines);

	int option = 0;

	if (menu)
	{
		for (int i = 0; i < numberOfLines; i++)
		{
			if (text[i])
			{
				subMenus[i] = CreatePopupMenu();

				if (subMenus[i])
				{
					AppendMenu(subMenus[i], MF_STRING, i*10 + 1, L"Copy mask value in hexedecimal");
					AppendMenu(subMenus[i], MF_STRING, i*10 + 2, L"Copy mask name");
				}

				AppendMenu(menu, MF_POPUP, subMenus[i], text[i]);
			}
		}

		option = TrackPopupMenu(
			menu,
			TPM_LEFTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RETURNCMD,
			p.x,
			p.y,
			0,
			hwnd,
			NULL);

		for (int i = 0; i < numberOfLines; i++)
		{
			DestroyMenu(subMenus[i]);
		}

		free(subMenus);

		DestroyMenu(menu);
	}

	return option;
}

void PrintCharacteristics(wchar_t* str[15], DWORD characteristics)
{
	int wr = 0;

	for (int i = 0; i < sizeof(headerTabCharacteristicsValues) / sizeof(int); i++)
	{
		if (characteristics & headerTabCharacteristicsValues[i])
		{
			str[i] = malloc(40 * sizeof(wchar_t));
			memset(str[i], 0, 40 * sizeof(wchar_t));
			wr = swprintf(str[i], 40, L"%s (0x%x)", headerTabCharacteristicsNames[i], headerTabCharacteristicsValues[i]);
		}
	}
}

void PrintDllCharacteristics(wchar_t* str[15], DWORD characteristics)
{
	int wr = 0;

	for (int i = 0; i < sizeof(headerTabCharacteristicsValues) / sizeof(int); i++)
	{
		if (characteristics & headerTabCharacteristicsValues[i])
		{
			str[i] = malloc(80 * sizeof(wchar_t));
			memset(str[i], 0, 80 * sizeof(wchar_t));
			wr = swprintf(str[i], 80, L"%s (0x%x)", headerTabSectionDllCharacteristicFlagNames[i], headerTabCharacteristicsValues[i]);
		}
	}
}

void FreeHeaderLVUserData(HWND lw)
{
	LVITEM item = { 0 };
	item.mask = LVIF_PARAM;
	item.iItem = 7;
	item.iSubItem = 0;

	ListView_GetItem(lw, &item);

	if (item.lParam)
	{
		free(item.lParam);
	}
}

#define HEADER_VALUE_PRINT(format, value, row) memset(str, 0, wr * sizeof(wchar_t));  wr = swprintf(str, 1024, format, value);  ListViewAddItem(l, row, 1, str); 
#define HEADER_VALUE_PRINT2(format, value, value1, row) memset(str, 0, wr * sizeof(wchar_t));  wr = swprintf(str, 1024, format, value, value1);  ListViewAddItem(l, row, 1, str); 
int FillHeaderListView(HWND l, DWORD signature, IMAGE_FILE_HEADER* file, VOID* optional, DWORD magic)
{
	wchar_t str[1024] = { 0 };

	for (int i = 0; i < sizeof(headerTabProperties) / sizeof(uintptr_t); i++)
	{
		if (i == 16 && magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		{
			continue;
		}

		ListViewAddItem(l, i, 0, headerTabProperties[i]);
	}

	int wr = 0;
	int index = 0;

	HEADER_VALUE_PRINT(L"0x%x", signature, index++);
	wchar_t machine[256] = { 0 };
	PrintMachine(machine, 256, file->Machine);

	HEADER_VALUE_PRINT(L"%s", machine, index++);
	HEADER_VALUE_PRINT(L"%d", file->NumberOfSections, index++);

	wchar_t date[256] = { 0 };
	PrintDate(date, 256, file->TimeDateStamp);

	HEADER_VALUE_PRINT(L"%s", date, index++);
	HEADER_VALUE_PRINT(L"0x%x", file->PointerToSymbolTable, index++);
	HEADER_VALUE_PRINT(L"%d", file->NumberOfSymbols, index++);
	HEADER_VALUE_PRINT2(L"0x%x (%u bytes)", file->SizeOfOptionalHeader, file->SizeOfOptionalHeader, index++);

	wchar_t** characteristics = malloc(15 * sizeof(wchar_t*));
	memset(characteristics, 0, 15 * sizeof(wchar_t*));
	PrintCharacteristics(characteristics, file->Characteristics);
	HEADER_VALUE_PRINT(L"%s", L"[ Click to list ]", index++);

	LVITEM item = { 0 };
	item.mask = LVIF_PARAM;
	item.iItem = 7;
	item.iSubItem = 0;
	item.lParam = characteristics;
	ListView_SetItem(l, &item);

	wchar_t mag[256] = { 0 };
	PrintMagic(mag, 256, magic);

	HEADER_VALUE_PRINT(L"%s", mag, index++);

	IMAGE_DATA_DIRECTORY* dataDirectory = NULL;
	if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
	{
		IMAGE_OPTIONAL_HEADER32* op = (IMAGE_OPTIONAL_HEADER32*)optional;

		HEADER_VALUE_PRINT(L"%d", op->MajorLinkerVersion, index++);
		HEADER_VALUE_PRINT(L"%d", op->MinorLinkerVersion, index++);
		HEADER_VALUE_PRINT2(L"0x%x (%u bytes)", op->SizeOfCode, op->SizeOfCode, index++);
		HEADER_VALUE_PRINT2(L"0x%x (%u bytes)", op->SizeOfInitializedData, op->SizeOfInitializedData, index++);
		HEADER_VALUE_PRINT2(L"0x%x (%u bytes)", op->SizeOfUninitializedData, op->SizeOfUninitializedData, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->AddressOfEntryPoint, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->BaseOfCode, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->BaseOfData, index++);

		HEADER_VALUE_PRINT(L"0x%x", op->ImageBase, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->SectionAlignment, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->FileAlignment, index++);
		HEADER_VALUE_PRINT(L"%d", op->MajorOperatingSystemVersion, index++);
		HEADER_VALUE_PRINT(L"%d", op->MinorOperatingSystemVersion, index++);
		HEADER_VALUE_PRINT(L"%d", op->MajorImageVersion, index++);
		HEADER_VALUE_PRINT(L"%d", op->MinorImageVersion, index++);
		HEADER_VALUE_PRINT(L"%d", op->MajorSubsystemVersion, index++);
		HEADER_VALUE_PRINT(L"%d", op->MinorSubsystemVersion, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->Win32VersionValue, index++);
		HEADER_VALUE_PRINT2(L"0x%x (%u bytes)", op->SizeOfImage, op->SizeOfImage, index++);
		HEADER_VALUE_PRINT2(L"0x%x (%u bytes)", op->SizeOfHeaders, op->SizeOfHeaders, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->CheckSum, index++);
		wchar_t subs[40] = { 0 };
		PrintSubsystem(subs, 40, op->Subsystem);

		HEADER_VALUE_PRINT(L"%s", subs, index++);

		wchar_t** dllcharacteristics = malloc(15 * sizeof(wchar_t*));
		memset(dllcharacteristics, 0, 15 * sizeof(wchar_t*));
		PrintDllCharacteristics(dllcharacteristics, op->DllCharacteristics);
		HEADER_VALUE_PRINT(L"%s", L"[ Click to list ]", index++);

		LVITEM item = { 0 };
		item.mask = LVIF_PARAM;
		item.iItem = index - 1;
		item.iSubItem = 0;
		item.lParam = dllcharacteristics;
		ListView_SetItem(l, &item);

		//HEADER_VALUE_PRINT(L"0x%x", op->DllCharacteristics, index++);
		HEADER_VALUE_PRINT2(L"0x%x (%u bytes)", op->SizeOfStackReserve, op->SizeOfStackReserve, index++);
		HEADER_VALUE_PRINT2(L"0x%x (%u bytes)", op->SizeOfStackCommit, op->SizeOfStackCommit, index++);
		HEADER_VALUE_PRINT2(L"0x%x (%u bytes)", op->SizeOfHeapReserve, op->SizeOfHeapReserve, index++);
		HEADER_VALUE_PRINT2(L"0x%x (%u bytes)", op->SizeOfHeapCommit, op->SizeOfHeapCommit, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->LoaderFlags, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->NumberOfRvaAndSizes, index++);

		dataDirectory = op->DataDirectory;
	}
	else
	{
		IMAGE_OPTIONAL_HEADER64* op = (IMAGE_OPTIONAL_HEADER64*)optional;

		HEADER_VALUE_PRINT(L"%d", op->MajorLinkerVersion, index++);
		HEADER_VALUE_PRINT(L"%d", op->MinorLinkerVersion, index++);
		HEADER_VALUE_PRINT2(L"0x%x (%u bytes)", op->SizeOfCode, op->SizeOfCode, index++);
		HEADER_VALUE_PRINT2(L"0x%x (%u bytes)", op->SizeOfInitializedData, op->SizeOfInitializedData, index++);
		HEADER_VALUE_PRINT2(L"0x%x (%u bytes)", op->SizeOfUninitializedData, op->SizeOfUninitializedData, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->AddressOfEntryPoint, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->BaseOfCode, index++);

		HEADER_VALUE_PRINT(L"0x%llx", op->ImageBase, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->SectionAlignment, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->FileAlignment, index++);
		HEADER_VALUE_PRINT(L"%u", op->MajorOperatingSystemVersion, index++);
		HEADER_VALUE_PRINT(L"%u", op->MinorOperatingSystemVersion, index++);
		HEADER_VALUE_PRINT(L"%u", op->MajorImageVersion, index++);
		HEADER_VALUE_PRINT(L"%u", op->MinorImageVersion, index++);
		HEADER_VALUE_PRINT(L"%u", op->MajorSubsystemVersion, index++);
		HEADER_VALUE_PRINT(L"%u", op->MinorSubsystemVersion, index++);
		HEADER_VALUE_PRINT(L"%u", op->Win32VersionValue, index++);
		HEADER_VALUE_PRINT2(L"0x%x (%u bytes)", op->SizeOfImage, op->SizeOfImage, index++);
		HEADER_VALUE_PRINT2(L"0x%x (%u bytes)", op->SizeOfHeaders, op->SizeOfHeaders, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->CheckSum, index++);
		wchar_t subs[40] = { 0 };
		PrintSubsystem(subs, 40, op->Subsystem);

		HEADER_VALUE_PRINT(L"%s", subs, index++);

		wchar_t** dllcharacteristics = malloc(15 * sizeof(wchar_t*));
		memset(dllcharacteristics, 0, 15 * sizeof(wchar_t*));
		PrintDllCharacteristics(dllcharacteristics, op->DllCharacteristics);
		HEADER_VALUE_PRINT(L"%s", L"[ Click to list ]", index++);

		LVITEM item = { 0 };
		item.mask = LVIF_PARAM;
		item.iItem = index - 1;
		item.iSubItem = 0;
		item.lParam = dllcharacteristics;
		ListView_SetItem(l, &item);

		//HEADER_VALUE_PRINT(L"0x%x", op->DllCharacteristics, index++);
		HEADER_VALUE_PRINT2(L"0x%llx (%u bytes)", op->SizeOfStackReserve, op->SizeOfStackReserve, index++);
		HEADER_VALUE_PRINT2(L"0x%llx (%u bytes)", op->SizeOfStackCommit, op->SizeOfStackCommit, index++);
		HEADER_VALUE_PRINT2(L"0x%llx (%u bytes)", op->SizeOfHeapReserve, op->SizeOfHeapReserve, index++);
		HEADER_VALUE_PRINT2(L"0x%llx (%u bytes)", op->SizeOfHeapCommit, op->SizeOfHeapCommit, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->LoaderFlags, index++);
		HEADER_VALUE_PRINT(L"0x%x", op->NumberOfRvaAndSizes, index++);

		dataDirectory = op->DataDirectory;
	}

	for (int i = 0; i < 15; i++)
	{
		HEADER_VALUE_PRINT2(L"0x%x (%x bytes)", dataDirectory[i].VirtualAddress, dataDirectory[i].Size, index++);
	}

	return 0;
}

void HeadersNotify(POINT cpt, POINT pt, WINDOW_DATA *data, HWND from)
{
	LVHITTESTINFO hit = { 0 };
	hit.pt = cpt;

	int itemIndex = SendMessage(from, LVM_HITTEST, 0, &hit);
	if (itemIndex != -1)
	{
		ListView_SetItemState(from, itemIndex, LVIS_SELECTED, LVIS_SELECTED);

		LVITEM item = { 0 };
		item.mask = LVIF_PARAM;
		item.iItem = itemIndex;
		item.iSubItem = 0;

		ListView_GetItem(from, &item);

		if (item.lParam)
		{

			wchar_t** characteristics = item.lParam;
			int option = CreateCharacteristicsContextBox(characteristics, 15, from, pt);
			if (!option)
			{
				return 0;
			}

			int line = option / 10;
			int lineOption = option % 10;

			if (!OpenClipboard(data->main))
			{
				return 0;
			}

			if (!EmptyClipboard())
			{
				CloseClipboard();
				return 0;
			}

			wchar_t str[40] = { 0 };
			wchar_t mask[40] = { 0 };
			wchar_t c = 0;
			swscanf(characteristics[line], L"%s (0x%[^)])", str, mask);

			int sz = 0;

			if (lineOption == 2)
			{
				sz = lstrlenW(str) + 1;
			}
			else if (lineOption == 1)
			{
				sz = lstrlenW(mask) + 1;
			}

			HGLOBAL hclipboardData = GlobalAlloc(GMEM_MOVEABLE, sz * sizeof(wchar_t));
			if (!hclipboardData)
			{
				CloseClipboard();
				return 0;
			}

			wchar_t* clipboardData = GlobalLock(hclipboardData);

			if (lineOption == 2)
			{
				memcpy(clipboardData, str, sz * sizeof(wchar_t));
			}
			else if (lineOption == 1)
			{
				memcpy(clipboardData, mask, sz * sizeof(wchar_t));
			}

			GlobalUnlock(hclipboardData);

			SetClipboardData(CF_UNICODETEXT, hclipboardData);

			CloseClipboard();


		}

	}


}