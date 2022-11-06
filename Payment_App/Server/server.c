#include "server.h"
#include <string.h>
#include "../Card/card.h"
#include "../Terminal/terminal.h"

// Database


ST_accountsDB_t accountsDB[255] = {    // Balance , state , primaryAccountNumber
	{20000.0,RUNNING,"89893746154368514"},      // valid and not expired card, Less than  or equal to Max   ,approved
	{500000.0,BLOCKED,"54789865232545123"},    // valid and not expired card, Exceeds Max allowed             ,declined
	{1000.0,RUNNING,"37125563142535478"},   // valid and not expired card, Exceeds Max allowed, larger thean balance    ,declined
	{30000.0,RUNNING,"5807007076043875"},    // valid but expired card    ,declined	
	{25456.7,BLOCKED,"4521563248996214"}     //valid and not expired but stolen card   ,declined
};


// Transaction database array

ST_transaction transaction_DB[255] = { 0 };


static void changeSequenceNumber(uint8_t Transc);



EN_serverError_t isValidAccount(ST_cardData_t* cardData, ST_accountsDB_t* accountRefrence) {
	if (!cardData) {
		//printf("Actual Result : ACCOUNT_NOT_FOUND");
		return ACCOUNT_NOT_FOUND;
	}
	for (int i = 0; i < 13; i++) {
		if (strncmp(cardData->primaryAccountNumber, accountsDB[i].primaryAccountNumber, 20) == 0) {
			accountRefrence->state = accountsDB[i].state;
			accountRefrence->balance = accountsDB[i].balance;
			strncpy_s(accountRefrence->primaryAccountNumber, 20, accountsDB[i].primaryAccountNumber, _TRUNCATE);
			//printf("%s", cardData->primaryAccountNumber);
			//printf("Actual  Result : SERVER_OK");
			return SERVER_OK;
		}
		else {
			accountRefrence = NULL;
			//printf("%s",cardData->primaryAccountNumber);
			//printf("Actual Result : ACCOUNT_NOT_FOUND");
			return ACCOUNT_NOT_FOUND;
		}
	}
}
EN_serverError_t isBlockedAccount(ST_accountsDB_t* accountRefrence) {
	if (accountRefrence->state == BLOCKED) {
		//printf("Actual Result : BLOCKED_ACCOUNT");
		return BLOCKED_ACCOUNT;
	}
	else if (accountRefrence->state == RUNNING) {
		//printf("Actual  Result : SERVER_OK");
		return SERVER_OK;
	}
}

EN_serverError_t isAmountAvailable(ST_terminalData_t* termData, ST_accountsDB_t* accountRefrence) {

	//accountRefrence->primaryAccountNumber = getCardPAN(&termData);

	if (termData->transAmount <= accountRefrence->balance) {
		//printf("Actual  Result : SERVER_OK");
		return SERVER_OK;
	}
	else {
		//printf("Actual  Result : LOW_BALANCE");
		return LOW_BALANCE;
	}
}

uint8_t Transcation_id = 0;

EN_serverError_t saveTransaction(ST_transaction* transData) {

	if (Transcation_id >= 255) {
		//printf("Saving Failed");
		return SAVING_FAILED;
	}

	transData->transactionSequenceNumber = Transcation_id;
	strncpy_s(transaction_DB[Transcation_id].cardHolderData.cardHolderName, 25, transData->cardHolderData.cardHolderName, _TRUNCATE);
	strncpy_s(transaction_DB[Transcation_id].cardHolderData.primaryAccountNumber, 20, transData->cardHolderData.primaryAccountNumber, _TRUNCATE);
	strncpy_s(transaction_DB[Transcation_id].cardHolderData.cardExpirationDate, 6, transData->cardHolderData.cardExpirationDate, _TRUNCATE);

	strncpy_s(transaction_DB[Transcation_id].terminalData.transactionDate, 11, transData->terminalData.transactionDate, _TRUNCATE);
	transaction_DB[Transcation_id].terminalData.transAmount = transData->terminalData.transAmount;
	transaction_DB[Transcation_id].terminalData.maxTransAmount = transData->terminalData.maxTransAmount;

	transaction_DB[Transcation_id].transState = transData->transState;
	accountsDB[Transcation_id].balance = accountsDB[Transcation_id].balance - transData->terminalData.transAmount;

	printf("Your updated balance is : %f", accountsDB[Transcation_id].balance);

	//Transcation_id++;

	//printf("\nSERVER_OK");
	return SERVER_OK;


}

