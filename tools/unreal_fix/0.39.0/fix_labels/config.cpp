#include "std.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "emul.h"
#include "vars.h"
#include "memory.h"
#include "debug.h"
#include "dbglabls.h"
#include "draw.h"
#include "dx.h"
#include "fontatm2.h"
#include "snapshot.h"
#include "sndrender/sndcounter.h"
#include "sndrender/dev_moonsound.h"
#include "sound.h"
#include "sdcard.h"
#include "gs.h"
#include "zc.h"
#include "util.h"
#include "init.h"
#include "config.h"
#include "zxusbnet.h"

char load_errors;

void loadkeys(action*);
void loadzxkeys(CONFIG*);
void load_arch(const char*);

static unsigned load_rom(const char *path, unsigned char *bank, unsigned max_banks = 1)
{
   if (!*path) { norom: memset(bank, 0xFF, max_banks*PAGE); return 0; }
   char tmp[FILENAME_MAX]; strcpy(tmp, path);
   char *x = strrchr(tmp+2, ':');
   unsigned page = 0;
   if (x) { *x = 0; page = unsigned(atoi(x+1)); }
   if (max_banks == 16) page *= 16; // bank for scorp prof.rom

   FILE *ff = fopen(tmp, "rb");

   if (!ff) {
      errmsg("ROM file %s not found", tmp);
   err:
      load_errors = 1;
      goto norom;
   }

   if (fseek(ff, long(page*PAGE), SEEK_SET)) {
badrom:
      fclose(ff);
      errmsg("Incorrect ROM file: %s", path);
      goto err;
   }

   size_t size = fread(bank, 1, max_banks*PAGE, ff);
   if (!size || (size & (PAGE-1))) goto badrom;

   fclose(ff);
   return unsigned(size / 1024);
}

void load_atm_font()
{
   FILE *ff = fopen("SGEN.ROM", "rb");
   if (!ff)
   {
       memcpy(fontatm2, fontatm2_default, sizeof(fontatm2));
       return;
   }
   unsigned char font[0x800];
   size_t sz = fread(font, 1, 0x800, ff);
   if (sz == 0x800) {
      color(CONSCLR_INFO);
      printf("using ATM font from external SGEN.ROM\n");
      for (unsigned chr = 0; chr < 0x100; chr++)
         for (unsigned l = 0; l < 8; l++)
            fontatm2[chr+l*0x100] = font[chr*8+l];
   }
   fclose(ff);
}

static void load_atariset()
{
   memset(temp.ataricolors, 0, sizeof temp.ataricolors);
   if (!conf.atariset[0])
       return;
   char defs[4000]; *defs = 0; // =12*256, strlen("07:aabbccdd,")=12
   char keyname[80];
   sprintf(keyname, "atari.%s", conf.atariset);
   GetPrivateProfileString("COLORS", keyname, nil, defs, sizeof defs, ininame);
   if (!*defs)
       conf.atariset[0] = 0;
   for (char *ptr = defs; *ptr; )
   {
      if (ptr[2] != ':')
          return;
      for (int i = 0; i < 11; i++)
         if (i != 2 && !ishex(ptr[i]))
             return;
      unsigned index, val;
      sscanf(ptr, "%02X:%08X", &index, &val);
      temp.ataricolors[index] = val;
      // temp.ataricolors[(index*16 + index/16) & 0xFF] = val; // swap ink-paper
      ptr += 12;
      if (ptr [-1] != ',')
          return;
   }
}

void addpath(char *dst, const char *fname)
{
   if (!fname)
       fname = dst;
   else
       strcpy(dst, fname);
   if (!*fname)
       return; // empty filenames have special meaning
   if (fname[1] == ':' || (fname[0] == '\\' || fname[1] == '\\'))
       return; // already full name

   char tmp[FILENAME_MAX];
   GetModuleFileName(nullptr, tmp, sizeof tmp);
   char *xx = strrchr(tmp, '\\');
   if (*fname == '?')
       *xx = 0; // "?" to get exe directory
   else
       strcpy(xx+1, fname);
   strcpy(dst, tmp);
}

void save_nv()
{
   char line[FILENAME_MAX]; addpath(line, "CMOS");
   FILE *f0 = fopen(line, "wb");
   if(f0)
   {
       fwrite(cmos, 1, sizeof cmos, f0);
       fclose(f0);
   }

   addpath(line, "NVRAM");
   if((f0 = fopen(line, "wb")))
   {
       fwrite(nvram, 1, sizeof nvram, f0);
       fclose(f0);
   }
}

void load_romset(CONFIG *conf, const char *romset)
{
   char sec[0x200];
   sprintf(sec, "ROM.%s", romset);
   GetPrivateProfileString(sec, "sos", nil, conf->sos_rom_path, sizeof conf->sos_rom_path, ininame);
   GetPrivateProfileString(sec, "dos", nil, conf->dos_rom_path, sizeof conf->dos_rom_path, ininame);
   GetPrivateProfileString(sec, "128", nil, conf->zx128_rom_path, sizeof conf->zx128_rom_path, ininame);
   GetPrivateProfileString(sec, "sys", nil, conf->sys_rom_path, sizeof conf->sys_rom_path, ininame);
   addpath(conf->sos_rom_path);
   addpath(conf->dos_rom_path);
   addpath(conf->zx128_rom_path);
   addpath(conf->sys_rom_path);
}

static void add_presets(const char *section, const char *prefix0, unsigned *num, char **tab, unsigned char *curr)
{
   *num = 0;
   char buf[0x7F00], defval[64];
   GetPrivateProfileSection(section, buf, sizeof buf, ininame);
   GetPrivateProfileString(section, prefix0, "none", defval, sizeof defval, ininame);
   char *p = strchr(defval, ';');
   if (p) *p = 0;

   for (p = defval+strlen(defval)-1; p>=defval && *p == ' '; *p-- = 0);

   char prefix[0x200];
   strcpy(prefix, prefix0);
   strcat(prefix, ".");
   size_t plen = strlen(prefix);
   for (char *ptr = buf; *ptr; )
   {
      if (!strnicmp(ptr, prefix, plen))
      {
         ptr += plen;
         tab[*num] = setptr;
         while (*ptr && *ptr != '=')
             *setptr++ = *ptr++;
         *setptr++ = 0;

         if (!stricmp(tab[*num], defval))
             *curr = (unsigned char)*num;
         (*num)++;
      }
      while (*ptr) ptr++;
      ptr++;
   }
}

void load_ula_preset()
{
   if (conf.ula_preset >= num_ula) return;
   char line[128], name[64];
   sprintf(name, "PRESET.%s", ulapreset[conf.ula_preset]);
   static char defaults[] = "71680,17989,224,50,32,0,0,0,0,0,320,240,24,32,384,288,48,64";
   GetPrivateProfileString("ULA", name, defaults, line, sizeof line, ininame);
   unsigned t1, t2, t3, t4, t5;
   sscanf(line, "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u", &/*conf.frame*/frametime/*Alone Coder*/, &conf.paper,
       &conf.t_line, &conf.intfq, &conf.intlen, &t1, &t2, &t3, &t4, &t5,
       &conf.mcx_small, &conf.mcy_small, &conf.b_top_small, &conf.b_left_small,
       &conf.mcx_full, &conf.mcy_full, &conf.b_top_full, &conf.b_left_full);
   conf.even_M1 = (unsigned char)t1; conf.border_4T = (unsigned char)t2;
   conf.floatbus = (unsigned char)t3; conf.floatdos = (unsigned char)t4;
   conf.portff = t5 & 1;

   if(conf.mcx_small < 256)
       conf.mcx_small = 256;
   if(conf.mcy_small < 192)
       conf.mcy_small = 192;

   if(conf.mcx_full < 256)
       conf.mcx_full = 256;
   if(conf.mcy_full < 192)
       conf.mcy_full = 192;

   if(conf.b_left_full & 7)
   {
       err_printf("PRESET.%s:b_left_full not multiple of 8\n", ulapreset[conf.ula_preset]);
       exit();
   }
}

