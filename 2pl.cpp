#include <iostream>
#include <vector>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <cstdio>
#include <set>
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
	vector<int> txList;	//The list of IDs of tx which has issued that lock
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
 *		NOTE: It contains tx which has already been granted all the required locks!
 *		Additionally, it also contains info about the current
 *		operation the tx currently is on which is initially 0.
 ***********************************************************/
class CurrentlyExecutable {
public:
	Transaction * tx;	//The ptr to the tx
	int ptr;			//The op index within tx is currently to be executed
	CurrentlyExecutable() {
		ptr = 0;		//Initially ptr should be 0
	}
};
int t = 0;				//Time, t = 0 initially
vector<LockTableEntry *> LockTable;		//LockTable is a list of all the data items (variables) which are locked by any transcation currently.
vector<Transaction *> waitingTx;		//List of waiting transcations.
vector<CurrentlyExecutable *> currentlyExecutable;		//The list of tx which can currently be executed. Every second this list must be updated
vector<Transaction *> Transactions;		//It contains the list of all the transactions
vector<ScheduleEntry *> Schedule;		//Schedule which lists the order/interleaving of operations from various tx which will be performed by the system.
	
void updateCurrentlyExecutableTx();
void inputTransactions(vector<Transaction *> Transactions);
int chooseTxToExecute(vector<CurrentlyExecutable *> currentlyExecutable);
bool checkReadOrWriteLock(char dataItem);
//To check if there is a write lock on dataItem
bool checkWriteLock(char dataItem);
//To check if all locks can be granted to tx with index i
bool canAllLocksBeGranted(int i);
//To execute a currently executable tx
void execute(CurrentlyExecutable * c);
// check if a transaction in the waiting queue can be moved over to the executable queue.
void checkWaitingQueue();
// To free up all the locks held by transactio "i"
void freeLocks(int i);
//To grant all required locks by Transaction[i]
void grantAllRequiredLocks(int i);

int main() {
	
	inputTransactions(Transactions);		//Input all tx from stdin into 'Transactions'
	
	updateCurrentlyExecutableTx();	//Updating this list in the beginning
	
	int toExecute;	//To store the index (in currentlyExecuatable) of next tx which is to be executed next.
	while(currentlyExecutable.size() != 0) {
		toExecute = chooseTxToExecute(currentlyExecutable);

		//Execute an operation in currentlyExecutable[toExecute]; 
		//Since all read and write locks were done in the beginning it won't be any problem to simply execute this instruction
		//Execute also increments the ptr in this currently executable tx
		execute(currentlyExecutable[toExecute]);

		//If the currently executed op was the last op in the tx
		if(currentlyExecutable[toExecute]->ptr == currentlyExecutable[toExecute]->tx->operation.size()) {
			//In this case Free all the locks held by this tx.
			freeLocks(currentlyExecutable[toExecute]->tx->txID); //@TODO
			// then remove the tx from currently executable.
			currentlyExecutable.erase(currentlyExecutable.begin() + toExecute);
			//Check in waiting queue if any tx can be put in currently executable tx. If yes put it in currently exec.
			checkWaitingQueue();	//@TODO
		}
		else
			(currentlyExecutable[toExecute]->ptr)++;	//To increment to the next instuction within tx that is to be executed
		
		t++;
		updateCurrentlyExecutableTx();

	}
	
}

