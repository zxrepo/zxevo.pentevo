#include "std.h"

#include "resource.h"

#include "emul.h"
#include "vars.h"
#include "dx.h"
#include "memory.h"
#include "savesnd.h"

#include "util.h"

//=============================================================================
static unsigned char wavhdr[]=
{
    0x52, 0x49, 0x46, 0x46, 0xCC, 0xF6, 0x3E, 0x00,
    0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20,
    0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00,
    0x22, 0x56, 0x00, 0x00, 0x88, 0x58, 0x01, 0x00,
    0x04, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61,
    0xA8, 0xF6, 0x3E, 0x00
};
//=============================================================================
#pragma pack( push, 1)
//=============================================================================
static struct
{
    unsigned short sig;
    unsigned char stereo;
    unsigned short start;
    unsigned ayfq;
    unsigned char intfq;
    unsigned short year;
    unsigned rawsize;
} vtxheader;
//=============================================================================
#pragma pack( pop)
//=============================================================================
static bool silence(unsigned pos)
{
    return !(vtxbuf[ pos+8] | vtxbuf[ pos+9] | vtxbuf[ pos+10]) ||
	    (vtxbuf[ pos+7] & 0x3F) == 0x3F;
}
//=============================================================================
INT_PTR CALLBACK VtxDlg( HWND dlg, UINT msg, WPARAM wp, LPARAM lp);
//=============================================================================
static unsigned vtxyear;
static unsigned vtxchip;
static char vtxname[200];
static char vtxauthor[200];
static char vtxsoft[200];
static char vtxtracker[200];
static char vtxcomm[200];
//=============================================================================



