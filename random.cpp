    uint64_t retrieve_process_peb(_In_ void* process_permissions) {
        PROCESS_BASIC_INFORMATION pbi; NTSTATUS status = 0; DWORD return_length = 0; PEB local;
        auto nt_query_system_information_process = reinterpret_cast<NtQueryInformationProcess>(resolve_address("NtQueryInformationProcess"));
        status = nt_query_system_information_process(process_permissions, ProcessBasicInformation, &pbi, sizeof(pbi), &return_length);
        if (!NT_SUCCESS(status)) {
            message msg;
            msg.message = "Failed Getting PEB";
            msg.type = msg_type::WARNING;
            msg.primary_header = "PEB Failure";
            message_interperter(msg);
            return 0x0;
        }
        
        if (reinterpret_cast<std::uint64_t>(pbi.PebBaseAddress) != 0) {
            message msg;
            msg.message = "Recieved PEB";
            msg.type = msg_type::SUCCESS;
            msg.primary_header = "Obtained";
            message_interperter(msg);
            return (uint64_t)pbi.PebBaseAddress;
        }
    }

    void populate_module_entries(uint64_t peb_address) {
        if (!structure::process_handle) {
            MessageBoxA(NULL, "Invalid process handle", "Error", MB_OK);
            return;
        }

        PEB peb_local{};
        if (!ReadProcessMemory(structure::process_handle, reinterpret_cast<void*>(peb_address), &peb_local, sizeof(peb_local), nullptr)) {
            MessageBoxA(NULL, "Error 0x1", "Reading", MB_OK);
            return;
        }

 
        PEB_LDR_DATA ldr_local{};
        if (!ReadProcessMemory(structure::process_handle, peb_local.Ldr, &ldr_local, sizeof(ldr_local), nullptr)) {
            MessageBoxA(NULL, "Error 0x2", "Reading", MB_OK);
            return;
        }

  
        uintptr_t head_remote = reinterpret_cast<uintptr_t>(peb_local.Ldr) + offsetof(PEB_LDR_DATA, InLoadOrderModuleList);
        uintptr_t current_remote = reinterpret_cast<uintptr_t>(ldr_local.InLoadOrderModuleList.Flink);

        while (current_remote != head_remote && current_remote != 0) {
   
            LDR_DATA_TABLE_ENTRY entry_local{};
            uintptr_t remote_entry_addr = current_remote - offsetof(LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

            if (!ReadProcessMemory(structure::process_handle, reinterpret_cast<LPCVOID>(remote_entry_addr), &entry_local, sizeof(entry_local), nullptr)) {
                MessageBoxA(NULL, "Error 0x3", "Reading", MB_OK);
                break;
            }

      
            std::wstring name_w(entry_local.BaseDllName.Length / sizeof(wchar_t), L'\0');
            if (entry_local.BaseDllName.Buffer) {
                if (!ReadProcessMemory(structure::process_handle, entry_local.BaseDllName.Buffer, &name_w[0], entry_local.BaseDllName.Length, nullptr)) {
                    MessageBoxA(NULL, "Error 0x4", "Reading", MB_OK);
                }
            }

         
            std::string name_s(name_w.begin(), name_w.end());

       
            memory_entries mod{};
            mod.start = reinterpret_cast<uint64_t>(entry_local.DllBase);
            mod.stop = mod.start + entry_local.SizeOfImage;
            mod.name = name_s;
            structure::module_entries.push_back(mod);

 
            current_remote = reinterpret_cast<uintptr_t>(entry_local.InLoadOrderLinks.Flink);
        }
    }
