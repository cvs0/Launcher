#include "sys.h"

#include <windows.h>
#include <QDebug>

#include "ntstatus/NtStatusNames.hpp"

// See: https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/ns-sysinfoapi-system_info
#ifndef PROCESSOR_ARCHITECTURE_INTEL
#define PROCESSOR_ARCHITECTURE_INTEL 0
#endif
#ifndef PROCESSOR_ARCHITECTURE_ARM
#define PROCESSOR_ARCHITECTURE_ARM 5
#endif
#ifndef PROCESSOR_ARCHITECTURE_IA64
#define PROCESSOR_ARCHITECTURE_IA64 6
#endif
#ifndef PROCESSOR_ARCHITECTURE_AMD64
#define PROCESSOR_ARCHITECTURE_AMD64 9
#endif
#ifndef PROCESSOR_ARCHITECTURE_ARM64
#define PROCESSOR_ARCHITECTURE_ARM64 12
#endif
#ifndef PROCESSOR_ARCHITECTURE_UNKNOWN
#define PROCESSOR_ARCHITECTURE_UNKNOWN 0xffff
#endif

Sys::KernelInfo Sys::getKernelInfo()
{
    Sys::KernelInfo out;
    out.kernelType = KernelType::Windows;
    out.kernelName = "Windows";
    OSVERSIONINFOW osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOW));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
    GetVersionExW(&osvi);
    out.kernelVersion = QString("%1.%2").arg(osvi.dwMajorVersion).arg(osvi.dwMinorVersion);
    out.kernelMajor = osvi.dwMajorVersion;
    out.kernelMinor = osvi.dwMinorVersion;
    out.kernelPatch = osvi.dwBuildNumber;
    return out;
}

uint64_t Sys::getSystemRam()
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx( &status );
    // bytes
    return (uint64_t)status.ullTotalPhys;
}

Sys::DistributionInfo Sys::getDistributionInfo()
{
    DistributionInfo result;
    return result;
}

bool Sys::lookupSystemStatusCode(uint64_t code, std::string &name, std::string &description)
{
    bool hasCodeName = NtStatus::lookupNtStatusCodeName(code, name);

    PSTR messageBuffer = nullptr;
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if(!ntdll)
    {
        // ???
        qWarning() << "GetModuleHandleA returned nullptr for ntdll.dll";
        return false;
    }

    auto messageSize = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
            ntdll,
            code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<PSTR>(&messageBuffer),
            0,
            nullptr
    );

    bool hasDescription = messageSize > 0;
    if(hasDescription)
    {
        description = std::string(messageBuffer, messageSize);
    }

    if(messageBuffer)
    {
        LocalFree(messageBuffer);
    }

    return hasCodeName || hasDescription;
}

Sys::Architecture Sys::systemArchitecture() {
    SYSTEM_INFO info;
    ZeroMemory(&info, sizeof(SYSTEM_INFO));
    GetNativeSystemInfo(&info);
    auto arch = info.wProcessorArchitecture;

    QString qtArch = QSysInfo::currentCpuArchitecture();
    switch (arch) {
        case PROCESSOR_ARCHITECTURE_AMD64:
        {
            return Sys::Architecture(ArchitectureType::AMD64);
        }
        case PROCESSOR_ARCHITECTURE_ARM64:
        {
            return Sys::Architecture(ArchitectureType::AARCH64);
        }
        case PROCESSOR_ARCHITECTURE_INTEL:
        {
            return Sys::Architecture(ArchitectureType::X86);
        }

        default:
            return Sys::Architecture(qtArch);
    }
}