//=============================================================================
void savesnddialog()
{
    OnEnterGui();	//Alone Coder
    unsigned end;	//Alone Coder 0.36.7
    
    //-------------------------------------------------------------------------
    if (savesndtype)
    {
	//---------------------------------------------------------------------
	// wave
	if (savesndtype == 1)
	{
	    unsigned fsize = unsigned( ftell( savesnd));
	    fseek( savesnd, 0, SEEK_SET);
	    fsize -= sizeof wavhdr;
	    *(unsigned*)(wavhdr + 4) = fsize + 0x2c - 8;
	    *(unsigned*)(wavhdr + 0x28) = fsize;
	    fwrite( wavhdr, 1, sizeof wavhdr, savesnd);
	    MessageBox( wnd, "WAV save done", "Save sound", MB_ICONINFORMATION);
	}
	//---------------------------------------------------------------------
	// vtx
	else
	{ 
	    savesndtype = 0;
	    unsigned char *newb = (unsigned char*)malloc( vtxbuffilled);
	    //-----------------------------------------------------------------
	    for (/*unsigned*/ end = 0;    end < vtxbuffilled && silence( end);    end += 14);
	    //-----------------------------------------------------------------
	    vtxbuffilled -= end;
	    memcpy( vtxbuf, vtxbuf + end, vtxbuffilled);
	    //-----------------------------------------------------------------
	    for (end = vtxbuffilled;    end && silence( end - 14);    end -= 14);
	    //-----------------------------------------------------------------
	    vtxbuffilled = end;
	    int nrec = vtxbuffilled / 14;
	    //-----------------------------------------------------------------
	    for (int i = 0;    i < nrec;    i++)
	    {
		//-------------------------------------------------------------
		for (int j = 0;    j < 14;    j++)
		{
		    newb[ j * nrec + i] = vtxbuf[ i * 14 + j];
		}
		//-------------------------------------------------------------
	    }
	    //-----------------------------------------------------------------
	    free( vtxbuf);
	    FILE *ff = fopen( "vtx.tmp", "wb");
	    //-----------------------------------------------------------------
	    if (!ff)
		goto save_sound_exit;
		//return;
	    //-----------------------------------------------------------------
	    fwrite( newb, 1, vtxbuffilled, ff);
	    fclose( ff);
	    STARTUPINFO si = { sizeof si };
	    si.dwFlags = STARTF_USESHOWWINDOW;
	    si.wShowWindow = SW_HIDE;
	    PROCESS_INFORMATION pi;
	    char Parh[] = "lha a vtx.lzh vtx.tmp";
	    //-----------------------------------------------------------------
	    if ( CreateProcess(		nullptr,
					Parh,
					nullptr,
					nullptr,
					0,
					0,
					nullptr,
					nullptr,
					&si,
					&pi
				)
	     )
	    {
		WaitForSingleObject( pi.hProcess, 5000);
		CloseHandle( pi.hProcess);
		CloseHandle( pi.hThread);
		DeleteFile( "vtx.tmp");
	    }
	    //-----------------------------------------------------------------
	    else
	    {
		DeleteFile( "vtx.tmp");
		MessageBox(	wnd,
				"LHA.EXE not found in %PATH%",
				nullptr,
				MB_ICONERROR
			);
		goto save_sound_exit;
		//return;
	    }
	    //-----------------------------------------------------------------
	    ff = fopen( "vtx.lzh", "rb");
	    //-----------------------------------------------------------------
	    if (!ff)
		goto save_sound_exit;
		//return;
	    //-----------------------------------------------------------------v
	    fseek( ff, 0x22, SEEK_SET);
	    size_t packed = fread( newb, 1, vtxbuffilled, ff) - 1;
	    fclose( ff);
	    DeleteFile( "vtx.lzh");
	    DialogBox( hIn, MAKEINTRESOURCE( IDD_VTX), wnd, VtxDlg);
	    vtxheader.sig = (vtxchip & 1)  ?  WORD2( 'y','m') :
					      WORD2( 'a','y');
	    static unsigned char ste[] = { 1, 2, 0 };
	    vtxheader.stereo = ste[ vtxchip / 2];
	    vtxheader.ayfq = conf.sound.ayfq;
	    vtxheader.intfq = 50;	// хотя для совместимости
					// наверно стоит оставить 50	
	    vtxheader.year = u16( vtxyear);
	    vtxheader.rawsize = vtxbuffilled;
	    fwrite( &vtxheader,	1, 0x10,			savesnd);
	    fwrite( vtxname,	1, strlen( vtxname) + 1,	savesnd);
	    fwrite( vtxauthor,	1, strlen( vtxauthor) + 1,	savesnd);
	    fwrite( vtxsoft,	1, strlen( vtxsoft) + 1,	savesnd);
	    fwrite( vtxtracker,	1, strlen( vtxtracker) + 1,	savesnd);
	    fwrite( vtxcomm,	1, strlen( vtxcomm) + 1,	savesnd);
	    fwrite( newb,	1, packed,			savesnd);
	}
	//---------------------------------------------------------------------
        fclose( savesnd);
        savesndtype = 0;
    }
    //-------------------------------------------------------------------------
    else
    {
	OPENFILENAME ofn = { };
	char sndsavename[0x200];
	*sndsavename = 0;

	ofn.lStructSize = (WinVerMajor < 5)  ?	OPENFILENAME_SIZE_VERSION_400 :
						sizeof( OPENFILENAME);
	ofn.lpstrFilter = "All sound (WAV)\0*.wav\0AY sound (VTX)\0*.vtx\0";
	ofn.lpstrFile = sndsavename;
	ofn.nMaxFile = sizeof sndsavename;
	ofn.lpstrTitle = "Save Sound";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	ofn.hwndOwner = wnd;
	ofn.nFilterIndex = 1;
	//---------------------------------------------------------------------
	if (GetSaveFileName( &ofn))
	{
	    char *name = sndsavename;
	    //-----------------------------------------------------------------
	    for (char *x = name;    *x;    x++)
		if (*x == '\\')
		    name = x + 1;
	    //-----------------------------------------------------------------
	    if (!strchr( name, '.'))
	    {
		//-------------------------------------------------------------
		if (ofn.nFilterIndex == 1)
		{
		    strcat( sndsavename, ".wav");
		}
		//-------------------------------------------------------------
		else
		{
		    strcat( sndsavename, ".vtx");
		}
		//-------------------------------------------------------------
	    }
	    //-----------------------------------------------------------------
	    savesnd = fopen( ofn.lpstrFile, "wb");
	    //-----------------------------------------------------------------
	    if (!savesnd)
	    {
		MessageBox( wnd, "Can't create file", nullptr, MB_ICONERROR);
	    }
	    //-----------------------------------------------------------------
	    // vtx
	    else if (ofn.nFilterIndex == 2)
	    { 
		savesndtype = 2;
		vtxbuf = nullptr;
	    }
	    //-----------------------------------------------------------------
	    // wave. all params, except fq are fixed: 16bit,stereo
	    else
	    { 
		*(unsigned*)(wavhdr + 0x18) = conf.sound.fq; 		// fq
		*(unsigned*)(wavhdr + 0x1C) = conf.sound.fq * 4;	// bitrate
		fwrite( wavhdr, 1, 44, savesnd);			// header
		savesndtype = 1;
	    }
	    //-----------------------------------------------------------------
	}
	//---------------------------------------------------------------------
    }
    //-------------------------------------------------------------------------

save_sound_exit:

/*
    // вынесено в WM_INITMENU
    //-------------------------------------------------------------------------
    HMENU main_menu = GetMenu( wnd);
    //-------------------------------------------------------------------------
    if (main_menu)
    {
	//---------------------------------------------------------------------
	// запись пошла и ее можно остановить
	if (savesndtype)
	{
	    ModifyMenu(	    main_menu,
			    IDM_AUDIOREC,
			    MF_BYCOMMAND | MF_STRING,
			    IDM_AUDIOREC,
			    "Stop Audio Recording"
			);
	}
	//---------------------------------------------------------------------
	// запись можно начать
	else
	{
	    ModifyMenu(	    main_menu,
			    IDM_AUDIOREC,
			    MF_BYCOMMAND | MF_STRING,
			    IDM_AUDIOREC,
			    "Start Audio Recording"
			);
	}
	//---------------------------------------------------------------------
    }
*/
    //-------------------------------------------------------------------------
    eat();
    OnExitGui();	//Alone Coder
}
//=============================================================================



