//////////////////////////////////////////////////////////////////////////////
// Loads the required modules and initialises the appropriate api functions //
//////////////////////////////////////////////////////////////////////////////

bool initialiseKERNEL32ApiFunctions(void);
API_KERNEL32_MODULE getKERNEL32ApiFunctions(void);

bool initialiseNTDLLApiFunctions(void);
API_NTDLL_MODULE getNTDLLApiFunctions(void);