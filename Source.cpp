#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <bitset>
#include <vector>
#include <tuple>
//Mike Damiano
//UFID: 96140491
using namespace std;

//based off of https://www.geeksforgeeks.org/program-binary-decimal-conversion/ for conversion function

int binaryToDec(string n) {
	long num = stol(n);
	int dec = 0;
	int base = 1;

	long temp = num;
	while (temp) {
		long last = (int)temp % 10;
		temp /= 10;

		dec += last*base;

		base *= 2;
	}
	return dec;
}

int main() {

	string inputFile;
	double cacheBytes;
	double lineBytes;
	double setBytes;
	string strategy;

	int directHit = 0;
	int directMiss = 0;

	int fullHitLRU = 0;
	int fullMissLRU = 0;
	int fullHitFIFO = 0;
	int fullMissFIFO = 0;
	int fullcounter = 0;

	int setHitFIFO = 0;
	int setMissFIFO = 0;
	int setHitLRU = 0;
	int setMissLRU = 0;

	cout << "What file would you like to read?" << endl;
	cin >> inputFile;

	cout << "How many bytes in cache?" << endl;
	cin >> cacheBytes;

	cout << "How many bytes per line?" << endl;
	cin >> lineBytes;

	cout << "How many lines in a set?" << endl;
	cin >> setBytes;

	string input;
	ifstream file;

	int numlines = cacheBytes / lineBytes;

	vector<tuple<int, int, int, char>> directMapped(numlines);  //tag, line, offset, isFull
	vector<tuple<int, int, int, int>> setAssociativeLRU(numlines);
	vector<tuple<int, int, int, char>> setAssociativeFIFO(numlines);
	vector<tuple<int, int, int>> fullAssociativeLRU(numlines);
	vector<tuple<int, int, char>> fullAssociativeFIFO(numlines);

	file.open(inputFile);

	if (file.is_open()) {
		while (getline(file, input)) {

			input = input.substr(2);

			int space = input.find(" ");
			input = input.substr(0, space);

			stringstream ss;
			ss << hex << input;
			unsigned i;
			ss >> i;
			bitset<32> b(i);
			input = b.to_string();

			//direct mapped storing
			//offset
			int pos1 = 32 - log2(lineBytes);
			int offset = binaryToDec(input.substr(pos1));

			//line
			int pos2 = pos1 - (log2(cacheBytes / lineBytes));
			int line = binaryToDec(input.substr(pos2, pos1 - pos2));

			//tag
			int tagInt = stoi(input.substr(0, pos2), 0, 2);

			//insertion based on replacement strategy

			if (get<3>(directMapped.at(line)) == 'F') {
				if (get<0>(directMapped.at(line)) == tagInt) {
					directHit++;
				}
				else {
					directMapped.at(line) = (tuple<int, int, int, char>(tagInt, offset, line, 'F'));
					directMiss++;
				}
			}
			else {
				directMapped.at(line) = (tuple<int, int, int, char>(tagInt, offset, line, 'F'));
				directMiss++;
			}
			
			//fully associative
			tagInt = stoi(input.substr(0, pos1), 0, 2);
			
			//FIFO case
			int index = fullcounter % numlines;
			bool t = false;
			for (int i = 0; i < fullAssociativeFIFO.size(); i++) {
				if (get<0>(fullAssociativeFIFO.at(i)) == tagInt) {
					fullHitFIFO++;
					t = true;
				}
			}
			if (!t) {
				fullAssociativeFIFO.at(index) = (tuple<int, int, char>(tagInt, offset, 'F'));
				fullMissFIFO++;
			}

			//LRU case
			t = false;
			for (int i = 0; i < fullAssociativeLRU.size(); i++) {
				if (get<0>(fullAssociativeLRU.at(i)) == tagInt) {
					fullHitLRU++;
					get<2>(fullAssociativeLRU.at(i)) = fullcounter;
					t = true;
				}
			}
			if (!t) {
				int temp = 0;
				for (int i = 0; i < fullAssociativeLRU.size(); i++) {
					if (get<2>(fullAssociativeLRU.at(i)) < get<2>(fullAssociativeLRU.at(temp))) {
						temp = i;
					}
				}
				fullAssociativeLRU.at(temp) = (tuple<int, int, int>(tagInt, offset, fullcounter));
				fullMissLRU++;
			}

			//set associative
			int pos3 = pos1 - (log2((cacheBytes / lineBytes) / setBytes));
			int set = binaryToDec(input.substr(pos3, pos1 - pos3));
			index = set * setBytes;
			tagInt = stoi(input.substr(0, pos3), 0, 2);

			//FIFO
			t = false;
			int n = setBytes;
			for (int i = 0; i < n; i++) {

				if (index + i == setAssociativeFIFO.size()) {
					n = n - i;
					i = 0;
				}
				if (get<0>(setAssociativeFIFO.at(index+i)) == tagInt) {
					setHitFIFO++;
					t = true;
					break;
				}
			}
			if (!t) {
				setAssociativeFIFO.at(index + (fullcounter % (int)setBytes)) = (tuple<int, int, int, char>(tagInt, offset, set, 'F'));
				setMissFIFO++;
			}

			//LRU
			t = false;
			n = setBytes;
			for (int i = 0; i < n; i++) {

				if (index + i == setAssociativeLRU.size()) {
					n = n - i;
					i = 0;
				}
				if (get<0>(setAssociativeLRU.at(index + i)) == tagInt) {
					setHitLRU++;
					get<3>(setAssociativeLRU.at(index + i)) = fullcounter;
					t = true;
				}
			}
			if (!t) {
				int temp = index;
				n = setBytes;
				for (int i = 0; i < n; i++) {

					if (index + i == setAssociativeLRU.size()) {
						n = n - i;
						i = 0;
					}
					if (get<3>(setAssociativeLRU.at(i+index)) < get<3>(setAssociativeLRU.at(temp))) {
						temp = i + index;
					}
				}
				setAssociativeLRU.at(temp) = (tuple<int, int, int, char>(tagInt, offset, set, fullcounter));
				setMissLRU++;
			}
			t = false;
			fullcounter++;
		}
	}

	//outputs data in simple format
	cout << "Total Count: " << fullcounter << endl;
	cout << endl;
	cout << "Direct Hits: " << directHit << endl;
	cout << "Direct Misses: " << directMiss << endl;
	cout << endl;
	cout << "Full associative Hit (LRU): " << fullHitLRU << endl;
	cout << "Full associative Miss (LRU): " << fullMissLRU << endl;
	cout << "Full associative Hit (FIFO): " << fullHitFIFO << endl;
	cout << "Full associative Miss (FIFO): " << fullMissFIFO << endl;
	cout << endl;
	cout << "Set associative Hit (LRU): " << setHitLRU << endl;
	cout << "Set associative Miss (LRU): " << setMissLRU << endl;
	cout << "Set associative Hit (FIFO): " << setHitFIFO << endl;
	cout << "Set associative Miss (FIFO): " << setMissFIFO << endl;

	file.close();
	return 0;
}