void load_ay_stereo()
{
   char line[128], name[64]; sprintf(name, "STEREO.%s", aystereo[conf.sound.ay_stereo]);
   GetPrivateProfileString("AY", name, "100,10,66,66,10,100", line, sizeof line, ininame);
   unsigned *stereo = conf.sound.ay_stereo_tab;
   sscanf(line, "%u,%u,%u,%u,%u,%u", stereo+0, stereo+1, stereo+2, stereo+3, stereo+4, stereo+5);
}

void load_ay_vols()
{
   char line[512] = { 0 };
   static char defaults[] = "0000,0340,04C0,06F2,0A44,0F13,1510,227E,289F,414E,5B21,7258,905E,B550,D7A0,FFFF";
   char name[64]; sprintf(name, "VOLTAB.%s", ayvols[conf.sound.ay_vols]);
   GetPrivateProfileString("AY", name, defaults, line, sizeof line, ininame);
   if (line[74] != ',') strcpy(line, defaults);
   if (line[79] == ',') { // YM
      for (int i = 0; i < 32; i++)
         sscanf(line+i*5, "%X", &conf.sound.ay_voltab[i]);
   } else { // AY
       for(int i = 0; i < 16; i++)
       {
           sscanf(line + i * 5, "%X", &conf.sound.ay_voltab[2 * i]);
           conf.sound.ay_voltab[2 * i + 1] = conf.sound.ay_voltab[2 * i];
       }
   }
}

#if (defined _MSC_VER)
int strcasecmp(char const *s1, char const *s2)
{
	return ::_stricmp(s1, s2);
}
#endif


