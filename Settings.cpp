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
#include <cinttypes>

SoapyHydraSDR::SoapyHydraSDR(const SoapySDR::Kwargs &args)
{
    sampleRate = 3000000;
    centerFrequency = 0;

    numBuffers = DEFAULT_NUM_BUFFERS;

    agcMode = false;
    rfBias = false;
    bitPack = false;

    bufferedElems = 0;
    resetBuffer = false;

    streamActive = false;
    sampleRateChanged.store(false);

    dev = nullptr;

    lnaGain = mixerGain = vgaGain = 0;

    dev = nullptr;
    std::stringstream serialstr;
    serialstr.str("");

    if (args.count("serial") != 0)
    {
        try {
            serial = std::stoull(args.at("serial"), nullptr, 16);
        } catch (const std::invalid_argument &) {
            throw std::runtime_error("serial is not a hex number");
        } catch (const std::out_of_range &) {
            throw std::runtime_error("serial value of out range");
        }
        serialstr << std::hex << serial;
        if (hydrasdr_open_sn(&dev, serial) != HYDRASDR_SUCCESS) {
            throw std::runtime_error("Unable to open HydraSDR device with serial " + serialstr.str());
        }
        SoapySDR_logf(SOAPY_SDR_DEBUG, "Found HydraSDR device: serial = %" PRIx64, serial);
    }
    else
    {
        if (hydrasdr_open(&dev) != HYDRASDR_SUCCESS) {
            throw std::runtime_error("Unable to open HydraSDR device");
        }
    }

    //apply arguments to settings when they match
    for (const auto &info : this->getSettingInfo())
    {
        const auto it = args.find(info.key);
        if (it != args.end()) this->writeSetting(it->first, it->second);
    }
}

SoapyHydraSDR::~SoapyHydraSDR(void)
{
    hydrasdr_close(dev);
}

/*******************************************************************
 * Identification API
 ******************************************************************/

std::string SoapyHydraSDR::getDriverKey(void) const
{
    return "HydraSDR";
}

std::string SoapyHydraSDR::getHardwareKey(void) const
{
    return "HydraSDR";
}

SoapySDR::Kwargs SoapyHydraSDR::getHardwareInfo(void) const
{
    //key/value pairs for any useful information
    //this also gets printed in --probe
    SoapySDR::Kwargs args;

    std::stringstream serialstr;
    serialstr.str("");
    serialstr << std::hex << serial;
    args["serial"] = serialstr.str();

    return args;
}

/*******************************************************************
 * Channels API
 ******************************************************************/

size_t SoapyHydraSDR::getNumChannels(const int dir) const
{
    return (dir == SOAPY_SDR_RX) ? 1 : 0;
}

/*******************************************************************
 * Antenna API
 ******************************************************************/

std::vector<std::string> SoapyHydraSDR::listAntennas(const int direction, const size_t channel) const
{
    std::vector<std::string> antennas;
    antennas.push_back("RX");
    return antennas;
}

void SoapyHydraSDR::setAntenna(const int direction, const size_t channel, const std::string &name)
{
    // TODO
}

std::string SoapyHydraSDR::getAntenna(const int direction, const size_t channel) const
{
    return "RX";
}

/*******************************************************************
 * Frontend corrections API
 ******************************************************************/

bool SoapyHydraSDR::hasDCOffsetMode(const int direction, const size_t channel) const
{
    return false;
}

/*******************************************************************
 * Gain API
 ******************************************************************/

std::vector<std::string> SoapyHydraSDR::listGains(const int direction, const size_t channel) const
{
    //list available gain elements,
    //the functions below have a "name" parameter
    std::vector<std::string> results;

    results.push_back("LNA");
    results.push_back("MIX");
    results.push_back("VGA");

    return results;
}

bool SoapyHydraSDR::hasGainMode(const int direction, const size_t channel) const
{
    return true;
}

void SoapyHydraSDR::setGainMode(const int direction, const size_t channel, const bool automatic)
{
    agcMode = automatic;

    hydrasdr_set_lna_agc(dev, agcMode?1:0);
    hydrasdr_set_mixer_agc(dev, agcMode?1:0);

    SoapySDR_logf(SOAPY_SDR_DEBUG, "Setting AGC: %s", automatic ? "Automatic" : "Manual");
}

bool SoapyHydraSDR::getGainMode(const int direction, const size_t channel) const
{
    return agcMode;
}

void SoapyHydraSDR::setGain(const int direction, const size_t channel, const double value)
{
    //set the overall gain by distributing it across available gain elements
    //OR delete this function to use SoapySDR's default gain distribution algorithm...
    SoapySDR::Device::setGain(direction, channel, value);
}

void SoapyHydraSDR::setGain(const int direction, const size_t channel, const std::string &name, const double value)
{
    if (name == "LNA")
    {
        lnaGain = uint8_t(value);
        hydrasdr_set_lna_gain(dev, lnaGain);
    }
    else if (name == "MIX")
    {
        mixerGain = uint8_t(value);
        hydrasdr_set_mixer_gain(dev, mixerGain);
    }
    else if (name == "VGA")
    {
        vgaGain = uint8_t(value);
        hydrasdr_set_vga_gain(dev, vgaGain);
    }
}