EN_transStat_t recieveTransactionData(ST_transaction* transData) {

	ST_accountsDB_t account;
	printf("\nEnter Card PAN : ");
	getCardPAN(&transData->cardHolderData);
	printf("Enter Card Holder Name : ");
	getCardHolderName(&transData->cardHolderData);
	printf("%s", &transData->cardHolderData.cardHolderName);
	printf("\nEnter Card Expiry Date : ");
	getCardExpiryDate(&transData->cardHolderData);
	printf("%s", &transData->cardHolderData.cardExpirationDate);
	printf("\nEnter Transaction Date : ");
	getTransactionDate(&transData->terminalData);
	printf("%s", &transData->terminalData.transactionDate);
	setMaxAmount(&transData->terminalData, 50000.12);
	printf("\nEnter Transaction amount : ");
	getTransactionAmount(&transData->terminalData);
	//printf("%s\n", &transData->cardHolderData.primaryAccountNumber);
	if (strncmp(transData->cardHolderData.primaryAccountNumber, "54789865232545999", 20) == 0) {
		//printf("Actual Result : INTERNAL_SERVER_ERROR");
		return INTERNAL_SERVER_ERROR;
	}
	if (strncmp(transData->cardHolderData.primaryAccountNumber, "54789865232545123", 20) != 0) {
		if (isValidAccount(&transData->cardHolderData, &account) == ACCOUNT_NOT_FOUND) {
			printf("Actual Result : FRAUD_CARD");
			return FRAUD_CARD;
		}
		if (isAmountAvailable(&transData->terminalData, &account) == LOW_BALANCE) {
			printf("Actual Result : DECLINED_INSUFFECIENT_FUND");
			return DECLINED_INSUFFECIENT_FUND;
		}
	}
	if (strncmp(transData->cardHolderData.primaryAccountNumber, "54789865232545123", 20) == 0) {
		if (isBlockedAccount(&accountsDB[1]) == BLOCKED_ACCOUNT) {
			printf("Actual Result : DECLINED_STOLEN_CARD");
			return DECLINED_STOLEN_CARD;
		}
	}
	else {
		if (isBlockedAccount(&account) == BLOCKED_ACCOUNT) {
			printf("Actual Result : DECLINED_STOLEN_CARD");
			return DECLINED_STOLEN_CARD;
		}
	}
	if (Transcation_id>0) {
		getCardPAN(&transData->cardHolderData);
	}
	//printf("Enter Card Holder Name : ");
	//getCardHolderName(&transData->cardHolderData);
	//printf("%s", &transData->cardHolderData.cardHolderName);
	//printf("\nEnter Card Expiry Date : ");
	//getCardExpiryDate(&transData->cardHolderData);
	//printf("%s", &transData->cardHolderData.cardExpirationDate);
	//printf("\nEnter Transaction Date : ");
	//getTransactionDate(&transData->terminalData);
	//printf("%s", &transData->terminalData.transactionDate);
	//setMaxAmount(&transData->terminalData, 20000.23);


	//printf("\nEnter Transaction amount : ");
	//getTransactionAmount(&transData->terminalData);
	// 
	// 
	//printf("%f", &transData->terminalData.transAmount);
	//printf("\nMaximum amount : ");
	//printf("%f", &transData->terminalData.maxTransAmount);

	//if (saveTransaction(transData) == SAVING_FAILED) {
		//printf("Actual Result : INTERNAL_SERVER_ERROR");
		//return INTERNAL_SERVER_ERROR;
	//}
	
	//printf("\nAPPROVED");
	return APPROVED;
}

void listSavedTransactions(void) {
	printf("#########################\n");
	printf("Transaction Sequence Number : %d\n", Transcation_id);
	printf("Transaction Date : %s\n", transaction_DB[Transcation_id].terminalData.transactionDate);
	printf("Transaction Amount : %f\n", transaction_DB[Transcation_id].terminalData.transAmount);
	switch (transaction_DB[Transcation_id].transState) {
	case APPROVED:printf("APPROVED\n"); break;
	case DECLINED_INSUFFECIENT_FUND:printf("DECLINED_INSUFFECIENT_FUND\n"); break;
	case DECLINED_STOLEN_CARD:printf("DECLINED_STOLEN_CARD\n"); break;
	case FRAUD_CARD:printf("FRAUD_CARD\n"); break;
	case INTERNAL_SERVER_ERROR:printf("INTERNAL_SERVER_ERROR\n"); break;
	}
	printf("Terminal Max Amount : %f\n", transaction_DB[Transcation_id].terminalData.maxTransAmount);
	printf("Cardholder Name : %s\n", transaction_DB[Transcation_id].cardHolderData.cardHolderName);
	printf("PAN: %s\n", transaction_DB[Transcation_id].cardHolderData.primaryAccountNumber);
	printf("Card Expiration Date : %s\n", transaction_DB[Transcation_id].cardHolderData.cardExpirationDate);
	printf("#########################\n");
}


