

#include "../Card/card.h"
#include "app.h"
#include "../Server/server.h"
#include "../STD_Types.h"
#include "../Terminal/terminal.h"


void appStart(void) {
	ST_transaction trans;
	EN_transStat_t transvariable =  recieveTransactionData(&trans);
	// In receive I checked if the account is blocked and if it is valid or if the amount available
	ST_accountsDB_t ref;
	if (isCardExpired(&trans.cardHolderData, &trans.terminalData) == TERMINAL_OK) {
		//getTransactionAmount(&trans.terminalData);
		if (isBelowMaxAmount(&trans.terminalData) == TERMINAL_OK) {
			if (isValidAccount(&trans.cardHolderData, &ref) == SERVER_OK) {
				//recieveTransactionData(&trans);
				if (transvariable == APPROVED)
				{
					if (saveTransaction(&trans) == SERVER_OK)
					{
						printf("\nTranscation Saved");
					}
					else
						printf("\nSaving Failed");
				}
				else if (transvariable == DECLINED_STOLEN_CARD)
				{
					printf("\nDeclined Invalid Account");
				}
				else if (transvariable == DECLINED_INSUFFECIENT_FUND) {
					printf("\nInsuffecient Funds");
				}
				else {
					printf("Hello");
				}
					
			}
		}
		else if (isBelowMaxAmount(&trans.terminalData) == EXCEED_MAX_AMOUNT) {
			printf("Exceed Max Amount");
		}
	}
	else if (isCardExpired(&trans.cardHolderData, &trans.terminalData) == EXPIRED_CARD) {
		printf("Expired Card");
	}


}