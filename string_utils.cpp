#include "string_utils.h"
#include <sstream>

using namespace std;

vector<string> split(const string &s, char delim, int maxChunks)
{
	vector<string> output;
	
	if (s.size() == 0) {
		return output;
	}
	
	int chunkStart;
	if (s[0] == delim) {
		chunkStart = -1;
	}
	else {
		chunkStart = 0;
	}
	
	for (int i = 0; i < s.size(); i++) {
		if (s[i] == delim && chunkStart != -1) {
			if (maxChunks == -1 || output.size() < maxChunks - 1) {
				output.push_back(s.substr(chunkStart, i - chunkStart));
				chunkStart = -1;
			}
		}
		else if (s[i] != delim && chunkStart == -1) {
			chunkStart = i;
		}
	}
	
	if (chunkStart != -1) {
		output.push_back(s.substr(chunkStart, s.size() - chunkStart));
	}
	
	return output;
}
 
string trim(const string &s, const string &delims)
{
	int start = 0;
	
	while (start < s.size() && delims.find(s[start]) != string::npos) {
		start++;
	}
	
	int end = s.size();
	
	while (end > start && delims.find(s[end - 1]) != string::npos) {
		end--;
	}
	
	return s.substr(start, end - start);
}