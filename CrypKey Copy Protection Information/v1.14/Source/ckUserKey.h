void getUserKeyHashes(unsigned char* encryptedPasswordBytes, int encryptedPasswordLength, int* userKeyHash, int* userKeyHash6);
void getUserKeyHashesFromUserKey(char* userKeyString, int* userKeyHash, int* userKeyHash6);
void getUserKeyHashesFromPassword(char* passwordString, int* userKeyHash, int* userKeyHash6);

void showUserKeyInformation(char* userKeyString);
void createUserKey(void);