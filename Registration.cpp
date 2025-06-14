/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015 Charles J. Cliffe

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "SoapyHydraSDR.hpp"
#include <SoapySDR/Registry.hpp>
#include <cstdlib> //malloc
#include <algorithm>

static std::vector<SoapySDR::Kwargs> findHydraSDR(const SoapySDR::Kwargs &args)
{
    std::vector<SoapySDR::Kwargs> results;
    
    hydrasdr_lib_version_t asVersion;
    hydrasdr_lib_version(&asVersion);
    
    SoapySDR_logf(SOAPY_SDR_DEBUG, "HydraSDR Lib v%d.%d rev %d", asVersion.major_version, asVersion.minor_version, asVersion.revision);

    uint64_t serials[MAX_DEVICES];
    int count = hydrasdr_list_devices(serials, MAX_DEVICES);
    if (count < 0) {
        SoapySDR_logf(SOAPY_SDR_ERROR, "libhydrasdr error listing devices");
        return results;
    }

    SoapySDR_logf(SOAPY_SDR_DEBUG, "%d HydraSDR boards found.", count);
    
    for (int i = 0; i < count; i++) {
        std::stringstream serialstr;
        
        serialstr.str("");
        serialstr << std::hex << serials[i];
        
        SoapySDR_logf(SOAPY_SDR_DEBUG, "Serial %s", serialstr.str().c_str());        

        SoapySDR::Kwargs soapyInfo;

        soapyInfo["label"] = "HydraSDR RFOne [" + serialstr.str() + "]";
        soapyInfo["serial"] = serialstr.str();

        if (args.count("serial") != 0) {
            if (args.at("serial") != soapyInfo.at("serial")) {
                continue;
            }
            SoapySDR_logf(SOAPY_SDR_DEBUG, "Found device by serial %s", soapyInfo.at("serial").c_str());
        }

        results.push_back(soapyInfo);
    }
    return results;
}

static SoapySDR::Device *makeHydraSDR(const SoapySDR::Kwargs &args)
{
    return new SoapyHydraSDR(args);
}

static SoapySDR::Registry registerHydraSDR("hydrasdr", &findHydraSDR, &makeHydraSDR, SOAPY_SDR_ABI_VERSION);
