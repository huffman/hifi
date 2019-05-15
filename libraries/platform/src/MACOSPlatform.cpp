//
//  Created by Amer Cerkic 05/02/2019
//  Copyright 2019 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "MACOSPlatform.h"

#include <thread>
#include <GPUIdent.h>
#include <string>

#ifdef Q_OS_MAC
#include <unistd.h>
#include <cpuid.h>
#endif

using namespace platform;

static void getCpuId( uint32_t* p, uint32_t ax )
{
#ifdef Q_OS_MAC
    __asm __volatile
    (   "movl %%ebx, %%esi\n\t" 
     "cpuid\n\t"
     "xchgl %%ebx, %%esi"
     : "=a" (p[0]), "=S" (p[1]),
     "=c" (p[2]), "=d" (p[3])
     : "0" (ax)
     );
#endif
}

void MACOSInstance::enumerateCpu() {
    json cpu = {};
    uint32_t cpuInfo[4]={0,0,0,0};
    char CPUBrandString[16];
    char CPUModelString[16];
    char CPUClockString[16];
    uint32_t nExIds;
    getCpuId(cpuInfo, 0x80000000);
    nExIds = cpuInfo[0];
    
    for (uint32_t i = 0x80000000; i <= nExIds; ++i) {
        getCpuId(cpuInfo, i);
        // Interpret CPU brand string
        if (i == 0x80000002) {
            memcpy(CPUBrandString, cpuInfo, sizeof(cpuInfo));
        } else if (i == 0x80000003) {
            memcpy(CPUModelString, cpuInfo, sizeof(cpuInfo));
        } else if (i == 0x80000004) {
            memcpy(CPUClockString, cpuInfo, sizeof(cpuInfo));
        }
    }
 
     cpu[jsonKeys::cpuBrand] = CPUBrandString;
     cpu[jsonKeys::cpuModel] = CPUModelString;
     cpu[jsonKeys::cpuClockSpeed] = CPUClockString;
     cpu[jsonKeys::cpuNumCores] = std::thread::hardware_concurrency();

    _cpu.push_back(cpu);
}

void MACOSInstance::enumerateGpu() {
    GPUIdent* ident = GPUIdent::getInstance();
    json gpu = {};
    gpu[jsonKeys::gpuName] = ident->getName().toUtf8().constData();
    gpu[jsonKeys::gpuMemory] = ident->getMemory();
    gpu[jsonKeys::gpuDriver] = ident->getDriver().toUtf8().constData();

    _gpu.push_back(gpu);
    _display = ident->getOutput();
}

void MACOSInstance::enumerateMemory() {
    json ram = {};

#ifdef Q_OS_MAC
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    ram[jsonKeys::totalMemory] =  pages * page_size;;
#endif
    _memory.push_back(ram);
}