//=============================================================================
INT_PTR CALLBACK VtxDlg(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
    (void)lp;
    //-------------------------------------------------------------------------
    if (msg == WM_INITDIALOG)
    {
	static char chips[] = "ABC AY\0ABC YM\0ACB AY\0ACB YM\0MONO AY\0MONO YM\0";
	//-------------------------------------------------------------------------
	for (char *str = chips;    *str;    str += strlen( str) + 1)
	    SendDlgItemMessage(	    dlg,
				    IDC_VTXCHIP,
				    CB_ADDSTRING,
				    0,
				    (LPARAM)str
				);
	//-------------------------------------------------------------------------
	unsigned c = ((conf.sound.ay_voltab[8] == conf.sound.ay_voltab[9])  ?  0:
									       1);
	// убрано из конца unsigned c
	//		 + (conf.ay_preset > 2  ?  0:
	//					     conf.ay_preset * 2)
	//			
	SendDlgItemMessage(	dlg,
				IDC_VTXCHIP,
				CB_SETCURSEL,
				c,
				0
			);
	SetFocus(	GetDlgItem( dlg, IDE_VTXNAME));
	return 1;
    }
    //-------------------------------------------------------------------------
    if (  ((msg == WM_SYSCOMMAND) && ((wp & 0xFFF0) == SC_CLOSE))	||
	  ((msg == WM_COMMAND) && (LOWORD( wp) == IDOK))
     )
    {
	SendDlgItemMessage( dlg, IDE_VTXNAME,  WM_GETTEXT, sizeof vtxname,    (LPARAM)vtxname);
	SendDlgItemMessage( dlg, IDE_VTXAUTH,  WM_GETTEXT, sizeof vtxauthor,  (LPARAM)vtxauthor);
	SendDlgItemMessage( dlg, IDE_VTXSOFT,  WM_GETTEXT, sizeof vtxsoft,    (LPARAM)vtxsoft);
	SendDlgItemMessage( dlg, IDE_VTXTRACK, WM_GETTEXT, sizeof vtxtracker, (LPARAM)vtxtracker);
	SendDlgItemMessage( dlg, IDE_VTXCOMM,  WM_GETTEXT, sizeof vtxcomm,    (LPARAM)vtxcomm);
	vtxchip = unsigned( SendDlgItemMessage( dlg, IDC_VTXCHIP, CB_GETCURSEL, 0, 0));
	char xx[20];
	SendDlgItemMessage( dlg, IDE_VTXYEAR,  WM_GETTEXT, sizeof xx,         (LPARAM)xx);
	vtxyear = unsigned( atoi( xx));
	EndDialog( dlg, 1);
    }
    return 0;
}
//=============================================================================



