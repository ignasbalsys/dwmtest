#ifndef _HDRTAB_H
#define _HDRTAB_H

wchar_t* headerTabProperties[] =
{
	L"Signature",

	L"Machine",
	L"NumberOfSections",
	L"TimeDateStamp",
	L"PointerToSymbolTable",
	L"NumberOfSymbols",
	L"SizeOfOptionalHeader",
	L"Characteristics",

	L"Magic",
	L"MajorLinkerVersion",
	L"MinorLinkerVersion",
	L"SizeOfCode",
	L"SizeOfInitializedData",
	L"SizeOfUninitializedData",
	L"AddressOfEntryPoint",
	L"BaseOfCode",

	L"BaseOfData",


	L"ImageBase",
	L"SectionAlignment",
	L"FileAlignment",
	L"MajorOperatingSystemVersion",
	L"MinorOperatingSystemVersion",
	L"MajorImageVersion",
	L"MinorImageVersion",
	L"MajorSubsystemVersion",
	L"MinorSubsystemVersion",
	L"Win32VersionValue",
	L"SizeOfImage",
	L"SizeOfHeaders",
	L"CheckSum",
	L"Subsystem",
	L"DllCharacteristics",
	L"SizeOfStackReserve",
	L"SizeOfStackCommit",
	L"SizeOfHeapReserve",
	L"SizeOfHeapCommit",
	L"LoaderFlags",
	L"NumberOfRvaAndSizes",

	L"DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]",
	L"DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]",
	L"DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE]",
	L"DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION]",
	L"DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY]",
	L"DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]",
	L"DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG]",
	L"DataDirectory[IMAGE_DIRECTORY_ENTRY_ARCHITECTURE]",
	L"DataDirectory[IMAGE_DIRECTORY_ENTRY_GLOBALPTR]",
	L"DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS]",
	L"DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG]",
	L"DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT]",
	L"DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT]",
	L"DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT]",
	L"DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR]"
};



#endif /* _HDRTAB_H */