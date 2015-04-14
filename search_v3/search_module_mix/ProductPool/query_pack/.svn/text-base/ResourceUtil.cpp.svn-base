#include "ResourceUtil.h"

const char ResourceUtil::CH_KV_SEPARATOR = '=';
const char ResourceUtil::CH_ANNOTATION   = '#';
string ResourceUtil::m_resFilename;
hash_map<string, string> ResourceUtil::m_props;

bool ResourceUtil::configure(const string & filename)
{
    if (Trim(filename).empty())
    {
        return false;
    }
    m_resFilename = filename;
    return readToMap();
}

string ResourceUtil::getProperty(const string & key)
{
    hash_map<string, string>::const_iterator it = m_props.find(key);
	return (it == m_props.end() ? "" : it->second);
}

void ResourceUtil::printAll()
{
    for (hash_map<string, string>::const_iterator it = m_props.begin();
         it != m_props.end(); ++it)
    {
        cout << it->first << " = " << it->second << endl;
    }
}

bool ResourceUtil::readToMap()
{
    // 1. open file
    ifstream fin(m_resFilename.c_str());
    if (!fin)
    {
        cerr << "Open file '" << m_resFilename << "' failed: " << strerror(errno) << ". [" << __FILE__ << ", " << __LINE__ << "]" << endl;
        fin.clear();
        return false;
    }

    // 2. read line
    string line;
    int index = -1;
    while (getline(fin, line, '\n'))
    {
        // 3. if empty line and annotation line, to be continue
        if (Trim(line) == "" || line.at(0) == CH_ANNOTATION)
        {
            continue;
        }

        // 4. parse line to key-value
        index = line.find_first_of(CH_KV_SEPARATOR);
        if (index <= 0)
        {
            cerr << "File '" << m_resFilename << "' parse error: \n\t"
                 << "line '" << line << "' must be set like 'key=value'" << endl;
            continue;
        }
        string key = Trim(line.substr(0, index));
        string value = Trim(line.substr(index + 1));

        m_props[key] = value;
    }
    fin.close();

    return true;
}

/*string ResourceUtil::Trim(string & str)
{
    return public_func::Trim(str);
}*/

void ResourceUtil::separate(const string & str, const char sep_ch, vector<string> & result)
{
    int nextStartIdx = 0;
    int nextSepIdx = -1;
    while ((nextSepIdx = str.find_first_of(sep_ch, nextStartIdx)) != -1)
    {
        result.push_back(Trim(str.substr(nextStartIdx, nextSepIdx - nextStartIdx)));
        nextStartIdx = nextSepIdx + 1;
    }
    result.push_back(Trim(str.substr(nextStartIdx)));
}

void ResourceUtil::release()
{
    m_props.clear();
}

