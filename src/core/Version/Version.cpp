#include "Version.h"

Version::Version(unsigned int major, unsigned int minor, unsigned int patch, bool requireEEPROMFormat) :
        major(major), minor(minor), patch(patch), requireEEPROMFormat(requireEEPROMFormat) {
}

bool Version::operator!=(const Version &version) const {
    return this->major != version.major ||
           this->minor != version.minor ||
           this->patch != version.patch;
}

String Version::str_version() const {
    std::ostringstream vs;
    vs << this->major << "." << this->minor << "." << this->patch;
    return vs.str().c_str();
}

String Version::str_fullVersion() const {
    String vs = this->str_version();
    vs.concat(" - build date: ");
    vs.concat(__DATE__);
    return vs;
}

unsigned int Version::uint_version() const {
    std::ostringstream vs;
    vs << this->major << this->minor << this->patch;

    unsigned int version;
    std::istringstream(vs.str()) >> version;

    return version;
}

bool Version::EEPROMStructureChanged() const {
    return this->requireEEPROMFormat;
}
