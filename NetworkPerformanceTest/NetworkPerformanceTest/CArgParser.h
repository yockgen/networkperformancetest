#include <vector>
#include <map>
#include <string>

using namespace std;

class CArgParser
{
public:

	CArgParser(int argc_, char * argv_[], bool switches_on_ = false);
	~CArgParser(){}

	string get_arg(int i);
	string get_arg(string s);

private:

	int argc;
	vector<string> argv;

	bool switches_on;
	map<string, string> switch_map;
};
