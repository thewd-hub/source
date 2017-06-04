char* decryptPassword(unsigned char* encryptedPassword, int encryptedPasswordLength);
unsigned char* encryptPassword(unsigned char* plaintextPassword, int plaintextPasswordLength);

unsigned short getKeyCRC(unsigned char* key, int keyLength, int keyType);
bool isKeyCRCCorrect(unsigned char* key, int keyLength, unsigned short keyCRC, int keyType);
unsigned char* decryptKey(unsigned char* key, int keyLength, int keyType);
unsigned char* encryptKey(unsigned char* key, int keyLength, int keyType);

unsigned char* decryptSystemLicenceFile(unsigned char* fileBuffer, unsigned long fileLength);