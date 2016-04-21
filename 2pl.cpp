#include <iostream>
#include <vector>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <cstdio>
using namespace std;

/**************************************************************
 * Class : LockTableEntry
 *		Each instance of this class is an entry in the lock
 *		table which stores the locked data item by var,
 *		the type of lock(Read/Write or Shared/Exclusive) by type,
 *		and the list of tx which has issued that lock
 *************************************************************/
class LockTableEntry {
public:
	char var;			//The data item locked
	char type;			//The type of lock
	vector<int> txList;	//The list of tx which has issued that lock
};

/**************************************************************
 * Class : ScheduleEntry
 *		Each instance of this class is an entry in the schedule
 *		that is to be built at the end of this program.
 ************************************************************/
class ScheduleEntry {
public:
	int txID;		//The tx ID of tx to which this schedule entry belongs
	char opType;	//The type of op performed i.e. read, write, commit or abort
	char var;		//The data item on which this operation is being performed
	int timeSlot;	//The time at which this tx is being executed
};

/*************************************************************
 * Class : Transaction
 *		An instance of this class contains all info about a tx.
 ************************************************************/
class Transaction {
public:
	int txID;					//The ID of this transaction.
	int timestamp;				//The timestamp when this tx entered into the system.
	vector<char *> operation;	//List of the operations in this transaction. e.g. rx, wy etc.
};


/************************************************************
 * Class : CurrentlyExecutable
 * 		Each instance of this class contains some info about
 * 		any tx which can currently be executed. Therefore,
 *		it doesn't contain tx whose timestamp is greater 
 *		than current time or tx which has committed.
 *		Additionally, it also contains info about the current
 *		operation the tx currently is on which is initially 0.
 ***********************************************************/
class CurrentlyExecutable {
public:
	int txID;			//The ID of the tx
	int ptr;			//The op index within tx is currently to be executed
	CurrentlyExecutable() {
		ptr = 0;		//Initially ptr should be 0
	}
};
int t = 0;				//Time, t = 0 initially


void updateCurrentlyExecutableTx(vector<CurrentlyExecutable *> & currentlyExecutable, vector<Transaction *> & Transactions);
void inputTransactions(vector<Transaction *> Transactions);
int chooseTxToExecute(vector<CurrentlyExecutable *> currentlyExecutable);

int main() {
	vector<LockTableEntry *> LockTable;		//LockTable is a list of all the data items (variables) which are locked by any transcation currently.
	vector<ScheduleEntry *> Schedule;		//Schedule which lists the order/interleaving of operations from various tx which will be performed by the system.
	vector<Transaction *> Transactions;		//It contains the list of all the transactions
	vector<Transaction *> waitingTx;		//List of waiting transcations.
	inputTransactions(Transactions);		//Input all tx from stdin into 'Transactions'
	

	vector<CurrentlyExecutable *> currentlyExecutable;		//The list of tx which can currently be executed. Every second this list must be updated
	updateCurrentlyExecutableTx(currentlyExecutable, Transactions);	//Updating this list in the beginning
	
	
	int toExecute;	//To store the next tx which is to be executed next.
	cout << endl;
	while(currentlyExecutable.size() != 0) {
		toExecute = chooseTxToExecute(currentlyExecutable);
		cout << toExecute;
		
		//Execute an operation in currentlyExecutable[toExecute]; Since all read and write locks were done in the beginning it won't be any problem to simply execute this instruction
		//If the currently executed op was the last op in the tx then remove the tx from currently executable.
			//In this case Free all the locks held by this tx.
			//Check in waiting queue if any tx can be put in currently executable tx.
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

//inputTransactions inputs all the transactions from stdin and stores them in a list 'Transactions'
void inputTransactions(vector<Transaction *> Transactions) {
	char * line = new char[10];				//To store a line from the input
	char * op;								//
	Transaction * tx;						//To store the details of current transaction being input from stdin
	while(cin.getline(line, 10) ) {			//While there is input read a line
		cout << line;
		if(line[0] == 't') {				//If line starts with a 't' it means a new tx has started
			tx = new Transaction();			//In such a case allocate memory for the new tx
			sscanf(line, "t%d %d", &tx->txID, &tx->timestamp);		//Store the tx ID and timestamp when it entered the system.
			Transactions.push_back(tx);		//Add this transaction to the list of Transactions.
		}
		else {								//If first char is not 't' then this line must be a new operation in existing tx
			op = new char[2];				//Allocate memory for this operation
			op[0] = line[0];				//op[0] stores the type of operation ie read or write
			op[1] = line[1];				//op[1] stores the data item on which this op is being executed
			tx->operation.push_back(op);	//Add this operation to the current tx's list of operations.
		}
	}
}

int chooseTxToExecute(vector<CurrentlyExecutable *> currentlyExecutable) {
	srand(time(NULL));
	return rand() % currentlyExecutable.size();
}