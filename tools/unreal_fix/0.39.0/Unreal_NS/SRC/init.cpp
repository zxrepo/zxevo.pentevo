#include "std.h"
#include <time.h>	// NEDOREPO

#include "emul.h"
#include "vars.h"
#include "config.h"
#include "dx.h"
#include "draw.h"
#include "iehelp.h"
#include "gs.h"
#include "leds.h"
#include "tape.h"
#include "emulkeys.h"
#include "sshot_png.h"
#include "init.h"
#include "snapshot.h"
#include "savesnd.h"
#include "fdd.h"
#include "z80/tables.h"
#include "dbgbpx.h"

#include "util.h"

#include "zxusbnet.h"	// NEDOREPO


#include "midi.h"			// [NS]
#include "emul_no_modal_windows.h"	// hwnd-� ����������� ���� [NS]



//=============================================================================
static void cpu_info()
{
    char idstr[64];
    idstr[0] = 0;

    fillCpuString(idstr);

    trim(idstr);

    unsigned cpuver = cpuid( 1, 0);
    unsigned features = cpuid( 1, 1);
    temp.mmx = (features >> 23) & 1;
    temp.sse = (features >> 25) & 1;
    temp.sse2 = (features >> 26) & 1;

    unsigned features_apm = 0;
    bool features_apm_supported = false;
    unsigned max_ext_id = cpuid( 0x80000000, 0);
   
    //-------------------------------------------------------------------------
    if (max_ext_id >= 0x80000007)
    {
        features_apm_supported = true;
        features_apm = cpuid( 0x80000007, 1);
    }
    //-------------------------------------------------------------------------
    temp.cpufq = GetCPUFrequency();

    color( CONSCLR_DEFAULT);   printf("CPU: ");
    color( CONSCLR_HARDINFO);  printf("%s\n", idstr);

    color( CONSCLR_HARDITEM);
//  printf("%u.%u.%u [MMX:%s, SSE:%s, SSE2:%s, TSCINV:%s] ",	
    //-------------------------------------------------------------------------
    printf("     %u.%u.%u\n",	((cpuver >> 8) & 0x0F),
				((cpuver >> 4) & 0x0F),
				((cpuver     ) & 0x0F)  );
    //-------------------------------------------------------------------------
    printf("     MMX:    %s\n",	temp.mmx   ?	"YES" :
						"NO"	);
    //-------------------------------------------------------------------------
    printf("     SSE:    %s\n", temp.sse   ?	"YES" :
						"NO"	);
    //-------------------------------------------------------------------------
    printf("     SSE2:   %s\n", temp.sse2   ?	"YES" :
						"NO"	);
    //-------------------------------------------------------------------------
    printf("     TSCINV: %s\n",	features_apm_supported    ?	((features_apm & (1 << 8))   ?	"YES" :
												"NO") :
								"UNK");
    //-------------------------------------------------------------------------
    printf("     TSC at %u MHz\n", unsigned( temp.cpufq / 1000000U));
    //-------------------------------------------------------------------------v
#ifdef MOD_SSE2
    //-------------------------------------------------------------------------
    if (!temp.sse2)
    {
	color( CONSCLR_WARNING);
	printf("warning: this is an SSE2 build, recompile or download non-P4 version\n");
    }
    //-------------------------------------------------------------------------
#else 
    //-------------------------------------------------------------------------
    // MOD_SSE2
    if (temp.sse2)
    {
	color( CONSCLR_WARNING);
	printf("warning: SSE2 disabled in compile-time, recompile or download P4 version\n");
    }
    //-------------------------------------------------------------------------
#endif
}
//=============================================================================


