#include "ip_layer.h"
#include "parser.h"

using namespace std;

/**
 * Show the contents of the cache
 */
void ARPcache::displayTable()
{
	cout << "ARP CACHE" << endl;
	cout << "IP ADDRESS\tMAC ADDRESS" << endl;
	for(auto &it : m_ipToMacTable) {
		cout << ntop(it.first) << "\t";
		cout << it.second << "\t";
		cout << endl;
	}
}

