#include "std.h"

#include "emul.h"
#include "vars.h"

#include "util.h"

#if defined(MOD_GSZ80) || defined(MOD_GSBASS)
namespace BASS
{
/* BASS-specific functions and variables */
TGetVersion          GetVersion;
TInit                Init;
TFree                Free;
TPause               Pause;
TStart               Start;
TStop                Stop;
TGetConfig           GetConfig;
TSetConfig           SetConfig;
TGetInfo             GetInfo;

TMusicFree           MusicFree;
TMusicLoad           MusicLoad;
TChannelPause        ChannelPause;
TChannelPlay         ChannelPlay;
TChannelStop         ChannelStop;

TChannelGetPosition  ChannelGetPosition;
TChannelSetPosition  ChannelSetPosition;
TChannelSetAttribute ChannelSetAttribute;
TChannelGetAttribute ChannelGetAttribute;
TChannelGetLevel     ChannelGetLevel;
TErrorGetCode        ErrorGetCode;
TChannelFlags        ChannelFlags;

TChannelBytes2Seconds ChannelBytes2Seconds;

TChannelIsActive ChannelIsActive;

TStreamCreate        StreamCreate;
TStreamCreateFileUser StreamCreateFileUser;
TStreamFree          StreamFree;

HMODULE Bass = nullptr;
bool Initialized = false;

void Load()
{
   if(Bass)
       return;
#if defined(_WIN64) || defined(AMD64)
   Bass = LoadLibrary("bass64.dll");
   if (!Bass)
	   errexit("can't load bass64.dll");
#else
   Bass = LoadLibrary("bass.dll");
   if (!Bass)
	   errexit("can't load bass.dll");
#endif

   GetVersion = (TGetVersion)GetProcAddress(Bass, "BASS_GetVersion");
   if (!GetVersion || (HIWORD(GetVersion()) != 0x0204))
      errexit("unexpected BASS version. unreal requires BASS 2.4");

   ErrorGetCode = (BASS::TErrorGetCode)GetProcAddress(Bass, "BASS_ErrorGetCode");
   Init = (TInit)GetProcAddress(Bass, "BASS_Init");
   Free = (TFree)GetProcAddress(Bass, "BASS_Free");
   Pause = (TPause)GetProcAddress(Bass, "BASS_Pause");
   Start = (TStart)GetProcAddress(Bass, "BASS_Start");
   Stop = (TStop)GetProcAddress(Bass, "BASS_Stop");
   SetConfig = (TSetConfig)GetProcAddress(Bass, "BASS_SetConfig");
   GetConfig = (TGetConfig)GetProcAddress(Bass, "BASS_GetConfig");
   GetInfo = (TGetInfo)GetProcAddress(Bass, "BASS_GetInfo");

   MusicFree = (TMusicFree)GetProcAddress(Bass, "BASS_MusicFree");
   MusicLoad = (TMusicLoad)GetProcAddress(Bass, "BASS_MusicLoad");
   ChannelPlay = (TChannelPlay)GetProcAddress(Bass, "BASS_ChannelPlay");
   ChannelPause = (TChannelPause)GetProcAddress(Bass, "BASS_ChannelPause");
   ChannelStop = (TChannelStop)GetProcAddress(Bass, "BASS_ChannelStop");

   ChannelGetPosition = (TChannelGetPosition)GetProcAddress(Bass, "BASS_ChannelGetPosition");
   ChannelSetPosition = (TChannelSetPosition)GetProcAddress(Bass, "BASS_ChannelSetPosition");
   ChannelSetAttribute = (TChannelSetAttribute)GetProcAddress(Bass, "BASS_ChannelSetAttribute");
   ChannelGetAttribute = (TChannelGetAttribute)GetProcAddress(Bass, "BASS_ChannelGetAttribute");
   ChannelFlags = (TChannelFlags)GetProcAddress(Bass, "BASS_ChannelFlags");
   ChannelGetLevel = (TChannelGetLevel)GetProcAddress(Bass, "BASS_ChannelGetLevel");
   ChannelBytes2Seconds = (TChannelBytes2Seconds)GetProcAddress(Bass, "BASS_ChannelBytes2Seconds");
   ChannelIsActive = (TChannelIsActive)GetProcAddress(Bass, "BASS_ChannelIsActive");

   StreamCreate = (TStreamCreate)GetProcAddress(Bass, "BASS_StreamCreate");
   StreamCreateFileUser = (TStreamCreateFileUser)GetProcAddress(Bass, "BASS_StreamCreateFileUser");
   StreamFree = (TStreamFree)GetProcAddress(Bass, "BASS_StreamFree");

   if (!Init)
       errexit("can't import BASS API: BASS_Init");
   if (!Free)
       errexit("can't import BASS API: BASS_Free");
   if (!Pause)
       errexit("can't import BASS API: BASS_Pause");
   if (!Start)
       errexit("can't import BASS API: BASS_Start");
   if (!Stop)
       errexit("can't import BASS API: BASS_Stop");
   if (!MusicFree)
       errexit("can't import BASS API: BASS_MusicFree");
   if (!MusicLoad)
       errexit("can't import BASS API: BASS_MusicLoad");
   if (!GetConfig)
       errexit("can't import BASS API: BASS_GetConfig");
   if (!SetConfig)
       errexit("can't import BASS API: BASS_SetConfig");
   if (!GetInfo)
       errexit("can't import BASS API: BASS_GetInfo");
   if (!ChannelFlags)
       errexit("can't import BASS API: BASS_ChannelFlags");
   if (!ChannelSetAttribute)
       errexit("can't import BASS API: BASS_ChannelSetAttribute");
   if(!ChannelGetAttribute)
       errexit("can't import BASS API: BASS_ChannelGetAttribute");
   if (!ChannelGetPosition)
       errexit("can't import BASS API: BASS_ChannelGetPosition");
   if (!ChannelSetPosition)
       errexit("can't import BASS API: BASS_ChannelSetPosition");
   if (!ChannelGetLevel)
       errexit("can't import BASS API: BASS_ChannelGetLevel");
   if (!ChannelBytes2Seconds)
       errexit("can't import BASS API: BASS_ChannelBytes2Seconds");
   if (!ChannelIsActive)
       errexit("can't import BASS API: BASS_ChannelIsActive");
   if (!ChannelPlay)
       errexit("can't import BASS API: BASS_ChannelPlay");
   if (!ChannelPause)
       errexit("can't import BASS API: BASS_ChannelPause");
   if (!ChannelStop)
       errexit("can't import BASS API: BASS_ChannelStop");
   if (!ErrorGetCode)
       errexit("can't import BASS API: BASS_ErrorGetCode");
   if (!StreamCreate)
       errexit("can't import BASS API: BASS_StreamCreate");
   if (!StreamCreateFileUser)
       errexit("can't import BASS API: BASS_StreamCreateFileUser");
   if (!StreamFree)
       errexit("can't import BASS API: BASS_StreamFree");
}

void Unload()
{
    if(Bass)
    {
        if(Free)
        {
            Free();
        }
        FreeLibrary(Bass);
        Initialized = false;
        Bass = nullptr;
    }
}

}
#endif
