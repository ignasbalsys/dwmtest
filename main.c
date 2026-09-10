#define _CRT_SECURE_NO_WARNINGS 1
#include <Windows.h>
#include <dwmapi.h>
#include <uxtheme.h>

#include <phnt_windows.h>
#include <phnt.h>

#include <stdio.h>

DWORD oldElements[4] = { 0 };
BOOL hasBackup = 0;

HWND list = 0;

#define SHARED_COUNT 112
wchar_t* shared_names[] =
{
	L"TickCountLowDeprecated",
	L"TickCountMultiplier",
	L"InterruptTime",
	L"SystemTime",
	L"TimeZoneBias",
	L"ImageNumberLow",
	L"ImageNumberHigh",
	L"NtSystemRoot",
	L"MaxStackTraceDepth",
	L"CryptoExponent",
	L"TimeZoneId",
	L"LargePageMinimum",
	L"AitSamplingValue",
	L"AppCompatFlag",
	L"RNGSeedVersion",
	L"GlobalValidationRunlevel",
	L"TimeZoneBiasStamp",
	L"NtBuildNumber",
	L"NtProductType",
	L"ProductTypeIsValid",
	L"NativeProcessorArchitecture",
	L"NtMajorVersion",
	L"NtMinorVersion",
	L"ProcessorFeatures[PF_FLOATING_POINT_PRECISION_ERRATA]",
	L"ProcessorFeatures[PF_FLOATING_POINT_EMULATED]",
	L"ProcessorFeatures[PF_COMPARE_EXCHANGE_DOUBLE]",
	L"ProcessorFeatures[PF_MMX_INSTRUCTIONS_AVAILABLE]",
	L"ProcessorFeatures[PF_PPC_MOVEMEM_64BIT_OK]",
	L"ProcessorFeatures[PF_ALPHA_BYTE_INSTRUCTIONS]",
	L"ProcessorFeatures[PF_XMMI_INSTRUCTIONS_AVAILABLE]",
	L"ProcessorFeatures[PF_3DNOW_INSTRUCTIONS_AVAILABLE]",
	L"ProcessorFeatures[PF_RDTSC_INSTRUCTION_AVAILABLE]",
	L"ProcessorFeatures[PF_PAE_ENABLED]",
	L"ProcessorFeatures[PF_XMMI64_INSTRUCTIONS_AVAILABLE]",
	L"ProcessorFeatures[PF_SSE_DAZ_MODE_AVAILABLE]",
	L"ProcessorFeatures[PF_NX_ENABLED]",
	L"ProcessorFeatures[PF_SSE3_INSTRUCTIONS_AVAILABLE]",
	L"ProcessorFeatures[PF_COMPARE_EXCHANGE128]",
	L"ProcessorFeatures[PF_COMPARE64_EXCHANGE128]",
	L"ProcessorFeatures[PF_CHANNELS_ENABLED]",
	L"ProcessorFeatures[PF_XSAVE_ENABLED]",
	L"ProcessorFeatures[PF_ARM_VFP_32_REGISTERS_AVAILABLE]",
	L"ProcessorFeatures[PF_ARM_NEON_INSTRUCTIONS_AVAILABLE]",
	L"ProcessorFeatures[PF_SECOND_LEVEL_ADDRESS_TRANSLATION]",
	L"ProcessorFeatures[PF_VIRT_FIRMWARE_ENABLED]",
	L"ProcessorFeatures[PF_RDWRFSGSBASE_AVAILABLE]",
	L"ProcessorFeatures[PF_FASTFAIL_AVAILABLE]",
	L"ProcessorFeatures[PF_ARM_DIVIDE_INSTRUCTION_AVAILABLE]",
	L"ProcessorFeatures[PF_ARM_64BIT_LOADSTORE_ATOMIC]",
	L"ProcessorFeatures[PF_ARM_EXTERNAL_CACHE_AVAILABLE]",
	L"ProcessorFeatures[PF_ARM_FMAC_INSTRUCTIONS_AVAILABLE]",
	L"ProcessorFeatures[PF_RDRAND_INSTRUCTION_AVAILABLE]",
	L"ProcessorFeatures[PF_ARM_V8_INSTRUCTIONS_AVAILABLE]",
	L"ProcessorFeatures[PF_ARM_V8_CRYPTO_INSTRUCTIONS_AVAILABLE]",
	L"ProcessorFeatures[PF_ARM_V8_CRC32_INSTRUCTIONS_AVAILABLE]",
	L"ProcessorFeatures[PF_RDTSCP_INSTRUCTION_AVAILABLE]",
	L"MaximumUserModeAddressDeprecated",
	L"SystemRangeStartDeprecated",
	L"TimeSlip",
	L"AlternativeArchitecture",
	L"BootId",

	L"SystemExpirationDate",
	L"SuiteMask",
	L"KdDebuggerEnabled",
	L"MitigationPolicies",
	L"CyclesPerYield",
	L"ActiveConsoleId",
	L"DismountCount",
	L"ComPlusPackage",
	L"LastSystemRITEventTickCount",
	L"NumberOfPhysicalPages",
	L"SafeBootMode",
	L"VirtualizationFlags",
	L"SharedDataFlags",
	L"TestRetInstruction",
	L"QpcFrequency",
	L"SystemCall",
	L"FullNumberOfPhysicalPages",
	L"TickCountQuad",
	L"Cookie",
	L"ConsoleSessionForegroundProcessId",
	L"TimeUpdateLock",
	L"BaselineSystemTimeQpc",
	L"BaselineInterruptTimeQpc",
	L"QpcSystemTimeIncrement",
	L"QpcInterruptTimeIncrement",
	L"QpcSystemTimeIncrementShift",
	L"QpcInterruptTimeIncrementShift",
	L"UnparkedProcessorCount",
	L"EnclaveFeatureMask[0]",
	L"EnclaveFeatureMask[1]",
	L"EnclaveFeatureMask[2]",
	L"EnclaveFeatureMask[3]",
	L"TelemetryCoverageRound",
	L"UserModeGlobalLogger[ETW_UMGL_INDEX_HEAP]",
	L"UserModeGlobalLogger[ETW_UMGL_INDEX_CRITSEC]",
	L"UserModeGlobalLogger[ETW_UMGL_INDEX_LDR]",
	L"UserModeGlobalLogger[ETW_UMGL_INDEX_THREAD_POOL]",
	L"UserModeGlobalLogger[ETW_UMGL_INDEX_HEAPRANGE]",
	L"UserModeGlobalLogger[ETW_UMGL_INDEX_HEAPSUMMARY]",
	L"UserModeGlobalLogger[ETW_UMGL_INDEX_UMS]",
	L"UserModeGlobalLogger[ETW_UMGL_INDEX_WNF]",
	L"ImageFileExecutionOptions",
	L"LangGenerationCount",
	L"InterruptTimeBias",
	L"QpcBias",
	L"ActiveProcessorCount",
	L"ActiveGroupCount",
	L"QpcData",
	L"XState",
	L"UserPointerAuthMask",
	L"FeatureConfigurationChangeStamp"

};