void load_config(const char *fname)
{
   char line[FILENAME_MAX];
   load_errors = 0;

   GetModuleFileName(nullptr, ininame, sizeof(ininame));

   //strlwr(ininame); // WHAT'S THIS FOR???
   
   // this was a flawed code since strstr searches FIRST occurence!
   //*(unsigned*)(strstr(ininame, ".exe")+1) = WORD4('i','n','i',0);

   // make default config name: for EXENAME64.exe or EXENAME.exe that will be EXENAME.ini
   size_t ininame_len = strlen(ininame);
   const char const_64_exe[] = "64.exe";
   const char const_exe[] = ".exe";
   const char const_ini[] = ".ini";
   //
   if( !strcasecmp(ininame + ininame_len - sizeof(const_64_exe) + 1, const_64_exe) )
   	memcpy(ininame + ininame_len - sizeof(const_64_exe) + 1, const_ini, sizeof(const_ini)); // fix last "64.exe" to ".ini"
   else if( !strcasecmp(ininame + ininame_len - sizeof(const_exe) + 1, const_exe) )
   	memcpy(ininame + ininame_len - sizeof(const_exe) + 1, const_ini, sizeof(const_ini)); // fix last ".exe" to ".ini"

   if (fname && *fname) {
      char *dst = strrchr(ininame, '\\');
      if (strchr(fname, '/') || strchr(fname, '\\')) dst = ininame; else dst++;
      strcpy(dst, fname);
   }
   color(CONSCLR_DEFAULT); printf("ini: ");
   color(CONSCLR_INFO);    printf("%s\n", ininame);

   if (GetFileAttributes(ininame) == INVALID_FILE_ATTRIBUTES) errexit("config file not found");

   static const char* misc = "MISC";
   static const char* video = "VIDEO";
   static const char* ula = "ULA";
   static const char* beta128 = "Beta128"; 
   static const char* USBZXNET = "USBZXNET"; 
   static const char* leds = "LEDS";
   static const char* sound = "SOUND";
   static const char* input = "INPUT";
   static const char* colors = "COLORS";
   static const char* ay = "AY";
   static const char* saa1099 = "SAA1099";
   static const char* hdd = "HDD";
   static const char* rom = "ROM";
   static const char* ngs = "NGS";
   static const char* zc = "ZC";

   #ifdef MOD_MONITOR
   addpath(conf.sos_labels_path, "sos.l");
   #endif

   GetPrivateProfileString("*", "UNREAL", nil, line, sizeof line, ininame);
   unsigned a,b,c;
   sscanf(line, "%u.%u.%u", &a, &b, &c);
   if ((((a << 8U) | b) != VER_HL) || (c != (VER_A & 0x7F)))
       errexit("wrong ini-file version");

   GetPrivateProfileString(misc, "Help",  "help_eng.html", helpname, sizeof helpname, ininame);
   addpath(helpname);

   if (GetPrivateProfileInt(misc, "HideConsole", 0, ininame))
   {
       FreeConsole();
       nowait = 1;
   }

   conf.ConfirmExit = u8(GetPrivateProfileInt(misc, "ConfirmExit", 0, ininame));

   conf.sleepidle = u8(GetPrivateProfileInt(misc, "ShareCPU", 0, ininame));
   conf.highpriority = u8(GetPrivateProfileInt(misc, "HighPriority", 0, ininame));
   GetPrivateProfileString(misc, "SyncMode", "SOUND", line, sizeof line, ininame);
   conf.SyncMode = SM_SOUND;
   if(!strnicmp(line, "SOUND", 5))
       conf.SyncMode = SM_SOUND;
   else if(!strnicmp(line, "VIDEO", 5))
       conf.SyncMode = SM_VIDEO;
   else if(!strnicmp(line, "TSC", 3))
       conf.SyncMode = SM_TSC;
   conf.HighResolutionTimer = GetPrivateProfileInt(misc, "HighResolutionTimer", 0, ininame);
   conf.tape_traps = u8(GetPrivateProfileInt(misc, "TapeTraps", 1, ininame));
   conf.tape_autostart = u8(GetPrivateProfileInt(misc, "TapeAutoStart", 1, ininame));
   conf.EFF7_mask = u8(GetPrivateProfileInt(misc, "EFF7mask", 0, ininame));

   GetPrivateProfileString(rom, "ATM1", nil, conf.atm1_rom_path, sizeof conf.atm1_rom_path, ininame);
   GetPrivateProfileString(rom, "ATM2", nil, conf.atm2_rom_path, sizeof conf.atm2_rom_path, ininame);
   GetPrivateProfileString(rom, "ATM3", nil, conf.atm3_rom_path, sizeof conf.atm3_rom_path, ininame);
   GetPrivateProfileString(rom, "SCORP", nil, conf.scorp_rom_path, sizeof conf.scorp_rom_path, ininame);
   GetPrivateProfileString(rom, "PROFROM", nil, conf.prof_rom_path, sizeof conf.prof_rom_path, ininame);
   GetPrivateProfileString(rom, "PROFI", nil, conf.profi_rom_path, sizeof conf.profi_rom_path, ininame);
//[vv]   GetPrivateProfileString(rom, "KAY", nil, conf.kay_rom_path, sizeof conf.kay_rom_path, ininame);
   GetPrivateProfileString(rom, "PLUS3", nil, conf.plus3_rom_path, sizeof conf.plus3_rom_path, ininame);
   GetPrivateProfileString(rom, "QUORUM", nil, conf.quorum_rom_path, sizeof conf.quorum_rom_path, ininame);
   #ifdef MOD_GSZ80
   GetPrivateProfileString(rom, "GS", nil, conf.gs_rom_path, sizeof conf.gs_rom_path, ininame);
   addpath(conf.gs_rom_path);
   #endif
   GetPrivateProfileString(rom, "MOONSOUND", nil, conf.moonsound_rom_path, sizeof conf.moonsound_rom_path, ininame);
   addpath(conf.moonsound_rom_path);
   addpath(conf.atm1_rom_path);
   addpath(conf.atm2_rom_path);
   addpath(conf.atm3_rom_path);
   addpath(conf.scorp_rom_path);
   addpath(conf.prof_rom_path);
   addpath(conf.profi_rom_path);
   addpath(conf.plus3_rom_path);
   addpath(conf.quorum_rom_path);
//[vv]   addpath(conf.kay_rom_path);

   GetPrivateProfileString(rom, "ROMSET", "default", line, sizeof line, ininame);
   if(*line)
   {
       load_romset(&conf, line); conf.use_romset = 1;
   }
   else
   {
       conf.use_romset = 0;
   }

   conf.smuc = u8(GetPrivateProfileInt(misc, "SMUC", 0, ininame));
   GetPrivateProfileString(misc, "CMOS", nil, line, sizeof line, ininame);
   conf.cmos = 0;
   if (!strnicmp(line, "DALLAS", 6)) conf.cmos=1;
   if (!strnicmp(line, "512Bu1", 6)) conf.cmos=2;
   conf.cache = u8(GetPrivateProfileInt(misc, "Cache", 0, ininame));
   if (conf.cache && conf.cache!=16 && conf.cache!=32) conf.cache = 0;
   GetPrivateProfileString(misc, "HIMEM", nil, line, sizeof line, ininame);
   conf.mem_model = MM_PENTAGON;
   unsigned i; //Alone Coder 0.36.7
   for (/*unsigned*/ i = 0; i < N_MM_MODELS; i++)
      if (!strnicmp(line, mem_model[i].shortname, strlen(mem_model[i].shortname)))
         conf.mem_model = mem_model[i].Model;
   conf.ramsize = GetPrivateProfileInt(misc, "RamSize", 128, ininame);
   conf.Sna128Lock = GetPrivateProfileInt(misc, "Sna128Lock", 1, ininame);

   GetPrivateProfileString(misc, "DIR", nil, conf.workdir, sizeof conf.workdir, ininame);

   GetCurrentDirectory(_countof(line), line);
   SetCurrentDirectory(conf.workdir);
   GetCurrentDirectory(_countof(temp.SnapDir), temp.SnapDir);
   SetCurrentDirectory(line);
   strcpy(temp.RomDir, temp.SnapDir);
   strcpy(temp.HddDir, temp.SnapDir);
   strcpy(temp.SdDir, temp.SnapDir);

   conf.reset_rom = RM_SOS;
   GetPrivateProfileString(misc, "RESET", nil, line, sizeof line, ininame);
   if (!strnicmp(line, "DOS", 3)) conf.reset_rom = RM_DOS;
   if (!strnicmp(line, "MENU", 4)) conf.reset_rom = RM_128;
   if (!strnicmp(line, "SYS", 3)) conf.reset_rom = RM_SYS;

   GetPrivateProfileString(misc, "Modem", nil, line, sizeof line, ininame);
   conf.modem_port = 0;

   sscanf(line, "COM%d", &conf.modem_port);

   conf.paper = GetPrivateProfileInt(ula, "Paper", 17989, ininame);
   conf.t_line = GetPrivateProfileInt(ula, "Line", 224, ininame);
   conf.intfq = GetPrivateProfileInt(ula, "int", 50, ininame);
   conf.intlen = GetPrivateProfileInt(ula, "intlen", 32, ininame);
   /*conf.frame*/frametime/*Alone Coder*/ = GetPrivateProfileInt(ula, "Frame", 71680, ininame);
   conf.border_4T = u8(GetPrivateProfileInt(ula, "4TBorder", 0, ininame));
   conf.even_M1 = u8(GetPrivateProfileInt(ula, "EvenM1", 0, ininame));
   conf.floatbus = u8(GetPrivateProfileInt(ula, "FloatBus", 0, ininame));
   conf.floatdos = u8(GetPrivateProfileInt(ula, "FloatDOS", 0, ininame));
   conf.portff = GetPrivateProfileInt(ula, "PortFF", 0, ininame) != 0;
   conf.mcx_small = GetPrivateProfileInt(ula, "mcx_small", 320, ininame);
   conf.mcy_small = GetPrivateProfileInt(ula, "mcy_small", 240, ininame);
   conf.b_top_small = GetPrivateProfileInt(ula, "b_top_small", 24, ininame);
   conf.b_left_small = GetPrivateProfileInt(ula, "b_left_small", 32, ininame);
   conf.mcx_full = GetPrivateProfileInt(ula, "mcx_full", 384, ininame);
   conf.mcy_full = GetPrivateProfileInt(ula, "mcy_full", 288, ininame);
   conf.b_top_full = GetPrivateProfileInt(ula, "b_top_full", 48, ininame);
   conf.b_left_full = GetPrivateProfileInt(ula, "b_left_full", 64, ininame);

   conf.ula_preset=u8(-1U);
   add_presets(ula, "preset", &num_ula, ulapreset, &conf.ula_preset);

   load_ula_preset();

   conf.atm.mem_swap = u8(GetPrivateProfileInt(ula, "AtmMemSwap", 0, ininame));
   conf.use_comp_pal = u8(GetPrivateProfileInt(ula, "UsePalette", 1, ininame));
   conf.profi_monochrome = u8(GetPrivateProfileInt(ula, "ProfiMonochrome", 0, ininame));
   conf.ula_plus = GetPrivateProfileInt(ula, "ULAPlus", 0, ininame) != 0;

   conf.flashcolor = u8(GetPrivateProfileInt(video, "FlashColor", 0, ininame));
   conf.frameskip = u8(GetPrivateProfileInt(video, "SkipFrame", 0, ininame));
   conf.flip = (conf.SyncMode == SM_VIDEO) ? 1 : u8(GetPrivateProfileInt(video, "VSync", 0, ininame));
   conf.fullscr = u8(GetPrivateProfileInt(video, "FullScr", 1, ininame));
   conf.refresh = GetPrivateProfileInt(video, "Refresh", 0, ininame);
   conf.frameskipmax = u8(GetPrivateProfileInt(video, "SkipFrameMaxSpeed", 33, ininame));
   conf.updateb = u8(GetPrivateProfileInt(video, "Update", 1, ininame));
   conf.ch_size = u8(GetPrivateProfileInt(video, "ChunkSize", 0, ininame));
   conf.noflic = u8(GetPrivateProfileInt(video, "NoFlic", 0, ininame));
   conf.alt_nf = u8(GetPrivateProfileInt(video, "AltNoFlic", 0, ininame));
   conf.scanbright = GetPrivateProfileInt(video, "ScanIntens", 66, ininame);
   conf.pixelscroll = u8(GetPrivateProfileInt(video, "PixelScroll", 0, ininame));
   conf.detect_video = u8(GetPrivateProfileInt(video, "DetectModel", 1, ininame));
   conf.fontsize = 8;

   conf.videoscale = u8(GetPrivateProfileInt(video, "scale", 2, ininame));

   conf.rsm.mix_frames = u8(GetPrivateProfileInt(video, "rsm.frames", 8, ininame));
   GetPrivateProfileString(video, "rsm.mode", nil, line, sizeof line, ininame);
   conf.rsm.mode = RSM_FIR0;
   if (!strnicmp(line, "FULL", 4)) conf.rsm.mode = RSM_FIR0;
   if (!strnicmp(line, "2C", 2)) conf.rsm.mode = RSM_FIR1;
   if (!strnicmp(line, "3C", 2)) conf.rsm.mode = RSM_FIR2;
   if (!strnicmp(line, "SIMPLE", 6)) conf.rsm.mode = RSM_SIMPLE;

   GetPrivateProfileString(video, "AtariPreset", nil, conf.atariset, sizeof conf.atariset, ininame);

   GetPrivateProfileString(video, video, nil, line, sizeof line, ininame);
   conf.render = 0;
   for (i = 0; renders[i].func; i++)
      if (!strnicmp(line, renders[i].nick, strlen(renders[i].nick)))
         conf.render = i;

   GetPrivateProfileString(video, "driver", nil, line, sizeof line, ininame);
   conf.driver = DRIVER_DDRAW;
   for (i = 0; drivers[i].nick; i++)
      if (!strnicmp(line, drivers[i].nick, strlen(drivers[i].nick)))
         conf.driver = i;

   conf.fast_sl = u8(GetPrivateProfileInt(video, "fastlines", 0, ininame));

   GetPrivateProfileString(video, "Border", nil, line, sizeof line, ininame);
   conf.bordersize = 1;
   if (!strnicmp(line, "none", 4))
       conf.bordersize = 0;
   else if (!strnicmp(line, "small", 5))
       conf.bordersize = 1;
   else if (!strnicmp(line, "wide", 4))
       conf.bordersize = 2;
   conf.minres = GetPrivateProfileInt(video, "MinRes", 0, ininame);

   GetPrivateProfileString(video, "Hide", nil, line, sizeof line, ininame);
   char *ptr = strchr(line, ';'); if (ptr) *ptr = 0;
   for (ptr = line;;)
   {
      size_t max = renders_count - 1;
      for (i = 0; renders[i].func; i++)
      {
         size_t sl = strlen(renders[i].nick);
         if (!strnicmp(ptr, renders[i].nick, sl) && !isalnum(ptr[sl]))
         {
            ptr += sl;
            memcpy(&renders[i], &renders[i+1], (sizeof *renders) * (max-i));
            break;
         }
      }
      if (!*ptr++)
          break;
   }

   GetPrivateProfileString(video, "ScrShotDir", ".", conf.scrshot_dir, sizeof(conf.scrshot_dir), ininame);

   GetPrivateProfileString(video, "ScrShot", nil, line, sizeof line, ininame);
   conf.scrshot = 0;
   if(!strnicmp(line, "scr", 3))
       conf.scrshot = 0;
   else if(!strnicmp(line, "bmp", 3))
       conf.scrshot = 1;
   else if(!strnicmp(line, "png", 3))
       conf.scrshot = 2;

   GetPrivateProfileString(video, "ffmpeg.exec", "ffmpeg.exe", conf.ffmpeg.exec, sizeof conf.ffmpeg.exec, ininame);
   GetPrivateProfileString(video, "ffmpeg.parm", nil, conf.ffmpeg.parm, sizeof conf.ffmpeg.parm, ininame);
   GetPrivateProfileString(video, "ffmpeg.vout", "video#.avi", conf.ffmpeg.vout, sizeof conf.ffmpeg.vout, ininame);
   conf.ffmpeg.newcons = i8(GetPrivateProfileInt(video, "ffmpeg.newconsole", 1, ininame));

   conf.trdos_present = u8(GetPrivateProfileInt(beta128, "beta128", 1, ininame));
   conf.trdos_traps = u8(GetPrivateProfileInt(beta128, "Traps", 1, ininame));
   conf.wd93_nodelay = u8(GetPrivateProfileInt(beta128, "Fast", 1, ininame));
   conf.trdos_interleave = u8(GetPrivateProfileInt(beta128, "IL", 1, ininame)-1);
   if (conf.trdos_interleave > 2) conf.trdos_interleave = 0;
   conf.fdd_noise = u8(GetPrivateProfileInt(beta128, "Noise", 0, ininame));
   GetPrivateProfileString(beta128, "BOOT", nil, conf.appendboot, sizeof conf.appendboot, ininame);
   addpath(conf.appendboot);
   conf.trdos_IORam=GetPrivateProfileInt(beta128, "RamPageFddIO", 0, ininame); //временный код, потом надо удалить
   conf.wiznet=GetPrivateProfileInt(USBZXNET, "WizNet", 0, ininame);
   GetPrivateProfileString(misc, "ColdRAM", "0000FFFF", conf.cold_ram_pat, sizeof conf.cold_ram_pat, ininame);
   
   conf.led.enabled = u8(GetPrivateProfileInt(leds, "leds", 1, ininame));
   conf.led.flash_ay_kbd = u8(GetPrivateProfileInt(leds, "KBD_AY", 1, ininame));
   conf.led.perf_t = u8(GetPrivateProfileInt(leds, "PerfShowT", 0, ininame));
   conf.led.bandBpp = GetPrivateProfileInt(leds, "BandBpp", 512, ininame);
   if (conf.led.bandBpp != 64 && conf.led.bandBpp != 128 && conf.led.bandBpp != 256 && conf.led.bandBpp != 512) conf.led.bandBpp = 512;

   static char nm[] = "AY\0Perf\0LOAD\0Input\0Time\0OSW\0MemBand";
   char *n2 = nm;
   for (i = 0; i < NUM_LEDS; i++) {
      GetPrivateProfileString(leds, n2, nil, line, sizeof line, ininame);
      int x, y;
      unsigned z; unsigned r; n2 += strlen(n2) + 1;
      if (sscanf(line, "%u:%d,%d", &z, &x, &y) != 3) r = 0;
      else r = (x & 0xFFFF) + ((y << 16) & 0x7FFFFFFF) + z*0x80000000;
      *(&conf.led.ay+i) = r;
   }

   conf.sound.do_sound = do_sound_none;
   GetPrivateProfileString(sound, "SoundDrv", nil, line, sizeof line, ininame);
   if (!strnicmp(line, "wave", 4)) {
      conf.sound.do_sound = do_sound_wave;
      conf.soundbuffer = GetPrivateProfileInt(sound, "SoundBuffer", 0, ininame);
      if (!conf.soundbuffer) conf.soundbuffer = 6;
      if (conf.soundbuffer >= MAXWQSIZE) conf.soundbuffer = MAXWQSIZE-1;
   }
   if (!strnicmp(line, "ds", 2)) {
      conf.sound.do_sound = do_sound_ds;
//      conf.soundbuffer = GetPrivateProfileInt(sound, "DSoundBuffer", 1000, ininame);
//      conf.soundbuffer *= 4; // 16-bit, stereo
   }

   conf.sound.enabled = u8(GetPrivateProfileInt(sound, "Enabled", 1, ininame));
   #ifdef MOD_GS
   conf.sound.gsreset = u8(GetPrivateProfileInt(sound, "GSReset", 0, ininame));
   #endif
   conf.sound.fq = GetPrivateProfileInt(sound, "Fq", 44100, ininame);
   conf.sound.dsprimary = u8(GetPrivateProfileInt(sound, "DSPrimary", 0, ininame));

   conf.gs_type = 0;
#ifdef MOD_GS
   GetPrivateProfileString(sound, "GSTYPE", nil, line, sizeof line, ininame);
   #ifdef MOD_GSZ80
   if (!strnicmp(line, "Z80", 3)) conf.gs_type = 1;
   #endif
   #ifdef MOD_GSBASS
   if (!strnicmp(line, "BASS", 4)) conf.gs_type = 2;
   #endif
   conf.gs_ramsize = GetPrivateProfileInt(ngs, "RamSize", 2048, ininame);
#endif

   conf.soundfilter = u8(GetPrivateProfileInt(sound, "SoundFilter", 0, ininame)); //Alone Coder 0.36.4
   conf.RejectDC = u8(GetPrivateProfileInt(sound, "RejectDC", 1, ininame));

   conf.sound.beeper_vol = int(GetPrivateProfileInt(sound, "BeeperVol", 4000, ininame));
   conf.sound.micout_vol = int(GetPrivateProfileInt(sound, "MicOutVol", 1000, ininame));
   conf.sound.micin_vol = int(GetPrivateProfileInt(sound, "MicInVol", 1000, ininame));
   conf.sound.ay_vol = int(GetPrivateProfileInt(sound, "AYVol", 4000, ininame));
   conf.sound.covoxFB = int(GetPrivateProfileInt(sound, "CovoxFB", 0, ininame));
   conf.sound.covoxFB_vol = int(GetPrivateProfileInt(sound, "CovoxFBVol", 8192, ininame));
   conf.sound.covoxDD = int(GetPrivateProfileInt(sound, "CovoxDD", 0, ininame));
   conf.sound.covoxDD_vol = int(GetPrivateProfileInt(sound, "CovoxDDVol", 4000, ininame));
   conf.sound.sd = int(GetPrivateProfileInt(sound, "SD", 0, ininame));
   conf.sound.sd_vol = int(GetPrivateProfileInt(sound, "SDVol", 4000, ininame));
   //conf.sound.saa1099 = GetPrivateProfileInt(sound, "Saa1099", 0, ininame);
   conf.sound.saa1099 = SAA_NONE;
   GetPrivateProfileString(sound, "Saa1099", 0, line, sizeof line, ininame);
   if(!strnicmp(line, "ZXM", 3))
       conf.sound.saa1099 = SAA_ZXM;
   else if(!strnicmp(line, "TFMpro", 6))
       conf.sound.saa1099 = SAA_TFM_PRO;
   conf.sound.moonsound = GetPrivateProfileInt(sound, "MoonSound", 0, ininame);
   conf.sound.moonsound_vol = GetPrivateProfileInt(sound, "MoonSoundVol", 4000, ininame);

   #ifdef MOD_GS
   conf.sound.gs_vol = int(GetPrivateProfileInt(sound, "GSVol", 8000, ininame));
   #endif

   #ifdef MOD_GSBASS
   conf.sound.bass_vol = int(GetPrivateProfileInt(sound, "BASSVol", 8000, ininame));
   #endif

   conf.sound.saa1099fq = GetPrivateProfileInt(saa1099, "Fq", 8000000, ininame);

   add_presets(ay, "VOLTAB", &num_ayvols, ayvols, &conf.sound.ay_vols);
   add_presets(ay, "STEREO", &num_aystereo, aystereo, &conf.sound.ay_stereo);
   conf.sound.ayfq = GetPrivateProfileInt(ay, "Fq", 1774400, ininame);

   GetPrivateProfileString(ay, "Chip", nil, line, sizeof line, ininame);
   conf.sound.ay_chip = SNDCHIP::CHIP_YM;
   if (!strnicmp(line, "AY", 2)) conf.sound.ay_chip = SNDCHIP::CHIP_AY;
   if (!strnicmp(line, "YM2203", 6)) conf.sound.ay_chip = SNDCHIP::CHIP_YM2203;else //Dexus
   if (!strnicmp(line, "YM", 2)) conf.sound.ay_chip = SNDCHIP::CHIP_YM;

   conf.sound.ay_samples = u8(GetPrivateProfileInt(ay, "UseSamples", 0, ininame));

   GetPrivateProfileString(ay, "Scheme", nil, line, sizeof line, ininame);
   conf.sound.ay_scheme = AY_SCHEME_NONE;
   if (!strnicmp(line, "default", 7)) conf.sound.ay_scheme = AY_SCHEME_SINGLE;
   if(!strnicmp(line, "FULLER", 6)) conf.sound.ay_scheme = AY_SCHEME_FULLER;
   if (!strnicmp(line, "PSEUDO", 6)) conf.sound.ay_scheme = AY_SCHEME_PSEUDO;
   if (!strnicmp(line, "QUADRO", 6)) conf.sound.ay_scheme = AY_SCHEME_QUADRO;
   if (!strnicmp(line, "POS", 3)) conf.sound.ay_scheme = AY_SCHEME_POS;
   if (!strnicmp(line, "CHRV", 4)) conf.sound.ay_scheme = AY_SCHEME_CHRV;

   GetPrivateProfileString(input, "ZXKeyMap", "default", conf.zxkeymap, sizeof conf.zxkeymap, ininame);
   for (i = 0; i < zxk_maps_count; i++)
   {
      if (!strnicmp(conf.zxkeymap, zxk_maps[i].name, strlen(zxk_maps[i].name)))
      {
          conf.input.active_zxk = &zxk_maps[i];
          break;
      }
   }

   if(!conf.input.active_zxk)
   {
       errmsg("Invalid keyboard layout '%s' specified, default used", conf.zxkeymap);
       conf.input.active_zxk = &zxk_maps[0]; // default
   }

   GetPrivateProfileString(input, "KeybLayout", "default", line, sizeof(line), ininame);
   ptr = strtok(line, " ;");
   strcpy(conf.keyset, ptr ? ptr : line);

   GetPrivateProfileString(input, "Mouse", nil, line, sizeof line, ininame);
   conf.input.mouse = 0;
   if (!strnicmp(line, "KEMPSTON", 8)) conf.input.mouse = 1;
   if (!strnicmp(line, "AY", 2)) conf.input.mouse = 2;
//0.36.6 from 0.35b2
   GetPrivateProfileString(input, "Wheel", nil, line, sizeof line, ininame);
   conf.input.mousewheel = MOUSE_WHEEL_NONE;
   if (!strnicmp(line, "KEMPSTON", 8)) conf.input.mousewheel = MOUSE_WHEEL_KEMPSTON;
   if (!strnicmp(line, "KEYBOARD", 8)) conf.input.mousewheel = MOUSE_WHEEL_KEYBOARD;
//~
   conf.input.joymouse = u8(GetPrivateProfileInt(input, "JoyMouse", 0, ininame));
   conf.input.mousescale = i8(GetPrivateProfileInt(input, "MouseScale", 0, ininame));
   conf.input.mouseswap = u8(GetPrivateProfileInt(input, "SwapMouse", 0, ininame));
   conf.input.kjoy = u8(GetPrivateProfileInt(input, "KJoystick", 1, ininame));
   conf.input.fjoy = GetPrivateProfileInt(input, "FJoystick", 1, ininame) != 0;
   conf.input.keymatrix = u8(GetPrivateProfileInt(input, "Matrix", 1, ininame));
   conf.input.firedelay = u8(GetPrivateProfileInt(input, "FireRate", 1, ininame));
   conf.input.altlock = u8(GetPrivateProfileInt(input, "AltLock", 1, ininame));
   conf.input.paste_hold = u8(GetPrivateProfileInt(input, "HoldDelay", 2, ininame));
   conf.input.paste_release = u8(GetPrivateProfileInt(input, "ReleaseDelay", 5, ininame));
   conf.input.paste_newline = u8(GetPrivateProfileInt(input, "NewlineDelay", 20, ininame));
   conf.input.keybpcmode = u8(GetPrivateProfileInt(input, "KeybPCMode", 0, ininame));
   conf.atm.xt_kbd = u8(GetPrivateProfileInt(input, "ATMKBD", 0, ininame));
   conf.input.JoyId = GetPrivateProfileInt(input, "Joy", 0, ininame);

   GetPrivateProfileString(input, "Fire", "0", line, sizeof line, ininame);
   conf.input.firenum = 0; conf.input.fire = 0;
   zxkeymap *active_zxk = conf.input.active_zxk;
   for (i = 0; i < active_zxk->zxk_size; i++)
      if (!stricmp(line, active_zxk->zxk[i].name))
      {  conf.input.firenum = i; break; }

   char buff[0x7000];
   GetPrivateProfileSection(colors, buff, sizeof buff, ininame);
   GetPrivateProfileString(colors, "color", "default", line, sizeof line, ininame);
   conf.pal = 0;

   for (i = 1, ptr = buff; i < _countof(pals); ptr += strlen(ptr)+1)
   {
      if (!*ptr)
          break;
      if (!isalnum(*ptr) || !strnicmp(ptr, "color=", 6))
          continue;
      char *ptr1 = strchr(ptr, '=');
      if (!ptr1)
          continue;
      *ptr1 = 0; strcpy(pals[i].name, ptr); ptr = ptr1+1;
      sscanf(ptr, "%02X,%02X,%02X,%02X,%02X,%02X:%X,%X,%X;%X,%X,%X;%X,%X,%X",
         &pals[i].ZZ,  &pals[i].ZN,  &pals[i].NN,
         &pals[i].NB,  &pals[i].BB,  &pals[i].ZB,
         &pals[i].r11, &pals[i].r12, &pals[i].r13,
         &pals[i].r21, &pals[i].r22, &pals[i].r23,
         &pals[i].r31, &pals[i].r32, &pals[i].r33);

      pals[i].r11 = min(pals[i].r11, 256U);
      pals[i].r12 = min(pals[i].r12, 256U);
      pals[i].r13 = min(pals[i].r13, 256U);

      pals[i].r21 = min(pals[i].r21, 256U);
      pals[i].r22 = min(pals[i].r22, 256U);
      pals[i].r23 = min(pals[i].r23, 256U);

      pals[i].r31 = min(pals[i].r31, 256U);
      pals[i].r32 = min(pals[i].r32, 256U);
      pals[i].r33 = min(pals[i].r33, 256U);

      if (!strnicmp(line, pals[i].name, strlen(pals[i].name)))
          conf.pal = i;
      i++;
   }
   conf.num_pals = i;

   GetPrivateProfileString(hdd, "SCHEME", nil, line, sizeof line, ininame);
   conf.ide_scheme = IDE_NONE;
   if(!strnicmp(line, "ATM", 3))
       conf.ide_scheme = IDE_ATM;
   else if(!strnicmp(line, "NEMO-DIVIDE", 11))
       conf.ide_scheme = IDE_NEMO_DIVIDE;
   else if(!strnicmp(line, "NEMO-A8", 7))
       conf.ide_scheme = IDE_NEMO_A8;
   else if(!strnicmp(line, "NEMO", 4))
       conf.ide_scheme = IDE_NEMO;
   else if(!strnicmp(line, "SMUC", 4))
       conf.ide_scheme = IDE_SMUC;
   else if(!strnicmp(line, "PROFI", 5))
       conf.ide_scheme = IDE_PROFI;
   else if(!strnicmp(line, "DIVIDE", 6))
       conf.ide_scheme = IDE_DIVIDE;

   conf.ide_skip_real = u8(GetPrivateProfileInt(hdd, "SkipReal", 0, ininame));
   GetPrivateProfileString(hdd, "CDROM", "SPTI", line, sizeof line, ininame);
   conf.cd_aspi = !strnicmp(line, "ASPI", 4) ? 1 : 0;

   for (int ide_device = 0; ide_device < 2; ide_device++)
   {
      char param[32];
      sprintf(param, "LBA%d", ide_device);

      GetPrivateProfileString(hdd, param, "0", line, sizeof(line), ininame);
      conf.ide[ide_device].lba = strtoull(line, nullptr, 10);
      sprintf(param, "CHS%d", ide_device);
      GetPrivateProfileString(hdd, param, "0/0/0", line, sizeof line, ininame);
      unsigned c, h, s;

      sscanf(line, "%u/%u/%u", &c, &h, &s);
      if(h > 16)
      {
          sprintf(line, "HDD%d heads count > 16 : %u\n", ide_device, h);
          errexit(line);
      }
      if(s > 63)
      {
          sprintf(line, "error HDD%d sectors count > 63 : %u\n", ide_device, s);
          errexit(line);
      }
      if(c > 16383)
      {
          sprintf(line, "error HDD%d cylinders count > 16383 : %u\n", ide_device, c);
          errexit(line);
      }

      conf.ide[ide_device].c = c;
      conf.ide[ide_device].h = h;
      conf.ide[ide_device].s = s;

      sprintf(param, "Image%d", ide_device);
      GetPrivateProfileString(hdd, param, nil, conf.ide[ide_device].image, sizeof conf.ide[ide_device].image, ininame);

      if(conf.ide[ide_device].image[0] &&
         conf.ide[ide_device].image[0] != '<')
          addpath(conf.ide[ide_device].image);

      sprintf(param, "HD%dRO", ide_device);
      conf.ide[ide_device].readonly = (BYTE)GetPrivateProfileInt(hdd, param, 0, ininame);
      sprintf(param, "CD%d", ide_device);
      conf.ide[ide_device].cd = (BYTE)GetPrivateProfileInt(hdd, param, 0, ininame);

      if(!conf.ide[ide_device].cd &&
         conf.ide[ide_device].lba == 0 &&
         conf.ide[ide_device].image[0] &&
         conf.ide[ide_device].image[0] != '<')
      {
          int file = open(conf.ide[ide_device].image, O_RDONLY | O_BINARY, S_IREAD);
          if(file >= 0)
          {
              __int64 sz = _filelengthi64(file);
              close(file);
              conf.ide[ide_device].lba = unsigned(sz / 512);
          }
      }
   }

   addpath(line, "CMOS");
   FILE *f0 = fopen(line, "rb");
   if (f0)
   {
     fread(cmos, 1, sizeof cmos, f0);
     fclose(f0);
   }
   else
       cmos[0x11] = 0xAA;

   addpath(line, "NVRAM");
   if ((f0 = fopen(line, "rb")))
   {
        fread(nvram, 1, sizeof nvram, f0);
        fclose(f0);
   }

   if(conf.gs_type == 1) // z80gs mode
   {
       GetPrivateProfileString(ngs, "SDCARD", nullptr, conf.ngs_sd_card_path, _countof(conf.ngs_sd_card_path), ininame);
       if ( strcmp(conf.ngs_sd_card_path, "<CH341>") != 0 ) addpath(conf.ngs_sd_card_path);
       if(conf.ngs_sd_card_path[0])
           printf("NGS SDCARD='%s'\n", conf.ngs_sd_card_path);
   }

   conf.zc = u8(GetPrivateProfileInt(misc, "ZC", 0, ininame));
   if(conf.zc)
   {
       GetPrivateProfileString(zc, "SDCARD", nullptr, conf.zc_sd_card_path, _countof(conf.zc_sd_card_path), ininame);
       if ( strcmp(conf.zc_sd_card_path, "<CH341>") != 0 ) addpath(conf.zc_sd_card_path);
       if(conf.zc_sd_card_path[0])
           printf("ZC SDCARD='%s'\n", conf.zc_sd_card_path);
       conf.sd_delay = GetPrivateProfileInt(zc, "SDDelay", 1000, ininame);
   }

   GetPrivateProfileString("AUTOLOAD", "DefaultDrive", nil, line, sizeof(line), ininame);
   if(!strnicmp(line, "Auto", 4))
       DefaultDrive = -1U;
   else if(!strnicmp(line, "A", 1))
       DefaultDrive = 0;
   else if(!strnicmp(line, "B", 1))
       DefaultDrive = 1;
   else if(!strnicmp(line, "C", 1))
       DefaultDrive = 2;
   else if(!strnicmp(line, "D", 1))
       DefaultDrive = 3;

   load_arch(ininame);
   loadkeys(ac_main);
#ifdef MOD_MONITOR
   loadkeys(ac_main_xt);
   loadkeys(ac_regs);
   loadkeys(ac_trace);
   loadkeys(ac_mem);
#endif
   temp.scale = GetPrivateProfileInt(video, "winscale", 1, ininame);
}