//=============================================================================
static void restrict_version(char legacy)
{
    (void)legacy;
//   color(CONSCLR_WARNING);
//   printf ("WARNING: Windows 95/98/Me is not fully supported.\n");
    temp.win9x = 1; //Dexus
    color();

/*   static const char vererror[] = "unsupported OS version";

   if (!legacy) errexit(vererror);

   #ifdef MOD_9X
   static const char verwarning[] =
      "detected windows version is not supported\r\n"
      "by current version of UnrealSpeccy and untested.\r\n\r\n"
      "you may click 'YES' to continue on your own risk.\r\n"
      "in this case you will experience crashes under\r\n"
      "some conditions. it's an OS-specific problem, please\r\n"
      "don't report it and consider using NT-based system";
   if (MessageBox(0, verwarning, vererror, MB_ICONERROR | MB_YESNO | MB_DEFBUTTON2) == IDNO) errexit(vererror);
   #endif
*/
}
//=============================================================================


 
//=============================================================================
void init_all(int argc, char **argv)
{
//   printf("%s\n", __FUNCTION__);

//	int main_pause_flag;
//	main_pause_flag = 0;	//NS

    conf.win_resize_request = 1;	// ��������� ��������� ������� [NS]
    conf.win_static_size_x = 777;
    conf.win_static_size_y = 555;
	
    conf.trace_follow_regs = REG_PC;	// ������� � �������� �� PC �� ������� [NS]
    conf.trace_follow_request = 1;	// � ������������� ��� ���� �� ���� PC ��������
    
    // �� ������ ������ �������������� hwnd-� �� ��������� ����
    hwnd_bp_dialog = NULL;	// hwnd ������������ ���� ������ [NS]
    hwnd_watchdlg = NULL;	// hwnd ������������ ���� ������� [NS]
    
    cpu_info();

    char *config = nullptr;
    char legacy = 0;
    //-------------------------------------------------------------------------
    for (int i = 0; i < argc; i++)
    {
	//---------------------------------------------------------------------
	if (argv[i][0] != '/' && argv[i][0] != '-')
	    continue;
	//---------------------------------------------------------------------
	if (!stricmp(argv[i] + 1, "i") && i + 1 < argc)
	{
	    config = argv[i + 1];
	    i++;
	}
	//---------------------------------------------------------------------
	#ifdef MOD_9X
	if (argv[i][1] == '9')
	    legacy = 1;
	#endif
	//---------------------------------------------------------------------
    }
    //-------------------------------------------------------------------------

    temp.Minimized = false;
    temp.win9x = 0; //Dexus
    //-------------------------------------------------------------------------
    if (GetVersion() >> 31)
        restrict_version(legacy);
    //-------------------------------------------------------------------------
    init_z80tables();
    video_permanent_tables();	//��������� ������ ��� �����?
    init_ie_help();
    load_config( config);
    //-------------------------------------------------------------------------
    {   //clear memory				//NEDOREPO
	unsigned char *tptr = RAM_BASE_M;
	char * a;
	//---------------------------------------------------------------------
	union
	{
	    unsigned long long int l;
	    unsigned char b[8];
	}
	pattern;
	//---------------------------------------------------------------------
	int i = 0;
	pattern.l = strtoul(conf.cold_ram_pat, &a, 16);
	int j = (a - conf.cold_ram_pat) >> sizeof(conf.cold_ram_pat[0]);
	srand((unsigned int)time(NULL));
	//---------------------------------------------------------------------
	while (tptr != (RAM_BASE_M + (MAX_RAM_PAGES * PAGE)))
	{
	    //-----------------------------------------------------------------
	    if ((rand() & 0xFF03) == 0)
	    {
		*tptr=(unsigned char)(rand()&0xff);
	    }
	    //-----------------------------------------------------------------
	    else
	    {
		*tptr=pattern.b[i];
	    }
	    //-----------------------------------------------------------------
	    tptr++;
	    //-----------------------------------------------------------------
	    if ((++i) == j)
		i = 0;
	    //-----------------------------------------------------------------
	}
	//---------------------------------------------------------------------
    }
    //-------------------------------------------------------------------------
//  make_samples();	//�� ����� ���������� ay samples
    init_leds();
    init_tape();
    init_hdd_cd();
    start_dx();		// ������� ������� ����
    set_priority();
    applyconfig();
    main_reset();
    autoload();
    init_bpx();
    //-------------------------------------------------------------------------
    if (conf.MIDI_128_Out)	// [NS]
    {
	midi_init();		// ���� ���������� ������ �� �������
    }
    //-------------------------------------------------------------------------
//  if (conf.wiznet)	//����������� � ����� NEDOREPO
//  { 		
//	Wiz5300_Init(); 
//  } 
    //-------------------------------------------------------------------------
    // libpng12.dll
    temp.PngSupport = PngInit();
    //-------------------------------------------------------------------------
    if (!temp.PngSupport)
    {
	color(CONSCLR_WARNING);
	printf("warning: libpng12.dll not found or wrong version -> png support disabled\n");
	//---------------------------------------------------------------------
	if (conf.scrshot == 2)	// ��� ������ ��� ���������� .png
	{
	    conf.scrshot = 1;	// ������ �� .bmp (�.�. ����������� libpng)
	}
    }
    //-------------------------------------------------------------------------
    // zlib1.dll
    temp.ZlibSupport = ZlibInit();
    //-------------------------------------------------------------------------
    if (!temp.ZlibSupport)
    {
	color(CONSCLR_WARNING);
	printf("warning: zlib1.dll not found or wrong version -> csw 2.0 support disabled\n");
    }
    temp.LastSnapName[0] = 0;
    load_errors = 0;
    trd_toload = 0;
    *(DWORD*)trd_loaded = 0;	// clear loaded flags, don't see autoload'ed images

    //-------------------------------------------------------------------------
    for (    ; argc;    argc--, argv++)
    {
	//---------------------------------------------------------------------
	if (**argv == '-' || **argv == '/')
	{
	    if (argc > 1 && !stricmp(argv[0] + 1, "i"))
	    {
		argc--;
		argv++;
	    }
	    continue;
	}
	//---------------------------------------------------------------------

	char fname[0x200];
	char *temp;
	GetFullPathName( *argv, sizeof fname, fname, &temp);

	trd_toload = DefaultDrive; // auto-select
	//---------------------------------------------------------------------
	if (!loadsnap(fname))
	{
	    errmsg("error loading <%s>", *argv);
	    load_errors = 1;
	}
	//---------------------------------------------------------------------
    }
    //-------------------------------------------------------------------------
    if (load_errors)
    {
	int code = MessageBox(	wnd,
				"Some files, specified in\r\ncommand line, failed to load\r\n\r\nContinue emulation?",
				"File loading error",
				MB_YESNO | MB_ICONWARNING
			      );
	//---------------------------------------------------------------------
	if (code != IDYES)
	    exit();
	//---------------------------------------------------------------------
    }
    //-------------------------------------------------------------------------
    SetCurrentDirectory(conf.workdir);
    //-------------------------------------------------------------------------
    if (conf.HighResolutionTimer)
	timeBeginPeriod(1);
    //-------------------------------------------------------------------------
}
//=============================================================================


