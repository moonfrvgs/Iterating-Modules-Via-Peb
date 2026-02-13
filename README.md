# Iterating-Modules-Via-Peb

```cpp
      PEB peb_local{};
       if (!ReadProcessMemory(structure::process_handle, reinterpret_cast<void*>(peb_address), &peb_local, sizeof(peb_local), nullptr)) {
           MessageBoxA(NULL, "Error 0x1", "Reading", MB_OK);
           return;
       }
```
Since we can't derefernece another process memory directly, we need to read the peb onto our local copy on our memory space


```cpp
 
        PEB_LDR_DATA ldr_local{};
        if (!ReadProcessMemory(structure::process_handle, peb_local.Ldr, &ldr_local, sizeof(ldr_local), nullptr)) {
            MessageBoxA(NULL, "Error 0x2", "Reading", MB_OK);
            return;
        }
```
We repeat the same process for the LDR located within the PEB

```cpp

        uintptr_t head_remote = reinterpret_cast<uintptr_t>(peb_local.Ldr) + offsetof(PEB_LDR_DATA, InLoadOrderModuleList);
        uintptr_t current_remote = reinterpret_cast<uintptr_t>(ldr_local.InLoadOrderModuleList.Flink);
```

*head_remote* is the address of PEB_LDR_DATA we plus the offset InLoadOrderModuleList so we can start iteration here not at the start of the structure 
*current_remote* this holds the address of the current entry but we can't do anything since it isnt in our process yet

```cpp
            uintptr_t remote_entry_addr = current_remote - offsetof(LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
```
This does the opposite from getting the offset within the structure to getting the base ( the start ) of the structure

```cpp
    std::wstring name_w(entry_local.BaseDllName.Length / sizeof(wchar_t), L'\0');``
```

Why we do Length / wchar_t is because BaseDLLName is a wide string and length isnt size of characters it's size of bytes e.g 24 bytes / wchar_t -> gives us how many characters we have etc we do L'\0' to intialise a null terminating string at the end









