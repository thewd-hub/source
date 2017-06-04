////////////////////////////////////////////////////////////
// Handles the patching threads that are created to patch //
// the target process, modules and execute plugin modules //
////////////////////////////////////////////////////////////

// current plugin api version
#define API_PLUGIN_VERSION							API_PLUGIN_VERSION_0103
#include "apiPluginSupport.h"

#define PATCHINGENGINE_SLEEPDELAY					50
#define PATCHINGENGINE_DEFAULTPROCESSACCESS			PROCESS_ALL_ACCESS

// patching engine has initialised all the plugins
#define EVENT_PATCHINGENGINE_PLUGINS_INITIALISED	101

bool hasPatchingEngineStarted(void);

void terminateQuitMessageThread(void);

char* getWaitingEventName(unsigned long eventId);
void waitForPatchingEngineWaitingEvent(unsigned long eventId, bool checkProcessStatus);
void setPatchingEngineWaitingEvent(unsigned long eventId, bool handleEventErrors = true);
void resetPatchingEngineWaitingEvent(unsigned long eventId, bool handleEventErrors = true);

void executePatchingEngine(void);
void terminatePatchingEngine(void);