//=============================================================================
void __declspec(noreturn) exit()
{

//   EnableMenuItem(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_ENABLED);

#ifdef _DEBUG //---------------------------------------------------------------
    if (!normal_exit)
    {
	__debugbreak();
    }
#endif //----------------------------------------------------------------------

    exitflag = 1;
    //-------------------------------------------------------------------------
    if (savesndtype)
	savesnddialog();  //stop saving sound
    //-------------------------------------------------------------------------
    if (videosaver_state)
	main_savevideo(); //stop saving video
    //-------------------------------------------------------------------------
    if (!normal_exit)
	done_fdd(false);
    //-------------------------------------------------------------------------
    done_tape();
    done_dx();
    done_gs();
    done_leds();
    save_nv();
    modem.close();
    done_ie_help();
    done_bpx();
    PngDone();
    ZlibDone();
    //-------------------------------------------------------------------------
    if (conf.HighResolutionTimer)
	timeEndPeriod(1);
    //-------------------------------------------------------------------------
    if (ay[1].Chip2203) YM2203Shutdown( ay[1].Chip2203); //Dexus
    if (ay[0].Chip2203) YM2203Shutdown( ay[0].Chip2203); //Dexus
    //-------------------------------------------------------------------------
    color();
    printf("\nsee you later!\n");
    //-------------------------------------------------------------------------
    if (!nowait)
    {
	SetConsoleTitle( "press a key...");
	FlushConsoleInputBuffer( GetStdHandle( STD_INPUT_HANDLE));
	getch();
    }
    //-------------------------------------------------------------------------
    fflush( stdout);
    SetConsoleCtrlHandler( ConsoleHandler, FALSE);
    exit(0);
}
//=============================================================================

