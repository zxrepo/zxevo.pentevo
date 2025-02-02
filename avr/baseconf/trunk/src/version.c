#include <avr/pgmspace.h>

#include "mytypes.h"
#include "main.h"
#include "ps2.h"

//base configuration version string pointer [far address of PROGMEM]
const ULONG baseVersionAddr = 0x1DFF0;

//bootloader version string pointer [far address of PROGMEM]
const ULONG bootVersionAddr = 0x1FFF0;

UBYTE GetVersionByte(UBYTE index)
{
	if ( index < 0x10 )
	{
		switch( ext_type_gluk )
		{
			case EXT_TYPE_BASECONF_VERSION:
			{
				//base configuration version
				return (UBYTE)pgm_read_byte_far(baseVersionAddr+(ULONG)index);
			}

			case EXT_TYPE_BOOTLOADER_VERSION:
			{
				//bootloader version
				return (UBYTE)pgm_read_byte_far(bootVersionAddr+(ULONG)index);
			}

			case EXT_TYPE_PS2KEYBOARDS_LOG:
			{
				//PS2 keyboards log
				return ps2keyboard_from_log();
			}

			case EXT_TYPE_RDCFG:
			{
				// read config byte
				return (index==0) ? modes_register : 0xFF;
			}
		}
	}
	return (UBYTE)0xFF;
}

void SetVersionType(UBYTE type)
{
	ext_type_gluk = type;
}


