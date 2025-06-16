#pragma once
#include <QString>
#include <QRegExp>

namespace Sys
{
const uint64_t mebibyte = 1024ull * 1024ull;

enum class KernelType {
    Undetermined,
    Windows,
    Darwin,
    Linux
};

enum class ArchitectureType {
    Undetermined,
    X86,
    AMD64,
    AARCH64,
    Unknown
};

class Architecture {
public:
    // Undetermined architecturte
    Architecture() = default;
    // Specific valid and supported architecture
    Architecture(ArchitectureType _type)
    {
        type = _type;
        switch(type)
        {
            case ArchitectureType::X86:
                raw = "x86";
                break;
            case ArchitectureType::AMD64:
                raw = "amd64";
                break;
            case ArchitectureType::AARCH64:
                raw = "aarch64";
                break;
            default:
                break;
        }
    }
    // Invalid architecture we don't know or understand
    Architecture(const QString& _raw)
    {
        type = ArchitectureType::Unknown;
        raw = _raw;
    }

    // FIXME: use spaceship operator when we can use C++20
    bool operator == (const Architecture& rhs) const
    {
        return type == rhs.type;
    }
    bool operator != (const Architecture& rhs) const
    {
        return type != rhs.type;
    }
    bool operator < (const Architecture& rhs) const
    {
        return type < rhs.type;
    }
    bool operator > (const Architecture& rhs) const
    {
        return type > rhs.type;
    }
    QString getRaw() const
    {
        return raw;
    }

    QString serialize() const
    {
        switch(type)
        {
            default:
            case ArchitectureType::Undetermined:
                return "null";
            case ArchitectureType::X86:
                return "x86";
            case ArchitectureType::AMD64:
                return "amd64";
            case ArchitectureType::AARCH64:
                return "aarch64";
            case ArchitectureType::Unknown:
                return QString("Unknown(%1)").arg(raw);
        }
    }
    bool is32() const
    {
        switch(type)
        {
            case ArchitectureType::X86:
                return true;
            default:
                return false;
        }
    }
    bool is64() const
    {
        switch(type)
        {
            case ArchitectureType::AMD64:
            case ArchitectureType::AARCH64:
                return true;
            default:
                return false;
        }
    }
    static Architecture deserialize(const QString& arch_str)
    {
        QRegExp unknown("^Unknown\\((.+)\\)$");
        if(arch_str == "null")
        {
            return Architecture();
        }
        else if(arch_str == "x86")
        {
            return Architecture(ArchitectureType::X86);
        }
        else if(arch_str == "amd64")
        {
            return Architecture(ArchitectureType::AMD64);
        }
        else if(arch_str == "aarch64")
        {
            return Architecture(ArchitectureType::AARCH64);
        }
        else
        {
            return Architecture(arch_str);
        }
    }
    static Architecture fromOSArch(const QString& os_arch)
    {
        Architecture arch;
        arch.raw = os_arch;

        if(os_arch == "x86_64" || os_arch == "amd64")
        {
            arch.type = Sys::ArchitectureType::AMD64;
        }
        else if(os_arch == "x86" || os_arch == "i386" || os_arch == "i486" || os_arch == "i586" || os_arch == "i686")
        {
            arch.type = Sys::ArchitectureType::X86;
        }
        else if(os_arch == "aarch64" || os_arch == "arm64")
        {
            arch.type = Sys::ArchitectureType::AARCH64;
        }
        else
        {
            arch.type = Sys::ArchitectureType::Unknown;
        }
        return arch;
    }

private:
    ArchitectureType type = ArchitectureType::Undetermined;
    QString raw;
};

struct KernelInfo
{
    QString kernelName;
    QString kernelVersion;

    KernelType kernelType = KernelType::Undetermined;
    int kernelMajor = 0;
    int kernelMinor = 0;
    int kernelPatch = 0;
    bool isCursed = false;
};

KernelInfo getKernelInfo();

struct DistributionInfo
{
    DistributionInfo operator+(const DistributionInfo& rhs) const
    {
        DistributionInfo out;
        if(!distributionName.isEmpty())
        {
            out.distributionName = distributionName;
        }
        else
        {
            out.distributionName = rhs.distributionName;
        }
        if(!distributionVersion.isEmpty())
        {
            out.distributionVersion = distributionVersion;
        }
        else
        {
            out.distributionVersion = rhs.distributionVersion;
        }
        return out;
    }
    QString distributionName;
    QString distributionVersion;
};

DistributionInfo getDistributionInfo();

uint64_t getSystemRam();

Architecture systemArchitecture();

bool lookupSystemStatusCode(uint64_t code, std::string &name, std::string &description);
}