int lineCounter = 0;
wchar_t(* linesGlobal)[256];

inline void Emit32(DWORD val)
{
	swprintf_s(linesGlobal[lineCounter], 256, L"%s: %x", shared_names[lineCounter], val);
	lineCounter++;
}

inline void Emit64(ULONGLONG val)
{
	swprintf_s(linesGlobal[lineCounter], 256, L"%s: %llx", shared_names[lineCounter], val);
	lineCounter++;
}

inline void EmitStr(wchar_t *val)
{
	swprintf_s(linesGlobal[lineCounter], 256, L"%s: %s", shared_names[lineCounter], val);
	lineCounter++;
}

void ReadSharedData()
{
	wchar_t lines[SHARED_COUNT][256] = { 0 };
	linesGlobal = lines;

	lineCounter = 0;

	SendMessage(list, LB_RESETCONTENT, 0, 0);

	Emit32(USER_SHARED_DATA->TickCountLowDeprecated);
	Emit32(USER_SHARED_DATA->TickCountMultiplier);
	Emit64((((ULONGLONG)USER_SHARED_DATA->InterruptTime.High1Time) << 32) | USER_SHARED_DATA->InterruptTime.LowPart);
	Emit64((((ULONGLONG)USER_SHARED_DATA->SystemTime.High1Time) << 32) | USER_SHARED_DATA->SystemTime.LowPart);
	Emit64((((ULONGLONG)USER_SHARED_DATA->TimeZoneBias.High1Time) << 32) | USER_SHARED_DATA->TimeZoneBias.LowPart);

	Emit32(USER_SHARED_DATA->ImageNumberLow);
	Emit32(USER_SHARED_DATA->ImageNumberHigh);
	EmitStr(USER_SHARED_DATA->NtSystemRoot); // str
	Emit32(USER_SHARED_DATA->MaxStackTraceDepth);

	Emit32(USER_SHARED_DATA->CryptoExponent);
	Emit32(USER_SHARED_DATA->TimeZoneId);
	Emit32(USER_SHARED_DATA->LargePageMinimum);
	Emit32(USER_SHARED_DATA->AitSamplingValue);
	Emit32(USER_SHARED_DATA->AppCompatFlag);
	Emit64(USER_SHARED_DATA->RNGSeedVersion);
	Emit32(USER_SHARED_DATA->GlobalValidationRunlevel);
	Emit32(USER_SHARED_DATA->TimeZoneBiasStamp);
	Emit32(USER_SHARED_DATA->NtBuildNumber);
	EmitStr(USER_SHARED_DATA->NtProductType == 1 ? L"NtProductWinNt" : (USER_SHARED_DATA->NtProductType == 2 ? L"NtProductLanManNT" : L"NtProductServer")); // str
	Emit32(USER_SHARED_DATA->ProductTypeIsValid);
	Emit32(USER_SHARED_DATA->NativeProcessorArchitecture);
	Emit32(USER_SHARED_DATA->NtMajorVersion);
	Emit32(USER_SHARED_DATA->NtMinorVersion);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[0]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[1]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[2]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[3]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[4]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[5]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[6]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[7]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[8]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[9]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[10]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[11]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[12]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[13]);

	Emit32(USER_SHARED_DATA->ProcessorFeatures[14]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[15]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[16]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[17]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[18]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[19]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[20]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[21]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[22]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[23]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[24]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[25]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[26]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[27]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[28]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[29]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[30]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[31]);
	Emit32(USER_SHARED_DATA->ProcessorFeatures[32]);
	Emit32(USER_SHARED_DATA->MaximumUserModeAddressDeprecated);
	Emit32(USER_SHARED_DATA->SystemRangeStartDeprecated);
	Emit32(USER_SHARED_DATA->TimeSlip);
	EmitStr(USER_SHARED_DATA->AlternativeArchitecture == 0 ? L"StandardDesign" : (USER_SHARED_DATA->AlternativeArchitecture == 1 ? L"NEC98x86" : L"EndAlternatives")); // str
	Emit32(USER_SHARED_DATA->BootId);

	Emit64(USER_SHARED_DATA->SystemExpirationDate.QuadPart);
	Emit32(USER_SHARED_DATA->SuiteMask);
	Emit32(USER_SHARED_DATA->KdDebuggerEnabled);

	Emit32(USER_SHARED_DATA->MitigationPolicies);
	Emit32(USER_SHARED_DATA->CyclesPerYield);
	Emit32(USER_SHARED_DATA->ActiveConsoleId);
	Emit32(USER_SHARED_DATA->DismountCount);
	Emit32(USER_SHARED_DATA->ComPlusPackage);
	Emit32(USER_SHARED_DATA->LastSystemRITEventTickCount);
	Emit32(USER_SHARED_DATA->NumberOfPhysicalPages);
	Emit32(USER_SHARED_DATA->SafeBootMode);
	Emit32(USER_SHARED_DATA->VirtualizationFlags);
	Emit32(USER_SHARED_DATA->SharedDataFlags);
	Emit64(USER_SHARED_DATA->TestRetInstruction);
	Emit64(USER_SHARED_DATA->QpcFrequency);
	Emit32(USER_SHARED_DATA->SystemCall);
	Emit64(USER_SHARED_DATA->FullNumberOfPhysicalPages);
	Emit64(USER_SHARED_DATA->TickCountQuad);
	Emit32(USER_SHARED_DATA->Cookie);
	Emit64(USER_SHARED_DATA->ConsoleSessionForegroundProcessId);
	Emit64(USER_SHARED_DATA->TimeUpdateLock);
	Emit64(USER_SHARED_DATA->BaselineSystemTimeQpc);
	Emit64(USER_SHARED_DATA->BaselineInterruptTimeQpc);
	Emit64(USER_SHARED_DATA->QpcSystemTimeIncrement);
	Emit64(USER_SHARED_DATA->QpcInterruptTimeIncrement);

	Emit32(USER_SHARED_DATA->QpcSystemTimeIncrementShift);
	Emit32(USER_SHARED_DATA->QpcInterruptTimeIncrementShift);
	Emit32(USER_SHARED_DATA->UnparkedProcessorCount);
	Emit32(USER_SHARED_DATA->EnclaveFeatureMask[0]);
	Emit32(USER_SHARED_DATA->EnclaveFeatureMask[1]);
	Emit32(USER_SHARED_DATA->EnclaveFeatureMask[2]);
	Emit32(USER_SHARED_DATA->EnclaveFeatureMask[3]);
	Emit32(USER_SHARED_DATA->TelemetryCoverageRound);
	Emit32(USER_SHARED_DATA->UserModeGlobalLogger[ETW_UMGL_INDEX_HEAP]);
	Emit32(USER_SHARED_DATA->UserModeGlobalLogger[ETW_UMGL_INDEX_CRITSEC]);
	Emit32(USER_SHARED_DATA->UserModeGlobalLogger[ETW_UMGL_INDEX_LDR]);
	Emit32(USER_SHARED_DATA->UserModeGlobalLogger[ETW_UMGL_INDEX_THREAD_POOL]);
	Emit32(USER_SHARED_DATA->UserModeGlobalLogger[ETW_UMGL_INDEX_HEAPRANGE]);
	Emit32(USER_SHARED_DATA->UserModeGlobalLogger[ETW_UMGL_INDEX_HEAPSUMMARY]);
	Emit32(USER_SHARED_DATA->UserModeGlobalLogger[ETW_UMGL_INDEX_UMS]);
	Emit32(USER_SHARED_DATA->UserModeGlobalLogger[ETW_UMGL_INDEX_WNF]);

	Emit32(USER_SHARED_DATA->ImageFileExecutionOptions);
	Emit32(USER_SHARED_DATA->LangGenerationCount);
	Emit64(USER_SHARED_DATA->InterruptTimeBias);
	Emit64(USER_SHARED_DATA->QpcBias);
	Emit32(USER_SHARED_DATA->ActiveProcessorCount);
	Emit32(USER_SHARED_DATA->ActiveGroupCount);
	Emit32(USER_SHARED_DATA->QpcData);

	EmitStr(L"-");
	Emit64(USER_SHARED_DATA->UserPointerAuthMask);
	Emit64((((ULONGLONG)USER_SHARED_DATA->FeatureConfigurationChangeStamp.High1Time) << 32) | USER_SHARED_DATA->FeatureConfigurationChangeStamp.LowPart);


	for (int i = 0; i < SHARED_COUNT; i++)
	{
		SendMessage(list, LB_ADDSTRING, 0, (LPARAM)lines[i]);
	}

}

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


