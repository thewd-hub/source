//////////////////////////////////////////////////////////////////
// Uses the RC4 algorithm to encrypt or decrypt the script file //
// (Supported by Process Patcher v2.50+                         //
//////////////////////////////////////////////////////////////////

bool isFileBufferEncrypted(unsigned char* fileBuffer);
void encryptFileBuffer(unsigned char* fileBuffer, unsigned long fileBufferLength, bool tryDefaultKey, bool isEncryptionKey);

void encryptScriptFile(char* scriptFilename, bool useDefaultKey);
void decryptScriptFile(char* scriptFilename, bool useDefaultKey);