#include <iostream>
#include <map>
#include <cstdio>

using namespace std;

class Student {
	int id;
	string name;
};

int main() {
	Student s1;
	printf("Original address: %p\n", &s1);

	multimap<int, Student*> m;

	m.insert(pair<int, Student*>(1, &s1));
	printf("Map address: %p\n", (*m.begin()));
	
}