void autoload()
{
   static char autoload[] = "AUTOLOAD";
   char line[512];

   for (unsigned disk = 0; disk < 4; disk++) {
      char key[8]; sprintf(key, "disk%c", int('A'+disk));
      GetPrivateProfileString(autoload, key, nil, line, sizeof line, ininame);
      if (!*line) continue;
      addpath(line);
      trd_toload = disk;
      if (!loadsnap(line)) errmsg("failed to autoload <%s>", line);
   }

   GetPrivateProfileString(autoload, "snapshot", nil, line, sizeof line, ininame);
   if (!*line) return;
   addpath(line);
   if (!loadsnap(line)) { color(CONSCLR_ERROR); printf("failed to start snapshot <%s>\n", line); }
}

static void apply_memory()
{
   #ifdef MOD_GSZ80
   if (conf.gs_type == 1)
   {
       if(load_rom(conf.gs_rom_path, ROM_GS_M, 32) != 512) // 512k rom
       {
           errmsg("invalid ROM size for NGS (need 512kb), NGS disabled\n");
           conf.gs_type = 0;
       }
   }
   #endif

   if (zxmmoonsound.load_rom(conf.moonsound_rom_path))
   {
       errmsg("failed to load MoonSound ROM, MoonSound disabled\n");
       conf.sound.moonsound = 0;
   }

   if (conf.ramsize != 128 && conf.ramsize != 256 && conf.ramsize != 512 && conf.ramsize != 1024 && conf.ramsize != 4096)
      conf.ramsize = 0;
   if (!(mem_model[conf.mem_model].availRAMs & conf.ramsize)) {
      conf.ramsize = mem_model[conf.mem_model].defaultRAM;
      color(CONSCLR_ERROR);
      printf("invalid RAM size for %s, using default (%uK)\n",
         mem_model[conf.mem_model].fullname, conf.ramsize);
   }

   switch(conf.mem_model)
   {
   case MM_ATM710:
   case MM_ATM3:
      base_sos_rom = ROM_BASE_M + 0*PAGE;
      base_dos_rom = ROM_BASE_M + 1*PAGE;
      base_128_rom = ROM_BASE_M + 2*PAGE;
      base_sys_rom = ROM_BASE_M + 3*PAGE;
   break;

   case MM_ATM450:
   case MM_PROFI:
      base_sys_rom = ROM_BASE_M + 0*PAGE;
      base_dos_rom = ROM_BASE_M + 1*PAGE;
      base_128_rom = ROM_BASE_M + 2*PAGE;
      base_sos_rom = ROM_BASE_M + 3*PAGE;
   break;

   case MM_PLUS3:
      base_128_rom = ROM_BASE_M + 0*PAGE;
      base_sys_rom = ROM_BASE_M + 1*PAGE;
      base_dos_rom = ROM_BASE_M + 2*PAGE;
      base_sos_rom = ROM_BASE_M + 3*PAGE;
   break;

   case MM_QUORUM:
      base_sys_rom = ROM_BASE_M + 0*PAGE;
      base_dos_rom = ROM_BASE_M + 1*PAGE;
      base_128_rom = ROM_BASE_M + 2*PAGE;
      base_sos_rom = ROM_BASE_M + 3*PAGE;
   break;

/*
   case MM_KAY:
      base_128_rom = ROM_BASE_M + 0*PAGE;
      base_sos_rom = ROM_BASE_M + 1*PAGE;
      base_dos_rom = ROM_BASE_M + 2*PAGE;
      base_sys_rom = ROM_BASE_M + 3*PAGE;
   break;
*/

   default:
      base_128_rom = ROM_BASE_M + 0*PAGE;
      base_sos_rom = ROM_BASE_M + 1*PAGE;
      base_sys_rom = ROM_BASE_M + 2*PAGE;
      base_dos_rom = ROM_BASE_M + 3*PAGE;
   }

   unsigned romsize;
   if (conf.use_romset)
   {
      if (!load_rom(conf.sos_rom_path, base_sos_rom))
          errexit("failed to load BASIC48 ROM");
      if (!load_rom(conf.zx128_rom_path, base_128_rom) && conf.reset_rom == RM_128)
          conf.reset_rom = RM_SOS;
      if (!load_rom(conf.dos_rom_path, base_dos_rom))
          conf.trdos_present = 0;
      if (!load_rom(conf.sys_rom_path, base_sys_rom) && conf.reset_rom == RM_SYS)
          conf.reset_rom = RM_SOS;
      romsize = 64;
   }
   else
   {
      if (conf.mem_model == MM_ATM710 || conf.mem_model == MM_ATM3)
      {
         romsize = load_rom(conf.mem_model == MM_ATM710 ? conf.atm2_rom_path : conf.atm3_rom_path, ROM_BASE_M, 64);
         if (romsize != 64 && romsize != 128 && romsize != 512 && romsize != 1024)
            errexit("invalid ROM size for ATM bios");
         unsigned char *lastpage = ROM_BASE_M + (romsize - 64) * 1024;
         base_sos_rom = lastpage + 0*PAGE;
         base_dos_rom = lastpage + 1*PAGE;
         base_128_rom = lastpage + 2*PAGE;
         base_sys_rom = lastpage + 3*PAGE;
      }
      else if (conf.mem_model == MM_PROFSCORP)
      {
         romsize = load_rom(conf.prof_rom_path, ROM_BASE_M, 16);
         if (romsize != 64 && romsize != 128 && romsize != 256)
            errexit("invalid PROF-ROM size");
      }
      else
      {
         const char *romname = nullptr;
         switch(conf.mem_model)
         {
         case MM_PROFI: romname = conf.profi_rom_path; break;
         case MM_SCORP: romname = conf.scorp_rom_path; break;
//[vv]         case MM_KAY: romname = conf.kay_rom_path; break;
         case MM_ATM450: romname = conf.atm1_rom_path; break;
         case MM_PLUS3: romname = conf.plus3_rom_path; break;
         case MM_QUORUM: romname = conf.quorum_rom_path; break;

         default:
             errexit("ROMSET should be defined for this memory model");
         }

         romsize = load_rom(romname, ROM_BASE_M, 64);
         if (romsize != 64)
             errexit("invalid ROM filesize");
      }
   }

   if (conf.mem_model == MM_PROFSCORP)
   {
      temp.profrom_mask = 0;
      if (romsize == 128)
          temp.profrom_mask = 1;
      if (romsize == 256)
          temp.profrom_mask = 3;

      comp.profrom_bank &= temp.profrom_mask;
      set_scorp_profrom(0);
   }

#ifdef MOD_MONITOR
   load_labels(conf.sos_labels_path, base_sos_rom, 0x4000);
#endif

   temp.gs_ram_mask = u8((conf.gs_ramsize-1) >> 4);
   temp.ram_mask = u8((conf.ramsize-1) >> 4);
   temp.rom_mask = u8((romsize-1) >> 4);
   set_banks();

   for(unsigned i = 0; i < CpuMgr.GetCount(); i++)
   {
       Z80 &cpu = CpuMgr.Cpu(i);
       cpu.dbgchk = isbrk(cpu);
   }
}


