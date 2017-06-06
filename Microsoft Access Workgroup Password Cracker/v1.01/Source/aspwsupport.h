////////////////////////////////////////////////////////////
// Microsoft Access Workgroup Password Cracker            //
// (C)thewd, thewd@hotmail.com                            //
////////////////////////////////////////////////////////////

bool isJetWorkgroup(unsigned char* fileHeader);
void doRC4(unsigned char* fileBuffer, int baseOffset, bool determineXORBaseValues);
void recoverWorkgroupInformation(unsigned char* fileBuffer, int fileOffset, int databaseFormat,
								 char** workgroupName, char** workgroupOrganisation, char** workgroupId);
char* recoverAccountName(unsigned char* fileBuffer, int fileOffset, int accountNameLength, int databaseFormat);
char* recoverPassword(unsigned char* fileBuffer, int fileOffset, int databaseFormat);
char* recoverPID(unsigned char* fileBuffer, int fileOffset, int pidLength);