//=============================================================================
static unsigned dopoke( int really)
{
    //-------------------------------------------------------------------------
    for (unsigned char *ptr = snbuf;    *ptr;    )
    {
	//---------------------------------------------------------------------
	while (	*ptr == ' '	||
		*ptr == ':'	||
		*ptr == ';'	||
		*ptr == ','
	 )
	{
	    ptr++;
	}
	//---------------------------------------------------------------------
	unsigned num = 0;
	//---------------------------------------------------------------------
	while (isdigit( *ptr))
	    num = num * 10 + (*ptr++ - '0');
	//---------------------------------------------------------------------
	if (num < 0x4000 || num > 0xFFFF)
	    return unsigned( ptr - snbuf + 1);
	//---------------------------------------------------------------------
	while (	*ptr == ' '	||
		*ptr == ':'	||
		*ptr == ';'	||
		*ptr == ','
	 )
	{
	    ptr++;
	}
	//---------------------------------------------------------------------
	unsigned val = 0;
	//---------------------------------------------------------------------
	while (isdigit( *ptr))
	    val = val * 10 + (*ptr++ - '0');
	//---------------------------------------------------------------------
	if (val > 0xFF)
	    return unsigned( ptr - snbuf + 1);
	//---------------------------------------------------------------------
	while (	*ptr == ' '	||
		*ptr == ':'	||
		*ptr == ';'	||
		*ptr == ','
	 )
	{
	    ptr++;
	}
	//---------------------------------------------------------------------
	if (really)
	    cpu.DirectWm( num, u8( val));
	//---------------------------------------------------------------------
    }
    //-------------------------------------------------------------------------
   return 0;
}
//=============================================================================



//=============================================================================
// а чего оно делает в звуке?
INT_PTR CALLBACK pokedlg( HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
    (void)lp;
    //-------------------------------------------------------------------------
    if (msg == WM_INITDIALOG)
    {
	SetFocus( GetDlgItem( dlg, IDE_POKE));
	return 1;
    }
    //-------------------------------------------------------------------------
    if ( (msg == WM_COMMAND && wp == IDCANCEL) ||
         (msg == WM_SYSCOMMAND && (wp & 0xFFF0) == SC_CLOSE)
     )
    {
	EndDialog( dlg, 0);
    }
    //-------------------------------------------------------------------------
    if (msg == WM_COMMAND && LOWORD(wp) == IDOK)
    {
	SendDlgItemMessage(	dlg,
				IDE_POKE,
				WM_GETTEXT,
				/*sizeof snbuf*/ 640 * 480 * 4,
				(LPARAM)snbuf
			);			//Alone Coder 0.36.5
	unsigned r = dopoke(0);
	//---------------------------------------------------------------------
	if (r)
	{
	    MessageBox( dlg, "Incorrect format", nullptr, MB_ICONERROR);
	    SendDlgItemMessage( dlg, IDE_POKE, EM_SETSEL, r, r);
	}
	//---------------------------------------------------------------------
	else
	{
	    dopoke(1);
	    EndDialog( dlg, 0);
	}
	//---------------------------------------------------------------------
    }
    return 0;
}
//=============================================================================

