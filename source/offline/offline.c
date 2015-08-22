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
 * Check if the arg pipeName is valid
 * Create The namedpipe
 * pipe receives and creates a new thread to handle
 * pipe should send/receive msgs like its a transport using the packet_ functions
 *
 * pipe_create create the namedpipe
 * pipe_recv in a while loop extracting data
 * pipe_send  packet_create response for each received request
 * packet_receive_via_pipe
 */

static VOID offline_main(char *pipeName)
{
        DWORD dwResult          = ERROR_SUCCESS;
        Packet *packet          = NULL;
        HANDLE hServerPipe      = NULL;
        Tlv methodTlv;

        do {
                //check
                if (!pipeName || !strlen(pipeName))
                        BREAK_ON_ERROR("[OFFLINE] offline_main.")
                dprintf("[OFFLINE] offline_main. pipeName=%s\n", pipeName);

                //create pipe
                hServerPipe = CreateNamedPipe(pipeName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE|PIPE_WAIT, 2, 0, 0, 0, NULL);
                if (!hServerPipe)
                        BREAK_ON_ERROR("[OFFLINE] CreateNamedPipe failed");
                while (1) {
                        //Wait for clients
                        if(!ConnectNamedPipe(hServerPipe, NULL)) {
                                if(GetLastError() != ERROR_PIPE_CONNECTED)
                                        continue;
                                BREAK_ON_ERROR("[OFFLINE] ConnectNamedPipe failed");
                        }
                        //recv
                        if (packet_receive_via_pipe(hServerPipe, &packet) != ERROR_SUCCESS)
                                BREAK_ON_ERROR("[OFFLINE] pipe_recv failed");
                        if (packet_get_tlv_string(packet, TLV_TYPE_METHOD, &methodTlv) != ERROR_SUCCESS)
                                BREAK_ON_ERROR("[OFFLINE] packet_get_tlv_string failed");
                }

	} while(0);

        if(hServerPipe) {
                DisconnectNamedPipe(hServerPipe);
                CLOSE_HANDLE(hServerPipe);
        }

}
/*
 * create the received packet
 * /
static DWORD pipe_recv(HANDLE hPipe, Packet **packet)
{
        
        if (packet_receive_via_pipe(hPipe, packet))
*/
static DWORD packet_receive_via_pipe(HANDLE hPipe, Packet **packet)
{
        TlvHeader header                = {0};
        DWORD dwBytes                   = ERROR_SUCCESS;
        DWORD dwResult                  = 0;
        DWORD payloadLength             = 0;
        DWORD payloadBytesLeft          = 0;
        PUCHAR payload                  = NULL;

        *packet = NULL;
        do {
                if (!ReadFile(hPipe, (LPVOID)&header, sizeof(header), &dwBytes, NULL))
                        BREAK_ON_ERROR("[OFFLINE] ReadFile failed");
                if (dwBytes != sizeof(header))
                        BREAK_WITH_ERROR("[OFFLINE] ReadFile incorrect header size", ERROR_INVALID_HANDLE);
                payloadLength = ntohl(header.length) - sizeof(header);
                payloadBytesLeft = payloadLength;
                dwBytes = 0;
                if (!(payload = malloc(payloadLength)))
                        BREAK_ON_ERROR("[OFFLINE] malloc return null");
                while (payloadBytesLeft > 0) {
                        if (!ReadFile(hPipe, (LPVOID)payload+payloadLength-payloadBytesLeft, payloadBytesLeft, &dwBytes, NULL))
                                BREAK_ON_ERROR("[OFFLINE] ReadFile failed");
                        payloadBytesLeft -= dwBytes;
                }
                if (!(*packet = malloc(sizeof(header))))
                        BREAK_ON_ERROR("[OFFLINE] malloc return null");
                *packet->header.length = header.length;
                *packet->header.type = header.type;
                *packet->payload = payload;
                *packet->payloadLength = payloadLength;
        } while (0);

        if (dwResult != ERROR_SUCCESS) {
                if (payload)
                        free(payload);
                if (*packet)
                        free(packet);
        }

        return dwResult;
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