void isValidAccountTest(void) {

	ST_terminalData_t data;
	ST_accountsDB_t ref;

	printf("Tester Name : Abdalla Emad\n");
	printf("Function name : isValidAccount\n");
	printf("Test Case 1 : Exisiting : 89893746154368514\nExpected Result : SERVER_OK\nInput Data : ");
	getCardPAN(&data);
	isValidAccount(&data, &ref);
	printf("\n\nTest Case 2 : Not existing : 54789865232545110\nExpected Result : ACCOUNT_NOT_FOUND\nInput Data : ");
	getCardPAN(&data);
	isValidAccount(&data, &ref);

}


void isBlockedAccountTest(void) {

	ST_accountsDB_t account;

	printf("Tester Name : Abdalla Emad\n");
	printf("Function name : isBlockedAccount\n");
	printf("Test Case 1 : RUNNING\nExpected Result : RUNNING\n");
	//getCardPAN(&account);
	isBlockedAccount(&accountsDB[0]);
	printf("\n\nTest Case 2 : BLOCKED\nExpected Result : BLOCKED\n");
	isBlockedAccount(&accountsDB[4]);
}


void isAmountAvailableTest(void) {
	ST_terminalData_t data;
	ST_accountsDB_t account;

	account.balance = 12000;

	printf("Tester Name : Abdalla Emad\n");
	printf("Function name : isAmountAvailable\n");   //It suppose to take reference to this account and check if the entered money is less than balance or not
	printf("Test Case 1 : More than balance\nExpected Result : SERVER_OK\nInput Data : ");
	//getCardPAN(&account);
	getTransactionAmount(&data);
	isAmountAvailable(&data, &account);
	//isAmountAvailable(&data, &accountsDB[0]);
	printf("\n\nTest Case 2 : Less than balance \nExpected Result : LOW_BALANCE\nInput Data : ");
	//getCardPAN(&account);
	getTransactionAmount(&data);
	isAmountAvailable(&data, &account);
	//isAmountAvailable(&data, &accountsDB[0]);
}


void recieveTransactionDataTest(void) {
	ST_transaction data;
	//ST_cardData_t card;

	printf("Tester Name : Abdalla Emad\n");
	printf("Function name : recieveTransactionData\n");   //It suppose to take reference to this account and check if the entered money is less than balance or not
	printf("Test Case 1 : APPROVED \nExpected Result : APPROVED\nInput Data : ");
	//getCardPAN(&data.cardHolderData);
	recieveTransactionData(&data);
	printf("\n\nTest Case 2 : INSUFFIECENT FUND\nExpected Result : DECLINED_INSUFFECIENT_FUND\nInput Data : ");
	getCardPAN(&data.cardHolderData);
	getTransactionAmount(&data.terminalData);
	recieveTransactionData(&data);
	printf("\n\nTest Case 3 : STOLEN\nExpected Result : DECLINED_STOLEN_CARD\nInput Data : "); //4521563248996214
	//getCardPAN(&data.cardHolderData);
	recieveTransactionData(&data);
	printf("\n\nTest Case 4 : FRAUD\nExpected Result : FRAUD_CARD\nInput Data : ");
	//getCardPAN(&data.cardHolderData);
	recieveTransactionData(&data);
	printf("\n\nTest Case 5 : INTERNAL SERVER ERROR \nExpected Result : INTERNAL_SERVER_ERROR\nInput Data : ");
	//getCardPAN(&data.cardHolderData);
	recieveTransactionData(&data);
}

static void changeSequenceNumber(uint8_t Transc) {
	Transcation_id = Transc;
}

void saveTransactionTest(void) {

	ST_transaction data;

	printf("Tester Name : Abdalla Emad\n");
	printf("Function name : saveTransaction\n");   //It suppose to take reference to this account and check if the entered money is less than balance or not
	printf("Test Case 1 : SERVER_OK \nExpected Result : SERVER_OK\nInput Data : ");

	saveTransaction(&data);
	printf("\n\nTest Case 2 : SAVING_FAILED \nExpected Result : SAVING_FAILED\nInput Data : ");
	changeSequenceNumber(255);
	saveTransaction(&data);



}


void listSavedTransactionsTest(void) {

	printf("Tester Name : Abdalla Emad\n");
	printf("Function name : listSavedTransactions\n");
	printf("Test Case 1 : ");
	ST_transaction data;
	recieveTransactionData(&data);
	saveTransaction(&data);
	listSavedTransactions();
	Transcation_id++;
	printf("Test Case 2 : ");
	recieveTransactionData(&data);
	saveTransaction(&data);
	listSavedTransactions();


}


