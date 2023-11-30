#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <map>
#include <string>

using namespace std;

int level = 0;
int nodeNumber = 0;
map<string, int> nodes;

void printBBTrace(char* filename, int start, int end) {
	char key[128];
	sprintf(key, "%s,%d,%d", filename, start, end);
	if (nodes.find(string(key)) == nodes.end()) {
		nodes[string(key)] = nodeNumber;
		cerr << "\n" << nodeNumber++ << " = (" << key << ")\n";
	}
	cerr << "," << nodes[string(key)];
}

void printLnTrace(char* filename, int line) {
	char key[128];
	sprintf(key, "%s,%d", filename, line);
	if (nodes.find(string(key)) == nodes.end()) {
		nodes[string(key)] = nodeNumber;
		cerr << "\n" << nodeNumber++ << " = (" << key << ")\n";
	}
	cerr << "," << nodes[string(key)];
}

void printTrace(int node) {
	fstream trace;
	trace.open("/tmp/trace/" __SOURCE__ ".tr", std::fstream::in | std::fstream::out | std::fstream::app);
	trace << "," << node;
	trace.close();
}


void printBBEntry(int ln, char* filename) {
	for (int i = 0; i<level; i++)
		cerr << "|  ";
	cerr << "Enter BasicBlock in file " << (string)filename << " at line # " << ln << endl;
	level++;
}

void printBBExit(int ln, char* filename) {
	level--;
	for (int i = 0; i<level; i++)
		cerr << "|  ";
	cerr << "Exit BasicBlock" << endl;
	// cerr << "Exit BasicBlock in file " << (string)filename << " at line # " << ln << endl;
}
