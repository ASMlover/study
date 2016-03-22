
#include <RCF/test/Usernames.hpp>

bool getUsernames(Usernames & usernames)
{
    std::string whichFile = RCF::getRelativeTestDataPath() + "sspi.txt";

    std::ifstream fin(whichFile.c_str());

    if (!fin)
    {
        return false;
    }

    fin >> usernames.mLocalUsername;
    fin >> usernames.mLocalPassword;
    fin >> usernames.mLocalPasswordBad;
    fin >> usernames.mAdUsername;
    fin >> usernames.mAdPassword;
    fin >> usernames.mAdPasswordBad;
    fin >> usernames.mAdDomain;

    assert(fin);

    if (!fin)
    {
        return false;
    }

    fin.close();

    return true;
}
