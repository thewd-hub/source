unsigned char* findDecryptionKeyIds(unsigned char* key, int keyLength, int keyType, unsigned long* key1Id, unsigned long* key2Id);

unsigned char* decryptKey6(unsigned char* key, int keyLength, int keyType, unsigned long key1Id, unsigned long key2Id);
unsigned char* encryptKey6(unsigned char* key, int keyLength, int keyType, unsigned long key1Id, unsigned long key2Id);

unsigned char* determineKey6StreamOffset(unsigned char* keyStream, unsigned long* keyStreamLength);
unsigned char* decryptKey6Stream(unsigned char* keyStream, unsigned long keyStreamLength, unsigned long* keyStreamDecryptedLength, unsigned long keyStreamChunks, int keyType);