void applyconfig(bool Init)
{
#ifdef MOD_GS
    init_gs(Init);
#endif
//   printf("%s\n", __FUNCTION__);

   //[vv] disable turbo
   comp.pEFF7 |= EFF7_GIGASCREEN;

//Alone Coder 0.36.4
   conf.frame = frametime;
   cpu.SetTpi(conf.frame);
/*
   if ((conf.mem_model == MM_PENTAGON)&&(comp.pEFF7 & EFF7_GIGASCREEN))
       conf.frame = 71680;
*/
//~Alone Coder
   temp.ticks_frame = (unsigned)(temp.cpufq / double(conf.intfq) + 1.0);
   loadzxkeys(&conf);
   apply_memory();

   temp.snd_frame_ticks = (conf.sound.fq << TICK_FF) / conf.intfq;
   temp.snd_frame_samples = temp.snd_frame_ticks >> TICK_FF;
   temp.frameskip = conf.sound.enabled? conf.frameskip : conf.frameskipmax;

   input.firedelay = 1; // if conf.input.fire changed
   input.clear_zx();

   modem.open(conf.modem_port);

   load_atariset();
   apply_video();
   apply_sound();

   hdd.dev[0].configure(conf.ide+0);
   hdd.dev[1].configure(conf.ide+1);
   if (conf.atm.xt_kbd) input.atm51.clear();

   if(conf.gs_type == 1)
   {
       SdCard.Close();
       SdCard.Open(conf.ngs_sd_card_path);
   }

   if(conf.zc)
   {
       Zc.Close();
       Zc.Open(conf.zc_sd_card_path);
   }
        if(conf.wiznet){ 
                comp.wiznet.p83=0; 
                comp.wiznet.p82=0; 
                comp.wiznet.p81=0; 
                Wiz5300_Close(); 
                Wiz5300_Init(); 
        } 

   setpal(0);
}