void updateCurrentlyExecutableTx() {
	CurrentlyExecutable * x;
	char dataItem;
	for(int i = 0; i < Transactions.size() && Transactions[i]->timestamp <= t; i++) {	//Loop through all Tx until timestamp = current time
		if(Transactions[i]->timestamp == t) {
			//If all the locks required by Transactions[i] can be granted (conservative 2PL) 
			//then only grant the locks
			//else put the tx in waiting queue
			if(canAllLocksBeGranted(i)) { //implies all locks can be granted if true!
				grantAllRequiredLocks(i); //Grant all the locks required by tx i
				//And put this tx into currently executable ones
				x = new CurrentlyExecutable();
				x->tx = Transactions[i];
				currentlyExecutable.push_back(x);
			}
			else
				waitingTx.push_back(Transactions[i]);	//put this tx in waiting Tx list
		}
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

//To check if there is a read or write lock on dataItem
bool checkReadOrWriteLock(char dataItem) {
	for(int i = 0; i < LockTable.size(); i++) {
		if(LockTable[i]->var == dataItem)
			return true;	//Indicating there is a read or write lock on dataItem
	}
	return false;	//Indicating there is no lock on dataItem
}

//To check if there is a write lock on dataItem
bool checkWriteLock(char dataItem) {
	for(int i = 0; i < LockTable.size(); i++) {
		if(LockTable[i]->var == dataItem && LockTable[i]->type == 'w')
			return true;	//Indicating there is a write lock on dataItem
	}
	return false;	//Indicating there is no write lock on dataItem
}

//To check if all locks can be granted to tx with index i
bool canAllLocksBeGranted(int i) {
	char dataItem;
	for(int j = 0; j < Transactions[i]->operation.size(); j++) {	//Loop through all the op in the tx
		dataItem = Transactions[i]->operation[j][1];
		if(Transactions[i]->operation[j][0] == 'w') { //If a write lock is needed...
			if(checkReadOrWriteLock(dataItem)) {	//Check if there is any read or write lock on data item given as argument
				return false;	//In this case this tx cannot be put right now in currently execuatable list
			}
			//else lock('w', dataItem);	//Since it is a write lock there is no need to store which tx locked this data item.
		}
		else {	//If a read lock is needed...
			if(checkWriteLock(dataItem)) {	//...but already some tx has write lock on it
				return false;	//then set flag = 0 indicating it is not possible to execute this tx now
			}
		}
	}
	return true; //If program reaches this point then it is sure that all locks can be granted at this point
}

//To execute a currently executable tx
void execute(CurrentlyExecutable * c) {
	ScheduleEntry *scheduleEntry = new ScheduleEntry();
	scheduleEntry->txID = c->tx->txID;
	scheduleEntry->opType = c->tx->operation[c->ptr][0];
	scheduleEntry->var = c->tx->operation[c->ptr][1];
	scheduleEntry->timeSlot = t;
	Schedule.push_back(scheduleEntry);	//Put this schedule entry into the schedule
	(c->ptr)++;	//Increment the ptr of this tx so that next time next operations is executed
	t++;
}

//To grant all required locks by Transaction[i]
void grantAllRequiredLocks(int i) {
	set<char> readSet;
	set<char> writeSet;
	char dataItem;
	char opType;
	for(int j = 0; j < Transactions[i]->operation.size(); j++) {	//Loop through all the op in the tx
		opType = Transactions[i]->operation[j][0];
		dataItem = Transactions[i]->operation[j][1];
		if(opType == 'w') {
			if(readSet.find(dataItem) != readSet.end()) {	//if dataItem is found in read set
				readSet.erase(readSet.find(dataItem));
				writeSet.insert(dataItem);
			}
		}
		else {
			if(writeSet.find(dataItem) == writeSet.end()) { //If dataItem is not found in write set
				readSet.insert(dataItem);
			}
		}
	}
	set<char>::iterator it;
	LockTableEntry * entry;
	//Enter all writeLocks requested into LockTable
	for(it = writeSet.begin(); it != writeSet.end(); ++it) {
		entry = new LockTableEntry();
		entry->var = *it;
		entry->type = 'w';
		entry->txList.push_back(Transactions[i]->txID);
		LockTable.push_back(entry);
	}
	int j;
	//Enter all readLocks requested into LockTable
	for(it = readSet.begin(); it != readSet.end(); ++it) {
		for(j = 0; j < LockTable.size(); j++) {
			if(LockTable[j]->var == *it) {
				LockTable[j]->txList.push_back(Transactions[i]->txID);
				break;
			}
		}
		if(j == LockTable.size()) { //i.e. if exisiting data item is not found in LockTable
			entry = new LockTableEntry();
			entry->var = *it;
			entry->type = 'r';
			entry->txList.push_back(Transactions[i]->txID);
		}
	}
}

void checkWaitingQueue()
{
	int i,j; // iterating variables.
	// loops through all the transactions in the waiting queue.
	for (i = 0; i < waitingTx.size() ; ++i){
		// find the index of the transaction in the transactions vector that has the same id 
		// as the one in waiting queue
		for (j = 0; j < Transactions.size(); ++j){
			if(waitingTx[i]->txID==Transactions[j]->txID){
				break;
			}
		}
		if(canAllLocksBeGranted(j)) { //implies all locks can be granted if true!
				grantAllRequiredLocks(j); //Grant all the locks required by tx i
				//And put this tx into currently executable ones
				CurrentlyExecutable *x = new CurrentlyExecutable();
				x->tx = Transactions[j];
				currentlyExecutable.push_back(x);
				waitingTx.erase(waitingTx.begin()+i);
			}
	}
}

void freeLocks(int i){
	int a=0;
	while (a<Transactions[i]->operation.size()){
	for (int j=0; j< LockTable.size();j++){
		if (LockTable[j]->var == Transactions[i]->operation[a][1])
		LockTable.erase(LockTable.begin()+j);
	}
	a++;
	}
}
	
void PrintSchedule(){
	cout<<"TX\t"<<"operation\t"<<"var\t"<<"time"<<endl;
	for (int i=0; i< Schedule.size();i++){
		cout<<Schedule[i]->txID<<"\t"<<Schedule[i]->opType<<"\t"<<Schedule[i]->var<<"\t"<<Schedule[i]->timeSlot<<endl;
	}

}







