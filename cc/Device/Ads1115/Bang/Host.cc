// BSD 2-Clause License, see github.com/ma16/rpio

#include "Host.h"
#include "Generator.h"

using namespace Device::Ads1115 ;

Bang::Record::Reset Bang::Host::reset()
{
    Record::Reset record ;
    auto script = Generator(config).reset(&record) ;
    RpiExt::Bang(this->rpi).execute(script) ;
    return record ;
}

Bang::Record::Read Bang::Host::readConfig()
{
    Record::Read record ;
    auto script = Generator(config).readConfig(&record) ;
    RpiExt::Bang(this->rpi).execute(script) ;
    return record ;
}

Bang::Record::Read Bang::Host::readSample()
{
    Record::Read record ;
    auto script = Generator(config).readSample(&record) ;
    RpiExt::Bang(this->rpi).execute(script) ;
    return record ;
}

Bang::Record::Write Bang::Host::writeConfig(uint16_t word)
{
    Record::Write record ;
    auto script = Generator(config).writeConfig(&record,word) ;
    RpiExt::Bang(this->rpi).execute(script) ;
    return record ;
}