void load_arch(const char *fname)
{
   GetPrivateProfileString("ARC", "SkipFiles", nil, skiparc, sizeof skiparc, fname);
   char *p; //Alone Coder 0.36.7
   for (/*char * */p = skiparc;;) {
      char *nxt = strchr(p, ';');
      if (!nxt) break;
      *nxt = 0; p = nxt+1;
   }
   p[strlen(p)+1] = 0;

   GetPrivateProfileSection("ARC", arcbuffer, sizeof arcbuffer, fname);
   for (char *x = arcbuffer; *x; ) {
      char *newx = x + strlen(x)+1;
      char *y = strchr(x, '=');
      if (!y) {
ignore_line:
         memcpy(x, newx, sizeof arcbuffer - size_t(newx-arcbuffer));
      } else {
         *y = 0; if (!stricmp(x, "SkipFiles")) goto ignore_line;
         x = newx;
      }
   }
}

void loadkeys(action *table)
{
   unsigned num[0x300], i = 0;
   unsigned j; //Alone Coder 0.36.7
   if (!table->name)
       return; // empty table (can't sort)
   for (action *p = table; p->name; p++, i++)
   {
      char line[0x400];
      GetPrivateProfileString("SYSTEM.KEYS", p->name, "`", line, sizeof line, ininame);
      if (*line == '`')
      {
         errmsg("keydef for %s not found", p->name);
         load_errors = 1;
bad_key:
         p->k1 = 0xFE;
         p->k2 = 0xFF;
         p->k3 = 0xFD;
         continue;
      }
      char *s = strchr(line, ';');
      if(s)
          *s = 0;
      p->k1 = p->k2 = p->k3 = p->k4 = 0; num[i] = 0;
      for (s = line;;)
      {
         while (*s == ' ') s++;
         if (!*s)
             break;
         char *s1 = s;
         while (isalnum(*s))
             s++;
         for (j = 0; j < pckeys_count; j++)
         {
            if ((int)strlen(pckeys[j].name)==s-s1 && !strnicmp(s1, pckeys[j].name, size_t(s-s1)))
            {
               switch (num[i])
               {
                  case 0: p->k1 = pckeys[j].virtkey; break;
                  case 1: p->k2 = pckeys[j].virtkey; break;
                  case 2: p->k3 = pckeys[j].virtkey; break;
                  case 3: p->k4 = pckeys[j].virtkey; break;
                  default:
                     color(CONSCLR_ERROR);
                     printf("warning: too many keys in %s=%s\n", p->name, line);
                     load_errors = 1;
               }
               num[i]++;
               break;
            }
         }
         if (j == pckeys_count)
         {
            color(CONSCLR_ERROR);
            char x = *s; *s = 0;
            printf("bad key: %s\n", s1); *s = x;
            load_errors = 1;
         }
      }
      if (!num[i])
          goto bad_key;
   }

   // sort keys
   for (unsigned k = 0; k < i-1; k++)
   {
      unsigned max = k;
      for (unsigned l = k+1; l < i; l++)
         if (num[l] > num[max])
             max = l;

      action tmp = table[k];
      table[k] = table[max];
      table[max] = tmp;

      unsigned tm = num[k];
      num[k] = num[max];
      num[max] = tm;
   }
}

