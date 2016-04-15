#include <iostream>
#include <vector>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

using namespace std;

class LockTableEntry {
public:
	char var;
	char type;
	vector<int> txList;
};

class ScheduleEntry {
public:
	int txID;
	char opType;
	char var;
	int timeSlot;
};

class Transaction {
public:
	int txID;
	int timestamp;
	vector<char *> operation;
};

class CurrentlyExecutable {
public:
	int txID;
	int ptr;
	CurrentlyExecutable() {
		ptr = 0;
	}
};
int t = 0;


void updateCurrentlyExecutableTx(vector<CurrentlyExecutable *> & currentlyExecutable, vector<Transaction *> & Transactions);
int main() {
	vector<LockTableEntry *> LockTable;
	vector<ScheduleEntry *> Schedule;
	vector<Transaction *> Transactions;
	char * line = new char[10];
	char * op;
	Transaction * tx;
	while(cin.getline(line, 10) ) {
		cout << line;
		if(line[0] == 't') {
			tx = new Transaction();
			tx->txID = line[1] - '0';
			tx->timestamp = line[3] - '0';
			Transactions.push_back(tx);
		}
		else {
			op = new char[2];
			op[0] = line[0];
			op[1] = line[1];
			tx->operation.push_back(op);
		}
	}

	vector<CurrentlyExecutable *> currentlyExecutable;
	updateCurrentlyExecutableTx(currentlyExecutable, Transactions);
	
	srand(time(NULL));
	int toExecute;
	cout << endl;
	while(currentlyExecutable.size() != 0) {
		toExecute = rand() % currentlyExecutable.size();	
		cout << toExecute;
		break;

		updateCurrentlyExecutableTx(currentlyExecutable, Transactions);

	}
	


}

void updateCurrentlyExecutableTx(vector<CurrentlyExecutable *> & currentlyExecutable, vector<Transaction *> & Transactions) {
	CurrentlyExecutable * x;
	for(int i = 0; i < Transactions.size(); i++) {
		if(Transactions[i]->timestamp == t) {
			x = new CurrentlyExecutable();
			x->txID = Transactions[i]->txID;
			currentlyExecutable.push_back(x);
		}
		if(Transactions[i]->timestamp > t) break;
	}
}