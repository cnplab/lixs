#include <lixs/mstore/util.hh>

#include <string>


bool lixs::mstore::basename(const std::string& path, std::string& parent, std::string& name)
{
    size_t pos;

    pos = path.rfind('/');
    if (pos == std::string::npos) {
        return false;
    } else {
        parent = path.substr(0, pos);
        name = path.substr(pos + 1);
        return true;
    }
}