void loadzxkeys(CONFIG *conf)
{
   char section[0x200];
   sprintf(section, "ZX.KEYS.%s", conf->keyset);
   char line[0x300];
   char *s; //Alone Coder 0.36.7
   unsigned k; //Alone Coder 0.36.7
   zxkeymap *active_zxk = conf->input.active_zxk;

   for (unsigned i = 0; i < VK_MAX; i++)
   {
      inports[i].port1 = inports[i].port2 = &input.kjoy;
      inports[i].mask1 = inports[i].mask2 = 0xFF;
      for (unsigned j = 0; j < pckeys_count; j++)
      {
         if (pckeys[j].virtkey == i)
         {
            GetPrivateProfileString(section, pckeys[j].name, "", line, sizeof line, ininame);
            s = strtok(line, " ;");
            if(s)
            {
               for(k = 0; k < active_zxk->zxk_size; k++)
               {
                  if (!stricmp(s, active_zxk->zxk[k].name))
                  {
                     inports[i].port1 = active_zxk->zxk[k].port;
                     inports[i].mask1 = active_zxk->zxk[k].mask;
                     switch(i)
                     {
                     case DIK_CONTROL:
                         inports[DIK_LCONTROL].port1 = active_zxk->zxk[k].port;
                         inports[DIK_LCONTROL].mask1 = active_zxk->zxk[k].mask;
                         inports[DIK_RCONTROL].port1 = active_zxk->zxk[k].port;
                         inports[DIK_RCONTROL].mask1 = active_zxk->zxk[k].mask;
                     break;

                     case DIK_SHIFT:
                         inports[DIK_LSHIFT].port1 = active_zxk->zxk[k].port;
                         inports[DIK_LSHIFT].mask1 = active_zxk->zxk[k].mask;
                         inports[DIK_RSHIFT].port1 = active_zxk->zxk[k].port;
                         inports[DIK_RSHIFT].mask1 = active_zxk->zxk[k].mask;
                     break;

                     case DIK_MENU:
                         inports[DIK_LMENU].port1 = active_zxk->zxk[k].port;
                         inports[DIK_LMENU].mask1 = active_zxk->zxk[k].mask;
                         inports[DIK_RMENU].port1 = active_zxk->zxk[k].port;
                         inports[DIK_RMENU].mask1 = active_zxk->zxk[k].mask;
                     break;
                     }
                     break;
                  }
               }
            }
            s = strtok(nullptr, " ;");
            if(s)
            {
               for (k = 0; k < active_zxk->zxk_size; k++)
               {
                  if (!stricmp(s, active_zxk->zxk[k].name))
                  {
                     inports[i].port2 = active_zxk->zxk[k].port;
                     inports[i].mask2 = active_zxk->zxk[k].mask;

                     switch(i)
                     {
                     case DIK_CONTROL:
                         inports[DIK_LCONTROL].port2 = active_zxk->zxk[k].port;
                         inports[DIK_LCONTROL].mask2 = active_zxk->zxk[k].mask;
                         inports[DIK_RCONTROL].port2 = active_zxk->zxk[k].port;
                         inports[DIK_RCONTROL].mask2 = active_zxk->zxk[k].mask;
                     break;

                     case DIK_SHIFT:
                         inports[DIK_LSHIFT].port2 = active_zxk->zxk[k].port;
                         inports[DIK_LSHIFT].mask2 = active_zxk->zxk[k].mask;
                         inports[DIK_RSHIFT].port2 = active_zxk->zxk[k].port;
                         inports[DIK_RSHIFT].mask2 = active_zxk->zxk[k].mask;
                     break;

                     case DIK_MENU:
                         inports[DIK_LMENU].port2 = active_zxk->zxk[k].port;
                         inports[DIK_LMENU].mask2 = active_zxk->zxk[k].mask;
                         inports[DIK_RMENU].port2 = active_zxk->zxk[k].port;
                         inports[DIK_RMENU].mask2 = active_zxk->zxk[k].mask;
                     break;
                     }
                     break;
                  }
               }
            }
            break;
         }
      }
   }
}
