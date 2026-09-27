#ifndef _DWMTEST_H
#define _DWMTEST_H

#include <Windows.h>
#include <dwmapi.h>
#include <uxtheme.h>

#include <phnt_windows.h>
#include <phnt.h>

#include <stdio.h>
#include <time.h>

#include "controls.h"
#include "hdrtab.h"

typedef struct _WINDOW_DATA
{
	HWND main;
	HWND dropNotice;
	HWND tabs;

	HWND headerListView;

	HWND sectionTree;
	HTREEITEM sectionTreeRoot;

	HWND dirTree[15];
	HTREEITEM dirTreeRoot[15];

	DWORD tabIndexes[15];
	DWORD tabCount;

	RECT crect;

	BOOL hasBackup;
	DWORD oldColors[4];
} WINDOW_DATA, * PWINDOW_DATA;

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

typedef struct _SECTION_TREE_ITEM
{
	wchar_t* PropertyName;
	ULONGLONG PropertyValue;
} SECTION_TREE_ITEM;

#define ST_SUCCESS 0
#define ST_OPEN_ERROR 1
#define ST_NOT_IMAGE_ERROR 2
#define ST_PARSE_ERROR 3
#define ST_MEMORY_ERROR 4

void FreeHeaderLWUserData(HWND lw);
int CreateCharacteristicsContextBox(wchar_t** text, int numberOfLines, HWND hwnd, POINT p);
void HeadersNotify(POINT cpt, POINT pt, WINDOW_DATA* data, HWND from);
void SectionsNotify(POINT cpt, POINT pt, WINDOW_DATA* data, HWND from);

#endif /* _DWMTEST_H */