VOID ReadImports(char* filename)
{
	FILE* f = NULL;
	IMAGE_DOS_HEADER dosHdr = { 0 };
	IMAGE_NT_HEADERS ntHdr = { 0 };
	size_t result = 0;
	errno_t err;
	IMAGE_SECTION_HEADER* sections = NULL;


	f = fopen(filename, "rb");
	if (!f)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		int a = errno;
		DWORD winerr = GetLastError();
		DebugBreak();
	}

	result = fread(&dosHdr, 1, sizeof(IMAGE_DOS_HEADER), f);
	if (result < sizeof(IMAGE_DOS_HEADER))
	{
		DebugBreak();
	}

	if (dosHdr.e_magic != IMAGE_DOS_SIGNATURE)
	{
		/* Not an image */
		DebugBreak();
	}

	if (fseek(f, dosHdr.e_lfanew, SEEK_SET))
	{
		/* No PE header */
		DebugBreak();
	}

	if (fread(&ntHdr, 1, sizeof(IMAGE_NT_HEADERS), f) < sizeof(IMAGE_NT_HEADERS))
	{
		/* No PE header */
		DebugBreak();
	}

	if (ntHdr.Signature != IMAGE_NT_SIGNATURE)
	{
		/* Not a PE */
		DebugBreak();
	}


	if (fseek(f, dosHdr.e_lfanew + sizeof(IMAGE_NT_HEADERS), SEEK_SET))
	{
		/* No PE header */
		DebugBreak();
	}

	size_t sectionTableSize = ntHdr.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);

	sections = malloc(sectionTableSize);

	if (fread(sections, 1, sectionTableSize, f) < sectionTableSize)
	{
		/* Could not read sections */
		DebugBreak();
	}

	DWORD importVa = ntHdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	DWORD importSize = ntHdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
	DWORD importFileOff = 0;

	/* Find import table */
	for (int i = 0; i < ntHdr.FileHeader.NumberOfSections; i++)
	{
		if (sections[i].VirtualAddress <= importVa && importVa < sections[i].VirtualAddress + sections[i].Misc.VirtualSize)
		{
			DWORD sectionRelAddress = importVa - sections[i].VirtualAddress;
			importFileOff = sectionRelAddress + sections[i].PointerToRawData;
			break;
		}
	}

	if (fseek(f, importFileOff, SEEK_SET))
	{
		/* No import table */
		DebugBreak();
	}

	IMAGE_IMPORT_DESCRIPTOR *descriptors = malloc(importSize);

	if (fread(descriptors, 1, importSize, f) < importSize)
	{
		/* Could not read import descriptors */
		DebugBreak();
	}

	for (int i = 0; i < (importSize / 0x14); i++)
	{
		if (descriptors[i].Characteristics == 0)
		{
			break;
		}

		DWORD offset = RvaToFileOffset(sections, ntHdr.FileHeader.NumberOfSections, descriptors[i].Name);

		char name[256] = { 0 };
		wchar_t namew[256] = { 0 };
		int ii = 0;

		fseek(f, offset, SEEK_SET);
		while (1)
		{
			if (fread(&name[ii], 1, 1, f) != 1) break;

			if (name[ii] == 0) break;

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

		SendMessage(list, LB_ADDSTRING, 0, (LPARAM)namew);

	}

	free(descriptors);
	free(sections);
	fclose(f);

}

