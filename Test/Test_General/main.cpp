#include <BaseLibrary/Logging_All.hpp>

#include "Test.hpp"
#include <iostream>
#include <string>
#include <conio.h>

using namespace std;

#if !defined(_WIN32) 
static_assert(false, "This file only works under Win32");
#endif


int main() {
	auto instance = TestFactory::GetInstance();	
	auto testList = instance->GetTests();

	cout << "List of available tests:" << endl;
	int i = 1;
	for (auto& v : testList) {
		cout << "[" << i++ << "] " << v << endl;
	}

	int choice;
	string line;
	do {
		cout << "Your choice: ";
		getline(cin, line);
		if (line == "exit") {
			return 0;
		}
		choice = atoi(line.c_str());
	} while (choice <= 0 || testList.size() < choice);

	--choice;

	system("cls");

	cout << "Running " << testList[choice] << "...\n";
	cout << "------------------------------------------------------------------------" << endl << endl;

	int ret = instance->Run(testList[choice]);

	cout << endl << "------------------------------------------------------------------------" << endl;
	cout << "Test returned: " << ret << endl;
	cout << "Press any key to exit." << endl;
	_getch();

	return 0;
}