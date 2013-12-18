#include <string.h>
#include <math.h>
#include <stdio.h>
#include <sem.h>
#include <nonansi.h>
#include <syslib.h>
#include <stdlib.h>
#include <windows.h>
#include <ddeml.h>


#define DDE_CLASS       "DdeTestClass"
#define DDE_INIT        APPCLASS_STANDARD
#define SERVER_HELP     "Math server: Topics: System and Scalar\n\
       System items: Help\n\
       Scalar items: Rand and SRand"

#define IDM_FILE_EXCEL   100
#define IDM_FILE_CLOCK   104
#define IDM_FILE_STOP    101
#define IDM_FILE_SERVER  102
#define IDM_FILE_END     103

static HMENU hMenu;
static HMENU hFilePopup;
static HWND  hWnd;
static DWORD idInst;
static BOOL  bClockRunning;
static Semaphore stop, running, done;

/* server static variables */
static HSZ      hszClockServer;
static HSZ      hszTimeTopic;
static HSZ      hszNowItem;
static HCONV    hNowConv;

static HSZ      hszMathServer;
static HSZ      hszScalarTopic;
static HSZ      hszVectorTopic;
static HSZ      hszRandItem;
static HSZ      hszSRandItem;
static HSZ      hsz1KVectorItem;
static HSZ      hszSystem;
static HSZ      hszHelp;
static DWORD    VectorData[1024];

BOOL InitApplication(HINSTANCE);
BOOL TidyApplication(HINSTANCE);
long MainWndProc(HWND, UINT, WPARAM, LPARAM);
void DoExcel(void);
void StartClock(void);
void EndClock(void);
void RegisterServer(void);
void UnregisterServer(void);
HDDEDATA CALLBACK DdeCallback(UINT, UINT, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD);

int WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
    MSG msg;

    if (!InitApplication(hInst))
        return 255;

    while (GetMessage(&msg, NULL, NULL, NULL))
        DispatchMessage(&msg);

    TidyApplication(hInst);
    return 0;
}

BOOL InitApplication(HINSTANCE hInst)
{
    WNDCLASS wc;
    DWORD    uRes=0;
    UINT     uRet;

    idInst = 0;
    if ((uRet=DdeInitialize(&idInst, DdeCallback, DDE_INIT, uRes)) != DMLERR_NO_ERROR)
    {
        printf("Failed to initialize DDE; ERROR 0x%x\n", uRet);
        return FALSE;
    }
    
    hszSystem = DdeCreateStringHandle(idInst, SZDDESYS_TOPIC, CP_WINANSI);
    hszHelp = DdeCreateStringHandle(idInst, SZDDESYS_ITEM_HELP, CP_WINANSI);
    
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInst;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = COLOR_APPWORKSPACE + 1;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = DDE_CLASS;
    if (!RegisterClass(&wc))
        return FALSE;

    InitSemaphore(&stop, 0);
    InitSemaphore(&running, 0);
    InitSemaphore(&done, 0);
    bClockRunning = FALSE;

    hMenu = CreateMenu();
    hFilePopup = CreatePopupMenu();

    AppendMenu(hMenu, MF_POPUP, hFilePopup, "&File");
    AppendMenu(hFilePopup, MF_STRING, IDM_FILE_EXCEL, "&Excel test");
    AppendMenu(hFilePopup, MF_STRING, IDM_FILE_CLOCK, "&Clock test");
    AppendMenu(hFilePopup, MF_STRING | MF_GRAYED, IDM_FILE_STOP, "&Stop");
    AppendMenu(hFilePopup, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFilePopup, MF_STRING, IDM_FILE_SERVER, "Start s&erver");
    AppendMenu(hFilePopup, MF_STRING | MF_GRAYED, IDM_FILE_END, "E&nd server");

    hWnd = CreateWindow(DDE_CLASS, "DDE Test", WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT,
                CW_USEDEFAULT, CW_USEDEFAULT,
                NULL, hMenu, hInst, NULL);
    ShowWindow(hWnd, SW_NORMAL);
    return TRUE;
}

BOOL TidyApplication(HINSTANCE hInst)
{
    if (IsWindow(hWnd))
        DestroyWindow(hWnd);
    
    DdeFreeStringHandle(idInst, hszSystem);
    DdeFreeStringHandle(idInst, hszHelp);
    
    UnregisterClass(DDE_CLASS, hInst);
    
    DestroyMenu(hFilePopup);
    DestroyMenu(hMenu);
    DdeUninitialize(idInst);
    return TRUE;
}

long MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_COMMAND:
            switch(wParam)
            {
                case IDM_FILE_EXCEL:
                    if (TestSemaphore(&running))
                        WINAPISendMessage(hWnd, WM_COMMAND, IDM_FILE_STOP, 0);
                    else
                        Signal(&running);
                    EnableMenuItem(hFilePopup, IDM_FILE_STOP, MF_ENABLED);
                    Fork(2000, DoExcel, 0);
                    break;

                case IDM_FILE_CLOCK:
                    if (bClockRunning)
                        EndClock();
                    else
                        StartClock();
                    break;

                case IDM_FILE_STOP:
                    if (TestSemaphore(&running))
                    {
                        Signal(&stop);
                        while (!TestSemaphore(&done));
                        Wait(&done);
                        Wait(&running);
                    }
                    EnableMenuItem(hFilePopup, IDM_FILE_STOP, MF_GRAYED);
                    break;

                case IDM_FILE_SERVER:
                    RegisterServer();
                    EnableMenuItem(hFilePopup, IDM_FILE_END, MF_ENABLED);
                    break;

                case IDM_FILE_END:
                    UnregisterServer();
                    EnableMenuItem(hFilePopup, IDM_FILE_END, MF_GRAYED);
                    break;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return DefWindowProc(hWnd, msg, wParam, lParam);

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

void DoExcel(void)
{
    HSZ      hszExcel;
    HSZ      hszRowCol;
    HSZ      hszSelection;
    HSZ      hszSheet;
    HDDEDATA hData;
    HCONV    hConv;
    HCONV    hConvSystem;
    char     string[80], *pos, rowcol[10];
    int      i;
    UINT     uCalc=0;

    hszExcel = DdeCreateStringHandle(idInst, "Excel", CP_WINANSI);
    hszSelection = DdeCreateStringHandle(idInst, "Selection", CP_WINANSI);
    hConvSystem = DdeConnect(idInst, hszExcel, hszSystem, NULL);

    while (!TestSemaphore(&stop))
    {
        hData = DdeClientTransaction("[New(1)]", 9, hConvSystem, NULL, CF_TEXT, XTYP_EXECUTE, 1000, NULL);
        hData = DdeClientTransaction(NULL, 0, hConvSystem, hszSelection, CF_TEXT, XTYP_REQUEST, 1000, NULL);
        DdeGetData(hData, string, 80, 0);
        pos = strchr(string, '!');
        *pos = '\0';

        hszSheet = DdeCreateStringHandle(idInst, string, CP_WINANSI);
        hConv = DdeConnect(idInst, hszExcel, hszSheet, NULL);

        hszRowCol = DdeCreateStringHandle(idInst, "R1C1", CP_WINANSI);
        DdeClientTransaction("0", 2, hConv, hszRowCol, CF_TEXT, XTYP_POKE, 1000, NULL);
        DdeFreeStringHandle(idInst, hszRowCol);
        hszRowCol = DdeCreateStringHandle(idInst, "R2C1", CP_WINANSI);
        DdeClientTransaction("+A1+15", 7, hConv, hszRowCol, CF_TEXT, XTYP_POKE, 1000, NULL);
        strcpy(string, "[select(\"R2C1:R25C1\")][fill.down()]");
        DdeClientTransaction(string, (DWORD)strlen(string)+1, hConv, NULL, CF_TEXT, XTYP_EXECUTE, 1000, NULL);
        DdeFreeStringHandle(idInst, hszRowCol);

        for (i=1; i<=25; i++)
        {
	  int		a = i - 1;
	  double	b = 3.1415926535 * (double)a * 15.0 / 180.0;
	
            sprintf(rowcol, "R%dC2", i);
            switch (uCalc)
            {
            case 0:
                sprintf(string, "%f", sin(b) );
                break;
            case 1:
                sprintf(string, "%f", cos(b) );
                break;
            case 2:
                sprintf(string, "%f", sin(b)+cos(b) );
                break;
            case 3:
                sprintf(string, "%f", sin(b)-cos(b) );
                break;
            case 4:
                sprintf(string, "%f", -sin(b)+cos(b) );
                break;
            case 5:
                sprintf(string, "%f", sin(b)*cos(b) );
                break;
            }
            hszRowCol = DdeCreateStringHandle(idInst, rowcol, CP_WINANSI);
            DdeClientTransaction(string, strlen(string), hConv, hszRowCol, CF_TEXT, XTYP_POKE, 1000, NULL);
            DdeFreeStringHandle(idInst, hszRowCol);
        }
        uCalc = (uCalc + 1) % 6;

        strcpy(string, "[select(\"R1C1:R25C2\")][new(2,2)]");
        DdeClientTransaction(string, (DWORD)strlen(string)+1, hConv, NULL, CF_TEXT, XTYP_EXECUTE, 1000, NULL);

        DdeDisconnect(hConv);
        DdeFreeStringHandle(idInst, hszSheet);
        Delay(OneSec);
        
        if (!TestSemaphore(&stop))
        {
            strcpy(string, "[close(false)][close(false)]");
            DdeClientTransaction(string, (DWORD)strlen(string)+1, hConvSystem, NULL, CF_TEXT, XTYP_EXECUTE, 1000, NULL);
        }
    }

    DdeDisconnect(hConvSystem);
    DdeFreeStringHandle(idInst, hszExcel);
    DdeFreeStringHandle(idInst, hszSelection);

    Wait(&stop);
    Signal(&done);
}

void StartClock(void)
{
    hszClockServer = DdeCreateStringHandle(idInst, "Clock", CP_WINANSI);
    hszTimeTopic = DdeCreateStringHandle(idInst, "Time", CP_WINANSI);
    hszNowItem = DdeCreateStringHandle(idInst, "Now", CP_WINANSI);

    hNowConv = DdeConnect(idInst, hszClockServer, hszTimeTopic, NULL);
    if (!hNowConv)
        return;

    if (!DdeClientTransaction(NULL, 0, hNowConv, hszNowItem, CF_TEXT, XTYP_ADVSTART | XTYPF_ACKREQ, 5000, NULL))
    {
        DdeDisconnect(hNowConv);

        DdeFreeStringHandle(idInst, hszClockServer);
        DdeFreeStringHandle(idInst, hszTimeTopic);
        DdeFreeStringHandle(idInst, hszNowItem);
    }
    else
        bClockRunning = TRUE;
}

void EndClock(void)
{
    DdeClientTransaction(NULL, 0, hNowConv, hszNowItem, CF_TEXT, XTYP_ADVSTOP, 5000, NULL);
    DdeDisconnect(hNowConv);

    DdeFreeStringHandle(idInst, hszClockServer);
    DdeFreeStringHandle(idInst, hszTimeTopic);
    DdeFreeStringHandle(idInst, hszNowItem);

    bClockRunning = FALSE;
}

void RegisterServer(void)
{
    int i;

    hszMathServer = DdeCreateStringHandle(idInst, "Math", CP_WINANSI);
    hszScalarTopic = DdeCreateStringHandle(idInst, "Scalar", CP_WINANSI);
    hszVectorTopic = DdeCreateStringHandle(idInst, "Vector", CP_WINANSI);
    hszRandItem = DdeCreateStringHandle(idInst, "Rand", CP_WINANSI);
    hszSRandItem = DdeCreateStringHandle(idInst, "SRand", CP_WINANSI);
    hsz1KVectorItem = DdeCreateStringHandle(idInst, "1K", CP_WINANSI);

    /* initialize vector */
    for (i=0; i<1024; i++)
        VectorData[i] = (DWORD)__rand();

    DdeNameService(idInst, hszMathServer, NULL, DNS_REGISTER);
}

void UnregisterServer(void)
{
    DdeNameService(idInst, hszMathServer, NULL, DNS_UNREGISTER);

    DdeFreeStringHandle(idInst, hszMathServer);
    DdeFreeStringHandle(idInst, hszScalarTopic);
    DdeFreeStringHandle(idInst, hszVectorTopic);
    DdeFreeStringHandle(idInst, hszRandItem);
    DdeFreeStringHandle(idInst, hszSRandItem);
    DdeFreeStringHandle(idInst, hsz1KVectorItem);
}

HDDEDATA CALLBACK DdeCallback(UINT wType, UINT wFmt, HCONV hConv,
        HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD dwData1, DWORD dwData2)
{
    char str[50];
    HDDEDATA hSendData;
    BOOL bSystem, bScalar, bHelp, bRand, bSRand, bVector, b1K;

/*    
    printf("Callback type 0x%x, FMT = 0x%x, hConv = 0x%x\n", wType, wFmt, hConv);
    DdeQueryString(idInst, hsz1, (LPSTR)str, 50, CP_WINANSI);
    printf("hsz1 = 0x%x,%s\n", hsz1, str);
    DdeQueryString(idInst, hsz2, (LPSTR)str, 50, CP_WINANSI);
    printf("hsz2 = 0x%x,%s\n", hsz2, str);
*/

    bSystem = DdeCmpStringHandles(hsz1, hszSystem) == 0;
    bScalar = !bSystem && (DdeCmpStringHandles(hsz1, hszScalarTopic) == 0);
    bVector = !bSystem && !bScalar && (DdeCmpStringHandles(hsz1, hszVectorTopic) == 0);
    
    bHelp = DdeCmpStringHandles(hsz2, hszHelp) == 0;
    bRand = !bHelp && (DdeCmpStringHandles(hsz2, hszRandItem) == 0);
    bSRand = !bHelp && !bRand && (DdeCmpStringHandles(hsz2, hszSRandItem) == 0);
    b1K = !bHelp && !bRand && !bSRand && (DdeCmpStringHandles(hsz2, hsz1KVectorItem) == 0);

    switch (wType)
    {
        case XTYP_ADVDATA:
            if (hConv == hNowConv)
            {
                DdeGetData(hData, str, 50, 0);
                printf("%s\n", str);
                return DDE_FACK;
            }
            return 0;

        case XTYP_ADVSTART:
            if (b1K)
                return (HDDEDATA)TRUE;
            else
                return (HDDEDATA)FALSE;

        case XTYP_ADVREQ:
            if (bVector && b1K)
            {
                hSendData = DdeCreateDataHandle(idInst, VectorData,
                        sizeof(DWORD)*1024, 0, hsz1KVectorItem, CF_TEXT, 0);
                return hSendData;
            }
            break;
        
        case XTYP_CONNECT_CONFIRM:
            if (!(bSystem || bScalar || bVector))
                DdeDisconnect(hConv);
            break;

        case XTYP_REQUEST:
            if (bSystem && bHelp)
            {
                hSendData = DdeCreateDataHandle(idInst, SERVER_HELP, 
                        (DWORD)strlen(SERVER_HELP)+1, 0, hszHelp, CF_TEXT, 0);
                return hSendData;
            }
            if (bScalar && bRand)
            {
                sprintf(str, "%g", (double)rand() / (double)RAND_MAX);
                hSendData = DdeCreateDataHandle(idInst, str, 
                        (DWORD)strlen(str)+1, 0, hszRandItem, CF_TEXT, 0);
                return hSendData;
            }
            if (bVector && b1K)
            {
                hSendData = DdeCreateDataHandle(idInst, VectorData,
                        sizeof(DWORD)*1024, 0, hsz1KVectorItem, CF_TEXT, 0);
                return hSendData;
            }
            break;

        case XTYP_EXECUTE:
            if (bSystem || bScalar)
            {
                char *pos, *end;
                BOOL OK = TRUE;
                HSZ hszTest;

                DdeGetData(hData, str, 50, 0);
                printf("In execute, str = '%s'\n", str);

                pos = strchr(str, '[');
                if (pos == NULL)
                    break;
                printf("found pos of [\n");

                do {
                    printf("in while\n");
                    pos++;
                    end = strchr(pos, ']');
                    if (end == NULL)
                    {
                        OK = FALSE;
                        break;
                    }
                    *end = '\0';
                    printf("creating string '%s'\n", pos);
                    hszTest = DdeCreateStringHandle(idInst, pos, CP_WINANSI);

                    OK = OK & !DdeCmpStringHandles(hszTest, hszHelp);
                    if (OK)
                        printf("compared OK\n");

                    DdeFreeStringHandle(idInst, hszTest);
                    *end = ']';
                } while (((pos = strchr(pos, '[')) != NULL) && OK);

                if (OK)
                    return DDE_FACK;
            }
            break;

        case XTYP_POKE:
            if (bScalar && bSRand)
            {
                int _new;
                DdeGetData(hData, str, 50, 0);
                _new = atoi(str);
                srand(_new);
                return DDE_FACK;
            }
            if (bVector && b1K)
            {
                DdeGetData(hData, VectorData, sizeof(DWORD)*1024, 0);
                DdePostAdvise(idInst, hszVectorTopic, hsz1KVectorItem);
                return DDE_FACK;
            }
            break;
    }
    return 0;
}