HRESULT UpdateDesktop(HWND hwnd)
{
	HRESULT result = 0;
	HTHUMBNAIL thumbnail = NULL;

	result = DwmRegisterThumbnail(hwnd, FindWindow(L"Progman", NULL), &thumbnail);
	if (SUCCEEDED(result))
	{

		RECT dest = { 0 };
		GetClientRect(hwnd, &dest);

		dest.top += 26;
		dest.left += 3;
		dest.right += 3;
		dest.bottom += 26;

		DWM_THUMBNAIL_PROPERTIES properties = { 0 };
		properties.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_VISIBLE | DWM_TNP_SOURCECLIENTAREAONLY;
		properties.fSourceClientAreaOnly = FALSE;
		properties.fVisible = TRUE;
		properties.opacity = (255 * 70) / 100;
		properties.rcDestination = dest;

		result = DwmUpdateThumbnailProperties(thumbnail, &properties);
		if (FAILED(result))
		{
			DebugBreak();
		}
	}
	return result;
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
	case WM_DESTROY:
	{
		if (hasBackup)
		{
			INT elements[4] = { COLOR_ACTIVECAPTION, COLOR_GRADIENTACTIVECAPTION, COLOR_CAPTIONTEXT, COLOR_WINDOW };
			SetSysColors(4, elements, oldElements);
		}

		PostQuitMessage(0);
		return 0;
		break;
	}

	case WM_COMMAND:
	{
		if (HIWORD(unnamedParam3) == BN_CLICKED)
		{
			switch (LOWORD(unnamedParam3))
			{
			case 100:
				
				ReadSharedData(list);
			}
		}

		break;
	}

	case WM_NCPAINT:
	case WM_NCACTIVATE:
	{	
		return DefWindowProcW(unnamedParam1, unnamedParam2, unnamedParam3, unnamedParam4);
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
	WNDCLASSEXW wc;
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

	enum DWMNCRENDERINGPOLICY renderingPolicy = DWMNCRP_DISABLED;

	int result = DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &renderingPolicy, sizeof(enum DWMNCRENDERINGPOLICY));
	if (result)
	{
		DebugBreak();
	}

	BOOL rtlLayout = TRUE;
	result = DwmSetWindowAttribute(hwnd, DWMWA_NONCLIENT_RTL_LAYOUT, &rtlLayout, sizeof(BOOL));
	if (result)
	{
		DebugBreak();
	}

	COLORREF colors[4] = {
				RGB(255, 0, 0),   
				RGB(135, 0, 0),    
				RGB(0, 255, 0), 
				RGB(0,0,0)
	};

	if (!hasBackup)
	{
		oldElements[0] = GetSysColor(COLOR_ACTIVECAPTION);
		oldElements[1] = GetSysColor(COLOR_GRADIENTACTIVECAPTION);
		oldElements[2] = GetSysColor(COLOR_CAPTIONTEXT);
		oldElements[3] = GetSysColor(COLOR_WINDOW);
		hasBackup = 1;
	}


	int elements[4] = { COLOR_ACTIVECAPTION, COLOR_GRADIENTACTIVECAPTION, COLOR_CAPTIONTEXT, COLOR_WINDOW };
	SetSysColors(3, elements, colors);

	RECT rect;
	GetWindowRect(hwnd, &rect);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	HRGN rgn = CreateRectRgn(0, 0, width, height);

	DWM_BLURBEHIND blur = { 0 };
	blur.dwFlags = DWM_BB_ENABLE;
	blur.fEnable = 1;
	blur.hRgnBlur = NULL;
	blur.fTransitionOnMaximized = 0;

	result = DwmEnableBlurBehindWindow(hwnd, &blur);
	if (result)
	{
		DebugBreak();
	}

	SetWindowTheme(hwnd, L"", L"");
	SetThemeAppProperties(0);
	
	INITCOMMONCONTROLSEX comctl;
	comctl.dwICC = ICC_TAB_CLASSES;
	comctl.dwSize = sizeof(INITCOMMONCONTROLSEX);

	if (!InitCommonControlsEx(&comctl))
	{
		DebugBreak();
	}


	HWND button = CreateWindow(
		L"BUTTON",  
		L"Read KUSER_SHARED_DATA",      
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  
		10,         // x 
		10,         // y 
		200,        //  width
		30,        //  height
		hwnd,     
		(HMENU)100,   
		hInstance,
		NULL);      

	list = CreateWindow(
		L"LISTBOX",
		NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL,
		10,50,460,330,
		hwnd,
		(HMENU)101,
		hInstance,
		NULL
	);

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);

	HWND tab = CreateWindow(
		WC_TABCONTROL,
		NULL,
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
		0,
		0,
		clientRect.right,
		clientRect.bottom,
		hwnd,
		NULL,
		hInstance,
		NULL);


	ReadImports("dwmtest.exe");

	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);

	MSG msg;

	while (GetMessageW(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return (int)msg.wParam;

}