double SoapyHydraSDR::getGain(const int direction, const size_t channel, const std::string &name) const
{
    if (name == "LNA")
    {
        return lnaGain;
    }
    else if (name == "MIX")
    {
        return mixerGain;
    }
    else if (name == "VGA")
    {
        return vgaGain;
    }

    return 0;
}

SoapySDR::Range SoapyHydraSDR::getGainRange(const int direction, const size_t channel, const std::string &name) const
{
    if (name == "LNA" || name == "MIX" || name == "VGA") {
        return SoapySDR::Range(0, 15);
    }

    return SoapySDR::Range(0, 15);
}

/*******************************************************************
 * Frequency API
 ******************************************************************/

void SoapyHydraSDR::setFrequency(
        const int direction,
        const size_t channel,
        const std::string &name,
        const double frequency,
        const SoapySDR::Kwargs &args)
{
    if (name == "RF")
    {
        centerFrequency = (uint32_t) frequency;
        resetBuffer = true;
        SoapySDR_logf(SOAPY_SDR_DEBUG, "Setting center freq: %d", centerFrequency);
        hydrasdr_set_freq(dev, centerFrequency);
    }
}

double SoapyHydraSDR::getFrequency(const int direction, const size_t channel, const std::string &name) const
{
    if (name == "RF")
    {
        return (double) centerFrequency;
    }

    return 0;
}

std::vector<std::string> SoapyHydraSDR::listFrequencies(const int direction, const size_t channel) const
{
    std::vector<std::string> names;
    names.push_back("RF");
    return names;
}

SoapySDR::RangeList SoapyHydraSDR::getFrequencyRange(
        const int direction,
        const size_t channel,
        const std::string &name) const
{
    SoapySDR::RangeList results;
    if (name == "RF")
    {
        results.push_back(SoapySDR::Range(24000000, 1800000000));
    }
    return results;
}

SoapySDR::ArgInfoList SoapyHydraSDR::getFrequencyArgsInfo(const int direction, const size_t channel) const
{
    SoapySDR::ArgInfoList freqArgs;

    // TODO: frequency arguments

    return freqArgs;
}

/*******************************************************************
 * Sample Rate API
 ******************************************************************/

void SoapyHydraSDR::setSampleRate(const int direction, const size_t channel, const double rate)
{
    SoapySDR_logf(SOAPY_SDR_DEBUG, "Setting sample rate: %d", sampleRate);

    if (sampleRate != rate) {
        sampleRate = rate;
        resetBuffer = true;
        sampleRateChanged.store(true);
    }
}

double SoapyHydraSDR::getSampleRate(const int direction, const size_t channel) const
{
    return sampleRate;
}

std::vector<double> SoapyHydraSDR::listSampleRates(const int direction, const size_t channel) const
{
    std::vector<double> results;

    uint32_t numRates;
	hydrasdr_get_samplerates(dev, &numRates, 0);

	std::vector<uint32_t> samplerates;
    samplerates.resize(numRates);

	hydrasdr_get_samplerates(dev, samplerates.data(), numRates);

	for (auto i: samplerates) {
        results.push_back(i);
	}

    return results;
}

void SoapyHydraSDR::setBandwidth(const int direction, const size_t channel, const double bw)
{
    SoapySDR::Device::setBandwidth(direction, channel, bw);
}

double SoapyHydraSDR::getBandwidth(const int direction, const size_t channel) const
{
    return SoapySDR::Device::getBandwidth(direction, channel);
}

std::vector<double> SoapyHydraSDR::listBandwidths(const int direction, const size_t channel) const
{
    std::vector<double> results;

    return results;
}

/*******************************************************************
 * Settings API
 ******************************************************************/

SoapySDR::ArgInfoList SoapyHydraSDR::getSettingInfo(void) const
{
    SoapySDR::ArgInfoList setArgs;

    // Bias-T
    SoapySDR::ArgInfo biasOffsetArg;
    biasOffsetArg.key = "biastee";
    biasOffsetArg.value = "false";
    biasOffsetArg.name = "Bias tee";
    biasOffsetArg.description = "Enable the 4.5v DC Bias tee to power a LNA via antenna connection.";
    biasOffsetArg.type = SoapySDR::ArgInfo::BOOL;

    setArgs.push_back(biasOffsetArg);

    // bitpack
    SoapySDR::ArgInfo bitPackingArg;
    bitPackingArg.key = "bitpack";
    bitPackingArg.value = "false";
    bitPackingArg.name = "Bit Pack";
    bitPackingArg.description = "Enable packing 4 12-bit samples into 3 16-bit words for 25% less USB trafic.";
    bitPackingArg.type = SoapySDR::ArgInfo::BOOL;

    setArgs.push_back(bitPackingArg);

    return setArgs;
}

void SoapyHydraSDR::writeSetting(const std::string &key, const std::string &value)
{
    if (key == "biastee") {
        bool enable = (value == "true");
        rfBias = enable;

        hydrasdr_set_rf_bias(dev, enable);
    }

     if (key == "bitpack") {
        bool enable = (value == "true");
        bitPack = enable;

        hydrasdr_set_packing(dev, enable);
    }

}

std::string SoapyHydraSDR::readSetting(const std::string &key) const
{
    if (key == "biastee") {
        return rfBias?"true":"false";
    }
    if (key == "bitpack") {
        return bitPack?"true":"false";
    }

    // SoapySDR_logf(SOAPY_SDR_WARNING, "Unknown setting '%s'", key.c_str());
    return "";
}
