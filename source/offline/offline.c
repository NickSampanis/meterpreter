//
// Note: To use the produced x86 dll on NT4 we use a post build event "editbin.exe /OSVERSION:4.0 /SUBSYSTEM:WINDOWS,4.0 elevator.dll" 
//       in order to change the MajorOperatingSystemVersion and MajorSubsystemVersion to 4 instead of 5 as Visual C++ 2008
//       can't build PE images for NT4 (only 2000 and up). The modified dll will then work on NT4 and up. This does
//       not apply to the produced x64 dll.
//

#include "offline.h"

// define this as we are going to be injected via LoadRemoteLibraryR
#define REFLECTIVEDLLINJECTION_VIA_LOADREMOTELIBRARYR

// define this as we want to use our own DllMain function
#define REFLECTIVEDLLINJECTION_CUSTOM_DLLMAIN

// include the Reflectiveloader() function
#include "../ReflectiveDLLInjection/dll/src/ReflectiveLoader.c"

/*
 * The real entrypoint for this app.
 */
VOID offline_main(char *pipeName)
{
        DWORD dwResult = ERROR_SUCCESS;
        
        do {
                if (!pipeName || !strlen(pipeName))
                        BREAK_ON_ERROR("[OFFLINE] offline_main.")
                dprintf("[OFFLINE] offline_main. pipeName=%s\n", pipeName);

	} while(0);
}

/*
 * DLL entry point. If we have been injected via RDI, lpReserved will be our command line.
 */
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
        BOOL bReturnValue = TRUE;
        
        switch( dwReason ) {
                case DLL_PROCESS_ATTACH:
			hAppInstance = hInstance;
			if(lpReserved != NULL)
                             offline_main((char *)lpReserved);
			break;
		case DLL_PROCESS_DETACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
                        break;
        }
        
        return bReturnValue;
}

