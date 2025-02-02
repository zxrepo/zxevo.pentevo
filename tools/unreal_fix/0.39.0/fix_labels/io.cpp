#include "std.h"

#include "emul.h"
#include "funcs.h"
#include "vars.h"
#include "draw.h"
#include "memory.h"
#include "atm.h"
#include "profi.h"
#include "sndrender/sndcounter.h"
#include "sndrender/dev_moonsound.h"
#include "sound.h"
#include "gs.h"
#include "sdcard.h"
#include "zc.h"
#include "tape.h"
#include "zxevo.h"
#include "upd765.h"
#include "zxusbnet.h"

void out(unsigned port, unsigned char val)
{
   port &= 0xFFFF;
   u8 p1 = (port & 0xFF);
   brk_port_out = port; brk_port_val = val;

   // � ������ ���������� ������ �� ������ 8���

   if(conf.ula_plus)
   {
       if(port == 0xBF3B)
       {
           comp.ula_plus_group = val >> 6;
           if(comp.ula_plus_group == 0)
           {
               comp.ula_plus_pal_idx = val & 0x3F;
           }
           return;
       }

       if(port == 0xFF3B)
       {
           if(comp.ula_plus_group == 0)
           {
               comp.comp_pal[comp.ula_plus_pal_idx] = val;
               temp.comp_pal_changed = 1;
               return;
           }

           if(comp.ula_plus_group == 1)
           {
               bool en = (val & 1) != 0;
               if(comp.ula_plus_en != en)
               {
                   comp.ula_plus_en = en;
                   if(comp.ula_plus_en)
                   {
                       temp.rflags |= RF_COMPPAL | RF_PALB;
                   }
                   else
                   {
                       temp.rflags &= unsigned(~(RF_COMPPAL | RF_PALB));
                   }
                   video_color_tables();
                   temp.comp_pal_changed = 1;
               }
               return;
           }
           return;
       }
   }

   #ifdef MOD_GS
   // 10111011 | BB
   // 10110011 | B3
   // 00110011 | 33
   if ((port & 0xFF) == 0x33 && conf.gs_type) // 33
   {
       out_gs(p1, val);
       return;
   }
   if ((port & 0xF7) == 0xB3 && conf.gs_type) // BB, B3
   {
       out_gs(p1, val);
       return;
   }
   #endif

   // ZXM-MoonSound
   if (conf.sound.moonsound &&
       (conf.mem_model == MM_PROFI ? !(comp.pDFFD & 0x80) : 1) &&
       (((p1 & 0xFC) == 0xC4) || ((p1 & 0xFE) == 0xC2)))
   {
       if (zxmmoonsound.write(port, val))
           return;
   }

   // z-controller
   if (conf.zc && (port & 0xFF) == 0x57 )
   {
      if ((port & 0x80FF) == 0x8057 && conf.mem_model == MM_ATM3
         &&(comp.flags & CF_DOSPORTS)) Zc.Wr(0x0077, val);
      else Zc.Wr(port, val);
      return;
   }
   
   if ( conf.zc && ( (port & 0xFF) == 0x77 ) )
   {
      if ( ( conf.mem_model != MM_ATM3 ) || ( ( comp.flags & CF_DOSPORTS) == 0 ) )
      {
        Zc.Wr(port, val);
        return;
      }
   }
   
   if(conf.wiznet && (port & 0xff) == 0xab ){ 
      pXXAB_Write(port,val); 
      return; 
   } 
   // divide �� nemo ������
   if(conf.ide_scheme == IDE_NEMO_DIVIDE)
   {
       if((port & 0x1E) == 0x10) // rrr1000x
       {
           if((port & 0xFF) == 0x11)
           {
               comp.ide_write = val;
               comp.ide_hi_byte_w = 0;
               comp.ide_hi_byte_w1 = 1;
               return;
           }

           if((port & 0xFE) == 0x10)
           {
               comp.ide_hi_byte_w ^= 1;

               if(comp.ide_hi_byte_w1) // ���� ������ � ���� 0x11 (������� ���� ��� ��������)
               {
                   comp.ide_hi_byte_w1 = 0;
               }
               else
               {
                   if(comp.ide_hi_byte_w) // ���������� ������� ����
                   {
                       comp.ide_write = val;
                       return;
                   }
                   else // ������ ������� � ������� ����� ������� (��� ����� ������� write_hdd_5)
                   {
                       u8 tmp = comp.ide_write;
                       comp.ide_write = val;
                       val = tmp;
                   }
               }
           }
           else
           {
               comp.ide_hi_byte_w = 0;
           }
           goto write_hdd_5;
       }
       else if((port & 0xFF) == 0xC8)
       {
           return hdd.write(8, val);
       }
   }

   if(conf.mem_model == MM_ATM3)
   {
	   switch(port & 0xFF){
		   case 0xBF:	// ���� ���������� �������
				if((comp.pBF ^ val) & comp.pBF & 8) // D3: 1->0
				{
					nmi_pending  = 1;
					trdos_in_nmi = comp.flags&CF_TRDOS;
				}
			   comp.pBF = val;
			   set_banks();
			   return;
		   case 0xBE:	// ���� ������������� RAM0 ���3
			   if(cpu.nmi_in_progress&&(cpu.nmi_in_progress==conf.trdos_IORam))
			   {
				  if(trdos_in_nmi)
					  comp.flags |= CF_SETDOSROM|CF_TRDOS;
				  cpu.nmi_in_progress = false;
				  set_banks();
				  return;
			   }
			   comp.pBE = 2; // ������� ��� ������ �� nmi
			   return;
		   case 0xBD:	// ���� ������ ���������� � ����� ��������� ������ FDD
			   switch(port & 0xEFFF){
				   case 0x00BD:
					   comp.brk_addr &= 0xFF00;
					   comp.brk_addr |= ((u16)val)&0x00FF;
					   break;
				   case 0x01BD:
					   comp.brk_addr &= 0x00FF;
					   comp.brk_addr |= ( ((u16)val) << 8 )&0xFF00;
					   break;
				   case 0x03BD:
					   comp.fddIO2Ram_mask = val;
					   break;					   
			   }
			   return;
	   }
   }

   if (comp.flags & CF_DOSPORTS)
   {
      if(conf.mem_model == MM_ATM3 && (p1 & 0x1F) == 0x0F && !(((p1 >> 5) - 1) & 4))
      {
          // 2F = 001|01111b
          // 4F = 010|01111b
          // 6F = 011|01111b
          // 8F = 100|01111b
		     comp.wd_shadow[(p1 >> 5) - 1] = val;
      }
      if (conf.ide_scheme == IDE_ATM && (port & 0x1F) == 0x0F)
      {
         if (port & 0x100) { comp.ide_write = val; return; }
      write_hdd_5:
         port >>= 5;
      write_hdd:
         port &= 7;
         if (port)
             hdd.write(port, val);
         else
             hdd.write_data(unsigned(val | (comp.ide_write << 8)));
         return;
      }

      if ((port & 0x18A3) == (0xFFFE & 0x18A3))
      { // SMUC
         if (conf.smuc)
         {
            if ((port & 0xA044) == (0xDFBA & 0xA044))
            { // clock
               if (comp.pFFBA & 0x80)
                   cmos_write(val);
               else
                   comp.cmos_addr = val;
               return;
            }
            if ((port & 0xA044) == (0xFFBA & 0xA044))
            { // SMUC system port
               if ((val & 1) && (conf.ide_scheme == IDE_SMUC))
                   hdd.reset();
               comp.nvram.write(val);
               comp.pFFBA = val;
               return;
            }
            if ((port & 0xA044) == (0x7FBA & 0xA044))
            {
                comp.p7FBA = val;
                return;
            }
         }
         if ((port & 0x8044) == (0xFFBE & 0x8044) && conf.ide_scheme == IDE_SMUC)
         { // FFBE, FEBE
            if(comp.pFFBA & 0x80)
            {
                if(!(port & 0x100))
                    hdd.write(8, val); // control register
                return;
            }

            if (!(port & 0x2000))
            {
                comp.ide_write = val;
                return;
            }
            port >>= 8;
                goto write_hdd;
         }
      }

      if (conf.mem_model == MM_ATM710 || conf.mem_model == MM_ATM3)
      {
         if ((conf.mem_model == MM_ATM3) && ((port & 0x3FFF) == 0x37F7)) // x7f7 ATM3 4Mb memory manager
         {
             unsigned idx = ((comp.p7FFD & 0x10) >> 2) | ((port >> 14) & 3);
             comp.pFFF7[idx] = (comp.pFFF7[idx] & ~0x1FFU) | (val ^ 0xFF); // always ram
             set_banks();
             return;
         }

         if (p1 == 0x77) // xx77
         {
             set_atm_FF77(port, val);
             return;
         }

         u32 mask = (conf.mem_model == MM_ATM3) ? /*0x3FFF*/ 0x0FFF : 0x00FF; // lvd fix: pentevo hardware decodes fully only low byte,
			// so using eff7 in shadow mode lead to outting to fff7,
                                                                              // unlike this was in unreal!
         if ((port & mask) == (0x3FF7 & mask)) // xff7
         {
             comp.pFFF7[((comp.p7FFD & 0x10) >> 2) | ((port >> 14) & 3)] = unsigned(((val & 0xC0) << 2) | (val & 0x3F)) ^ 0x33FU;
             set_banks();
             return;
         }

         if ((p1 & 0x9F) == 0x9F && !(comp.aFF77 & 0x4000))
             atm_writepal(port, val); // don't return - write to TR-DOS system port
      }

      if(conf.mem_model == MM_PROFI)
      {
          if((comp.p7FFD & 0x10) && (comp.pDFFD & 0x20)) // modified ports
          {
              // BDI ports
              if((p1 & 0x9F) == 0x83) // WD93 ports
              {
                  comp.wd.out((p1 & 0x60) | 0x1F, val);
                  return;
              }

              if((p1 & 0xE3) == 0x23) // port FF
              {
                  comp.wd.out(0xFF, val);
                  return;
              }

              // RTC
              if((port & 0x9F) == 0x9F && conf.cmos)
              {
                if(port & 0x20)
                {
                    comp.cmos_addr = val;
                    return;
                }
                cmos_write(val);
                return;
              }

              // IDE (AB=10101011, CB=11001011, EB=11101011)
              if ((p1 & 0x9F) == 0x8B && (conf.ide_scheme == IDE_PROFI))
              {
                  if(p1 & 0x40)
                  {    // cs1
                      if (!(p1 & 0x20))
                      {
                          comp.ide_write = val;
                          return;
                      }
                      port >>= 8;
                      goto write_hdd;
                  }

                  // cs3
                  if(p1 & 0x20)
                  {
                      if(((port>>8) & 7) == 6)
                          hdd.write(8, val);
                      return;
                  }
              }
          }
          else
          {
              // BDI ports
              if((p1 & 0x83) == 0x03)  // WD93 ports 1F, 3F, 5F, 7F
              {
                  comp.wd.out((p1 & 0x60) | 0x1F,val);
                  return;
              }

              if((p1 & 0xE3) == ((comp.pDFFD & 0x20) ? 0xA3 : 0xE3)) // port FF
              {
                  comp.wd.out(0xFF,val);
                  return;
              }
          }
      } // profi

      if(conf.mem_model == MM_QUORUM /* && !(comp.p00 & Q_TR_DOS)*/) // cpm ports
      {
          if((p1 & 0xFC) == 0x80) // 80, 81, 82, 83
          {
              p1 = u8(((p1 & 3) << 5) | 0x1F);

              comp.wd.out(p1, val);
              return;
          }

          if(p1 == 0x85) // 85
          {
//            01 -> 00 A
//            10 -> 01 B
//            00 -> 11 D (unused)
//            11 -> 11 D (unused)
              static const u8 drv_decode[] = { 3, 0, 1, 3 };
              u8 drv = drv_decode[val & 3];
              comp.wd.out(0xFF, ((val & ~3) ^ 0x10) | 0xCC | 8 | drv);
              return;
          }
      } // quorum
      else if ((p1 & 0x1F) == 0x1F) // 1F, 3F, 5F, 7F, FF
      {
		  if(p1 & 0x80) {
			comp.trdos_last_ff = val & 0x1f;
		  }
		  if((comp.flags & CF_TRDOS) && ( (comp.aFF77 & 0x4000) != 0 ) && conf.trdos_IORam && (bankr[0]==base_dos_rom) && (p1 & 0x80))
          {
		     comp.wd.out(p1, val);
			 if( ( (1<<comp.wd.drive) & comp.fddIO2Ram_mask )  )
             {
				trdos_in_nmi = comp.flags&CF_TRDOS;
				cpu.nmi_in_progress=conf.trdos_IORam;
				set_banks();
			 }
		  }
          else if((comp.flags & CF_TRDOS)  && ( (comp.aFF77 & 0x4000) != 0 ) 
			  && conf.trdos_IORam && (1<<comp.wd.drive)&comp.fddIO2Ram_mask && (bankr[0]==base_dos_rom))
		  {
		     trdos_in_nmi = comp.flags&CF_TRDOS;
			 cpu.nmi_in_progress=conf.trdos_IORam;
			 set_banks();
		  }
          else
          {
		     comp.wd.out(p1, val);
		  }
          return;
      }
      // don't return - out to port #FE works in trdos!
   }
   else // �� dos
   {
       if((p1 == 0x3F) && (conf.sound.ay_scheme == AY_SCHEME_FULLER)) // fuller AY register select
       {
           ay[0].select(val);
           return;
       }
       if((p1 == 0x5F) && (conf.sound.ay_scheme == AY_SCHEME_FULLER)) // fuller AY data
       {
           ay[0].write(temp.sndblock ? 0 : cpu.t, val);
           return;
       }

         if(((port & 0xA3) == 0xA3) && (conf.ide_scheme == IDE_DIVIDE))
         {
             if((port & 0xFF) == 0xA3)
             {
                 comp.ide_hi_byte_w ^= 1;
                 if(comp.ide_hi_byte_w)
                 {
                     comp.ide_write = val;
                     return;
                 }
                 u8 tmp = comp.ide_write;
                 comp.ide_write = val;
                 val = tmp;
             }
             else
             {
                 comp.ide_hi_byte_w = 0;
             }
             port >>= 2;
             goto write_hdd;
         }

         if ((unsigned char)port == 0x1F && conf.sound.ay_scheme == AY_SCHEME_POS)
         {
             comp.active_ay = val & 1;
             return;
         }

         if (!(port & 6) && (conf.ide_scheme == IDE_NEMO || conf.ide_scheme == IDE_NEMO_A8))
         {
             unsigned hi_byte = (conf.ide_scheme == IDE_NEMO)? (port & 1) : (port & 0x100);
             if (hi_byte)
             {
                 comp.ide_write = val;
                 return;
             }
             if ((port & 0x18) == 0x08)
             {
                 if ((port & 0xE0) == 0xC0)
                     hdd.write(8, val);
                 return;
             } // CS1=0,CS0=1,reg=6
             if ((port & 0x18) != 0x10)
                 return; // invalid CS0,CS1
             goto write_hdd_5;
         }
   }

   if((port & 0xFF) == 0x00 && conf.mem_model == MM_QUORUM)
   {
       comp.p00 = val;
       set_banks();
       return;
   }

   #ifdef MOD_VID_VD
   if ((unsigned char)port == 0xDF)
   {
       comp.pVD = val;
       comp.vdbase = (comp.pVD & 4)? vdmem[comp.pVD & 3] : 0;
       return;
   }
   #endif

   // port #FE
   bool pFE;

   // scorp  xx1xxx10 /dos=1 (sc16 green)
   if((conf.mem_model == MM_SCORP || conf.mem_model == MM_PROFSCORP))
       pFE = ((port & 0x23) == (0xFE & 0x23)) && !(comp.flags & CF_DOSPORTS);
   else if(conf.mem_model == MM_QUORUM) // 1xx11xx0
       pFE = ((port & 0x99) == (0xFE & 0x99));
   else // others xxxxxxx0
       pFE = !(port & 1);

   if (pFE)
   {
//[vv]      assert(!(val & 0x08));

      spkr_dig = (val & 0x10) ? conf.sound.beeper_vol : 0;
      mic_dig = (val & 0x08) ? conf.sound.micout_vol : 0;

      // speaker & mic
      if ((comp.pFE ^ val) & 0x18)
      {
//          __debugbreak();
          flush_dig_snd();
      }


      unsigned char new_border = (val & 7);
      if (conf.mem_model == MM_ATM710 || conf.mem_model == MM_ATM3 || conf.mem_model == MM_ATM450)
          new_border |= ((port & 8) ^ 8);
      if (comp.border_attr ^ new_border)
          update_screen();
      comp.border_attr = new_border;

      if (conf.mem_model == MM_ATM450)
          set_atm_aFE((unsigned char)port);

      if (conf.mem_model == MM_PROFI)
      {
        if(!(port & 0x80) && (comp.pDFFD & 0x80))
        {
          profi_writepal(u8(~(port >> 8)));
        }
      }

      comp.pFE = val;
      // do not return! intro to navy seals (by rst7) uses out #FC for to both FE and 7FFD
   }

   // #xD
   if (!(port & 2))
   {

      if (conf.sound.covoxDD && (unsigned char)port == 0xDD)
      { // port DD - covox
//         __debugbreak();
         flush_dig_snd();
         covDD_vol = val*conf.sound.covoxDD_vol/0x100;
         return;
      }

      if (!(port & 0x8000)) // zx128 port
      {
         // 0001xxxxxxxxxx0x (bcig4) // 1FFD
         // 0010xxxxxxxxxx0x (bcig4) // 2FFD
         // 0011xxxxxxxxxx0x (bcig4) // 3FFD
          if((port & (3 << 14)) == 0 && conf.mem_model == MM_PLUS3)
          {
              unsigned Idx = (port >> 12) & 3;
              switch(Idx)
              {
              case 1: // 1FFD
                  goto set1FFD;
              case 3: // 3FFD
                  Upd765.out(val);
                  return;
              }
          }

         if ((port & 0xC003) == (0x1FFD & 0xC003) && conf.mem_model == MM_KAY)
             goto set1FFD;

         // 00xxxxxxxx1xxx01 (sc16 green)
         if ((port & 0xC023) == (0x1FFD & 0xC023) && (conf.mem_model == MM_SCORP || conf.mem_model == MM_PROFSCORP))
         {
set1FFD:
            comp.p1FFD = val;
            set_banks();
            return;
         }

         // gmx
         if(port == 0x7EFD && conf.mem_model == MM_PROFSCORP)
         {
            comp.p7EFD = val;
            set_banks();
            return;
         }

         if (conf.mem_model == MM_ATM450 && (port & 0x8202) == (0x7DFD & 0x8202))
         {
             atm_writepal(port, val);
             return;
         }

         // if (conf.mem_model == MM_ATM710 && (port & 0x8202) != (0x7FFD & 0x8202)) return; // strict 7FFD decoding on ATM-2

         // 01xxxxxxxx1xxx01 (sc16 green)
         if ((port & 0xC023) != (0x7FFD & 0xC023) && (conf.mem_model == MM_SCORP || conf.mem_model == MM_PROFSCORP))
             return;
         // 0xxxxxxxxxx11x0x
         if ((port & 0x801A) != (0x7FFD & 0x801A) && (conf.mem_model == MM_QUORUM))
             return;

         // 7FFD
         if (comp.p7FFD & 0x20)
         { // 48k lock
            // #EFF7.2 forces lock
            if ((comp.pEFF7 & EFF7_LOCKMEM) && conf.mem_model == MM_PENTAGON && conf.ramsize == 1024)
                return;

			if ((comp.pEFF7 & EFF7_LOCKMEM) && conf.mem_model == MM_ATM3) // lvd added eff7 to atm3
				return;

				
            // if not pentagon-1024 or pentevo (atm3) --(added by lvd)-- or profi with #DFFD.4 set, apply lock
            if (!((conf.ramsize == 1024 && conf.mem_model == MM_PENTAGON) ||
			      (conf.mem_model == MM_ATM3)                             ||
                  (conf.mem_model == MM_PROFI && (comp.pDFFD & 0x10)))) // molodcov_alex
                return;
         }

         if ((comp.p7FFD ^ val) & 0x08)
             update_screen();

         comp.p7FFD = val;
         set_banks();
         return;
      }

      // xx0xxxxxxxxxxx0x (3.2) [vv]
      if ((port & 0x2002) == (0xDFFD & 0x2002) && conf.mem_model == MM_PROFI)
      {
          comp.pDFFD = val;
          set_banks();
          return;
      }

      if (conf.mem_model == MM_ATM450 && (port & 0x8202) == (0xFDFD & 0x8202))
      {
          comp.pFDFD = val;
          set_banks();
          return;
      }

      // 1x0xxxxxxxx11x0x
      if ((port & 0xA01A) == (0x80FD & 0xA01A) && conf.mem_model == MM_QUORUM)
      {
          comp.p80FD = val;
          set_banks();
          return;
      }

      if ((port & 0xC0FF) == 0xC0FD && conf.sound.ay_scheme >= AY_SCHEME_SINGLE)
      { // A15=A14=1, FxFD - AY select register
         if ((conf.sound.ay_scheme == AY_SCHEME_CHRV) && ((val & 0xF0) == 0xF0)) //Alone Coder
         {
            if (conf.sound.ay_chip == (SNDCHIP::CHIP_YM2203))
             {
				 comp.tfmstat = val;
                 fmsoundon0 = val & 4;
                 tfmstatuson0 = val & 2;
             } //Alone Coder 0.36.6
            comp.active_ay = val & 1;
			return;
         };
		 if((conf.sound.saa1099 == SAA_TFM_PRO)&&((comp.tfmstat&CF_TFM_SAA)==0)){
		    Saa1099.WrCtl(val);
		 }else{
            unsigned n_ay = (conf.sound.ay_scheme == AY_SCHEME_QUADRO)? (port >> 12) & 1 : comp.active_ay;
            ay[n_ay].select(val);
		 }
         return;
      }

      if ((port & 0xC000)==0x8000 && conf.sound.ay_scheme)
      {  // BFFD - AY data register
		 if((conf.sound.saa1099 == SAA_TFM_PRO)&&((comp.tfmstat&CF_TFM_SAA)==0)){
		    Saa1099.WrData(temp.sndblock ? 0 : cpu.t, val);
		 }else{
		    unsigned n_ay = (conf.sound.ay_scheme == AY_SCHEME_QUADRO)? (port >> 12) & 1 : comp.active_ay;
            ay[n_ay].write(temp.sndblock? 0 : cpu.t, val);
            if (conf.input.mouse == 2 && ay[n_ay].get_activereg() == 14)
                input.aymouse_wr(val);
		 }
         return;
      }
      return;
   }

   if (conf.sound.sd && (port & 0xAF) == 0x0F)
   { // soundrive
//      __debugbreak();
      if ((unsigned char)port == 0x0F) comp.p0F = val;
      if ((unsigned char)port == 0x1F) comp.p1F = val;
      if ((unsigned char)port == 0x4F) comp.p4F = val;
      if ((unsigned char)port == 0x5F) comp.p5F = val;
      flush_dig_snd();
      sd_l = (conf.sound.sd_vol * (comp.p0F+comp.p1F)) >> 8;
      sd_r = (conf.sound.sd_vol * (comp.p4F+comp.p5F)) >> 8;
      return;
   }
   if (conf.sound.covoxFB && !(port & 4))
   { // port FB - covox
//      __debugbreak();
      flush_dig_snd();
      covFB_vol = val*conf.sound.covoxFB_vol/0x100;
      return;
   }

   if (conf.sound.saa1099 == SAA_ZXM && ((port & 0xFF) == 0xFF)) // saa1099
   {
       if(port & 0x100)
           Saa1099.WrCtl(val);
       else
           Saa1099.WrData(temp.sndblock? 0 : cpu.t, val);
       return;
   }

   if( (port == 0xEFF7) && ( (conf.mem_model==MM_PENTAGON) || (conf.mem_model==MM_ATM3)
	   || conf.mem_model == MM_ATM710 ) ) // lvd added eff7 to atm3
   {
      unsigned char oldpEFF7 = comp.pEFF7; //Alone Coder 0.36.4
      comp.pEFF7 = (comp.pEFF7 & conf.EFF7_mask) | (val & ~conf.EFF7_mask);
	  if(conf.mem_model == MM_ATM710) 
		  return;
      comp.pEFF7 |= EFF7_GIGASCREEN; // [vv] disable turbo
//    if ((comp.pEFF7 ^ oldpEFF7) & EFF7_GIGASCREEN) {
//      conf.frame = frametime;
//      if ((conf.mem_model == MM_PENTAGON)&&(comp.pEFF7 & EFF7_GIGASCREEN))conf.frame = 71680;
//      apply_sound();
//    } //Alone Coder removed 0.37.1

      if (!(comp.pEFF7 & EFF7_4BPP))
      {
          temp.offset_vscroll = 0;
          temp.offset_vscroll_prev = 0;
          temp.offset_hscroll = 0;
          temp.offset_hscroll_prev = 0;
      }

      if ((comp.pEFF7 ^ oldpEFF7) & (EFF7_ROCACHE | EFF7_LOCKMEM))
          set_banks(); //Alone Coder 0.36.4
      return;
   }
   if (conf.cmos && (((comp.pEFF7 & EFF7_CMOS) && 
		(conf.mem_model == MM_PENTAGON || conf.mem_model == MM_ATM710)) || 
		conf.mem_model == MM_ATM3))
   {
      unsigned mask = (conf.mem_model == MM_ATM3 && (comp.flags & CF_DOSPORTS)) ? ~0x100U : 0xFFFF;

      if (port == (0xDFF7 & mask))
      {
          comp.cmos_addr = val;
          return;
      }
      if (port == (0xBFF7 & mask))
      {
	     /*if (comp.cmos_addr >= 0xF0 && (val & 0xf0) == 0x10 && conf.mem_model == MM_ATM3){
			comp.fddIO2Ram_mask=val;
		 }else */
		 if (comp.cmos_addr >= 0xF0 && val <= 2 && conf.mem_model == MM_ATM3)
         {//thims added
            if (val < 2)
            {
               input.buffer_enabled = false;
               static unsigned version = 0;
               if (!version)
               {
                  unsigned day, year;
                  char month[8];
                  static const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec"; 
                  sscanf(__DATE__, "%s %d %d", month, &day, &year);
                  version = day | ((strstr(months, month) - months) / 3 + 1) << 5 | (year - 2000) << 9;
               }
               
               strcpy((char*)cmos + 0xF0, "UnrealSpeccy");
               *(unsigned*)(cmos + 0xFC) = version;
            }
            else input.buffer_enabled = true;
         }
         else cmos_write(val);
         return;
      }
   }
   if ((port & 0xF8FF) == 0xF8EF && modem.open_port)
       modem.write((port >> 8) & 7, val);
}

__inline unsigned char in1(unsigned port)
{
   port &= 0xFFFF;
   brk_port_in = port;

   u8 p1 = (port & 0xFF);

/*
   if((port & 0xFF) == 0xF0)
       __debugbreak();

   if((comp.flags & CF_DOSPORTS) && port == 0xFADF)
       __debugbreak();
*/

   // � ������ ���������� ������ �� ������ 8���

   if(conf.ula_plus && port == 0xFF3B)
   {
       if(comp.ula_plus_group == 0)
       {
           return comp.comp_pal[comp.ula_plus_pal_idx];
       }

       if(comp.ula_plus_group == 1)
       {
           u8 val = comp.ula_plus_en ? 1 : 0;
           return val;
       }
       return 0xFF;
   }

   // ngs
   #ifdef MOD_GS
   if ((port & 0xF7) == 0xB3 && conf.gs_type)
       return in_gs(p1);
   #endif

   // ZXM-MoonSound
   if (conf.sound.moonsound &&
       (conf.mem_model == MM_PROFI ? !(comp.pDFFD & 0x80) : 1) &&
       (((p1 & 0xFC) == 0xC4) || ((p1 & 0xFE) == 0xC2)))
   {
       u8 val = 0xFF;

       if (zxmmoonsound.read(port, val))
           return val;
   }

   // z-controller
   if (conf.zc && (port & 0xFF) == 0x57)
   {
      // no shadow-mode ZXEVO patch here since 0x57 port in read mode is the same
	  // as in noshadow-mode, i.e. no A15 is used to decode port.
      return Zc.Rd(port);
   }
   
   if(conf.wiznet && (port & 0xff) == 0xab ){ 
       return pXXAB_Read(port); 
   } 

   if(conf.mem_model == MM_ATM3)
   {
       // ���� ���������� ���3
       if((port & 0xFF) == 0xBF)
           return comp.pBF;

       if ((port & 0xFF) == 0xBD)
       {
           u8 port_hi = (port >> 8) & 0xFF;
           if((port_hi & ~7) == 0) // ������ �� ���������������� ������ ��������
           {
               unsigned PgIdx = port_hi & 7;
               return (comp.pFFF7[PgIdx] & 0xFF) ^ 0xFF;
           }

           switch(port_hi)
           {
           case 0x8: // ram/rom
           {
               u8 RamRomMask = 0;
               for(unsigned i = 0; i < 8; i++)
                   RamRomMask |= ((comp.pFFF7[i] >> 8) & 1) << i;
               return ~RamRomMask;
           }
           case 0x9: //dos7ffd
           {
               u8 RamRomMask = 0;
               for(unsigned i = 0; i < 8; i++)
                   RamRomMask |= ((comp.pFFF7[i] >> 9) & 1) << i;
               return ~RamRomMask;
           }
           case 0xA: return comp.p7FFD;
           case 0xB: return comp.pEFF7; // lvd - added EFF7 reading in pentevo (atm3)

           // lvd: fixed bug with no-anding bits from aFF77, added CF_TRDOS to bit 4
		   // lvd: changed bit 4 to dos state, remembered during nmi
           case 0xC: return (((comp.aFF77 >> 14) << 7) & 0x0080) | (((comp.aFF77 >> 9) << 6) & 0x0040) | (((comp.aFF77 >> 8) << 5) & 0x0020) | (/*(comp.flags & CF_TRDOS)*/trdos_in_nmi?0x0010:0) | (comp.pFF77 & 0xF);
           case 0xD: return atm_readpal();
		   case 0xE: return zxevo_readfont();
		   
		   // breakpoint address readback
		   case 0x10: return comp.brk_addr&0x00FF;
		   case 0x11: return (comp.brk_addr>>8)&0x00FF;
		   
		   //read fddIO2Ram_mask
		   case 0x13: return comp.fddIO2Ram_mask;
		   
		   //read scanline
		   case 0x14: return ((cpu.t / 224)>>1)&0x00FF;
		   
           }
       }
   }

   // divide �� nemo ������
   if(conf.ide_scheme == IDE_NEMO_DIVIDE)
   {
       if(((port & 0x1E) == 0x10)) // rrr1000x
       {
           if((port & 0xFF) == 0x11)
           {
               comp.ide_hi_byte_r = 0;
               return comp.ide_read;
           }

           if((port & 0xFE) == 0x10)
           {
               comp.ide_hi_byte_r ^= 1;
               if(!comp.ide_hi_byte_r)
               {
                   return comp.ide_read;
               }
           }
           else
           {
               comp.ide_hi_byte_r = 0;
           }
           goto read_hdd_5;
       }
       else if((port & 0xFF) == 0xC8)
       {
        return hdd.read(8);
       }
   }

   // quorum additional keyboard port
   if((conf.mem_model == MM_QUORUM) && ((port & 0xFF) == 0x7E))
   {
      u8 val = input.read_quorum(u8(port >> 8));
      return val;
   }

   if (comp.flags & CF_DOSPORTS)
   {
      if(conf.mem_model == MM_ATM3 && (p1 & 0x1F) == 0x0F && !(((p1 >> 5) - 1) & 4))
      {
          // 2F = 001|01111b
          // 4F = 010|01111b
          // 6F = 011|01111b
          // 8F = 100|01111b
		  return comp.wd_shadow[(p1 >> 5) - 1];
			
     }


      if (conf.ide_scheme == IDE_ATM && (port & 0x1F) == 0x0F)
      {
         if (port & 0x100)
             return comp.ide_read;
      read_hdd_5:
         port >>= 5;
      read_hdd:
         port &= 7;
         if (port)
             return hdd.read(port);
         unsigned v = hdd.read_data();
         comp.ide_read = (unsigned char)(v >> 8);
         return (unsigned char)v;
      }

      if ((port & 0x18A3) == (0xFFFE & 0x18A3))
      { // SMUC
         if (conf.smuc)
         {
            if ((port & 0xA044) == (0xDFBA & 0xA044)) return cmos_read(); // clock
            if ((port & 0xA044) == (0xFFBA & 0xA044)) return comp.nvram.out; // SMUC system port
            if ((port & 0xA044) == (0x7FBA & 0xA044)) return comp.p7FBA | 0x37; // was 0x3F, bit 3 seems to be used in profrom to indicate presence of HDD (Muchkin)
            if ((port & 0xA044) == (0x5FBA & 0xA044)) return 0x3F;
            if ((port & 0xA044) == (0x5FBE & 0xA044)) return 0x57;
            if ((port & 0xA044) == (0x7FBE & 0xA044)) return 0x57;
         }
         if ((port & 0x8044) == (0xFFBE & 0x8044) && conf.ide_scheme == IDE_SMUC)
         { // FFBE, FEBE
            if(comp.pFFBA & 0x80)
            {
                if(!(port & 0x100))
                    return hdd.read(8); // alternate status
                return 0xFF; // obsolete register
            }

            if (!(port & 0x2000))
                return comp.ide_read;
            port >>= 8;
            goto read_hdd;
         }
      }

      if (conf.mem_model == MM_PROFI) // molodcov_alex
      {
          if((comp.p7FFD & 0x10) && (comp.pDFFD & 0x20))
          { // modified ports
            // BDI ports
            if((p1 & 0x9F) == 0x83)
                return comp.wd.in((p1 & 0x60) | 0x1F);  // WD93 ports (1F, 3F, 7F)
            if((p1 & 0xE3) == 0x23)
                return comp.wd.in(0xFF);                // port FF

            // RTC
            if((port & 0x9F) == 0x9F && conf.cmos)
            {
                if(!(port & 0x20))
                    return cmos_read();
            }

            // IDE
            if((p1 & 0x9F) == 0x8B && (conf.ide_scheme == IDE_PROFI))
            {
                if(p1 & 0x40) // cs1
                {
                    if (p1 & 0x20)
                        return comp.ide_read;
                    port >>= 8;
                    goto read_hdd;
                }
            }
          }
          else
          {
              // BDI ports
              if((p1 & 0x83) == 0x03)
                  return comp.wd.in((p1 & 0x60) | 0x1F);  // WD93 ports
              if((p1 & 0xE3) == ((comp.pDFFD & 0x20) ? 0xA3 : 0xE3))
                  return comp.wd.in(0xFF);                // port FF
          }
      }

      if(conf.mem_model == MM_QUORUM /* && !(comp.p00 & Q_TR_DOS) */) // cpm ports
      {
          if((p1 & 0xFC) == 0x80) // 80, 81, 82, 83
          {
              p1 = u8(((p1 & 3) << 5) | 0x1F);
              return comp.wd.in(p1);
          }
      }
          // 1F = 0001|1111b
          // 3F = 0011|1111b
          // 5F = 0101|1111b
          // 7F = 0111|1111b
          // DF = 1101|1111b ���� ����
          // FF = 1111|1111b
      else if ((p1 & 0x9F) == 0x1F || p1 == 0xFF) {// 1F, 3F, 5F, 7F, FF
		  if((comp.flags & CF_TRDOS) && conf.trdos_IORam && ( (comp.aFF77 & 0x4000) != 0 )
				&&((1<<comp.wd.drive)&comp.fddIO2Ram_mask)&&(bankr[0]==base_dos_rom))
		  {
              comp.fddIO2Ram_wr_disable = true;
		      cpu.nmi_in_progress=conf.trdos_IORam;
			  trdos_in_nmi = comp.flags&CF_TRDOS;
			  set_banks(); 
			  return 0xff;
		  }else{
		      if(conf.trdos_IORam && (p1&0x80))
			  {  
				return (comp.wd.in(p1) & 0xE0) | comp.trdos_last_ff;
			  }
			  return comp.wd.in(p1);
		  }
	  }
   }
   else // �� dos
   {
       if(((port & 0xA3) == 0xA3) && (conf.ide_scheme == IDE_DIVIDE))
       {
           if((port & 0xFF) == 0xA3)
           {
               comp.ide_hi_byte_r ^= 1;
               if(!comp.ide_hi_byte_r)
               {
                   return comp.ide_read;
               }
           }
           else
           {
               comp.ide_hi_byte_r = 0;
           }
           port >>= 2;
           goto read_hdd;
       }


       if (!(port & 6) && (conf.ide_scheme == IDE_NEMO || conf.ide_scheme == IDE_NEMO_A8))
       {
          unsigned hi_byte = (conf.ide_scheme == IDE_NEMO)? (port & 1) : (port & 0x100);
          if(hi_byte)
              return comp.ide_read;
          comp.ide_read = 0xFF;
          if((port & 0x18) == 0x08)
              return ((port & 0xE0) == 0xC0)? hdd.read(8) : 0xFF; // CS1=0,CS0=1,reg=6
          if((port & 0x18) != 0x10)
              return 0xFF; // invalid CS0,CS1
          goto read_hdd_5;
       }
   }


   
   
   if (!(port & 0x20))
   { // kempstons
      port = (port & 0xFFFF) | 0xFA00; // A13,A15 not used in decoding
      if ((port == 0xFADF || port == 0xFBDF || port == 0xFFDF) && conf.input.mouse == 1)
      { // mouse
         input.mouse_joy_led |= 1;
         if (port == 0xFBDF)
             return input.kempston_mx();
         if (port == 0xFFDF)
             return input.kempston_my();
         return input.mbuttons;
      }
      input.mouse_joy_led |= 2;
      unsigned char res = (conf.input.kjoy)? input.kjoy : 0xFF;
      if (conf.mem_model == MM_SCORP || conf.mem_model == MM_PROFSCORP)
         res = (res & 0x1F) | (comp.wd.in(0xFF) & 0xE0);
      return res;
   }

   // fuller joystick
   if((p1 == 0x7F) && conf.input.fjoy)
   {
       input.mouse_joy_led |= 2;
       return  input.fjoy;
   }

   // port #FE
   bool pFE;

   // scorp  xx1xxx10 (sc16)
   if((conf.mem_model == MM_SCORP || conf.mem_model == MM_PROFSCORP))
       pFE = ((port & 0x23) == (0xFE & 0x23)) && !(comp.flags & CF_DOSPORTS);
   else if(conf.mem_model == MM_QUORUM) // 1xx11xx0
       pFE = ((port & 0x99) == (0xFE & 0x99));
   else // others xxxxxxx0
       pFE = !(port & 1);

   if (pFE)
   {
      if ((cpu.pc & 0xFFFF) == 0x0564 && bankr[0][0x0564]==0x1F && conf.tape_autostart && comp.tape.stopped)
          start_tape();
      u8 val = input.read(u8(port >> 8));
      if (conf.mem_model == MM_ATM450)
          val = (val & 0x7F) | atm450_z(cpu.t);
      return val;
   }

   if ((port & 0x8202) == (0x7FFD & 0x8202) && (conf.mem_model == MM_ATM710 || conf.ide_scheme == IDE_ATM))
   { // ATM-2 IDE+DAC/ADC
      unsigned char irq = 0x40;
      if (conf.ide_scheme == IDE_ATM) irq = (hdd.read_intrq() & 0x40);
      return irq + 0x3F;
   }

   // 0001xxxxxxxxxx0x (bcig4) // 1FFD
   // 0010xxxxxxxxxx0x (bcig4) // 2FFD
   // 0011xxxxxxxxxx0x (bcig4) // 3FFD
   if((port & ((3 << 14) | 2)) == 0 && conf.mem_model == MM_PLUS3)
   {
       unsigned Idx = (port >> 12) & 3;
       switch(Idx)
       {
       case 2: // 2FFD
           return Upd765.in(Idx);
       case 3: // 3FFD
           return Upd765.in(Idx);
       }
   }

   if ((unsigned char)port == 0xFD && conf.sound.ay_scheme >= AY_SCHEME_SINGLE)
   {
      if((conf.sound.ay_scheme == AY_SCHEME_CHRV) && (conf.sound.ay_chip == (SNDCHIP::CHIP_YM2203)) && (tfmstatuson0 == 0))
          return 0x7f /*always ready*/; //Alone Coder 0.36.6
      if ((port & 0xC0FF) != 0xC0FD) return 0xFF;
      unsigned n_ay = (conf.sound.ay_scheme == AY_SCHEME_QUADRO)? (port >> 12) & 1 : comp.active_ay;
      // else FxFD - read selected AY register
      if (conf.input.mouse == 2 && ay[n_ay].get_activereg() == 14) { input.mouse_joy_led |= 1; return input.aymouse_rd(); }
      return ay[n_ay].read();
   }

//   if ((port & 0x7F) == 0x7B) { // FB/7B
   if ((port & 0x04) == 0x00)
   { // FB/7B //Alone Coder 0.36.6 (for MODPLAYi)
      if (conf.mem_model == MM_ATM450)
      {
         comp.aFB = (unsigned char)port;
         set_banks();
      }
      else if (conf.cache)
      {
         comp.flags &= ~CF_CACHEON;
         if (port & 0x80) comp.flags |= CF_CACHEON;
         set_banks();
      }
      return 0xFF;
   }

   if (conf.cmos && ((comp.pEFF7 & EFF7_CMOS) || conf.mem_model == MM_ATM3 || conf.mem_model == MM_ATM710))
   {
      unsigned mask = (conf.mem_model == MM_ATM3 && (comp.flags & CF_DOSPORTS)) ? ~0x100U : 0xFFFF;
      if(port == (0xBFF7 & mask))
          return cmos_read();
   }

   if ((port & 0xF8FF) == 0xF8EF && modem.open_port)
       return modem.read((port >> 8) & 7);

   if (conf.portff && ((port & 0xFF) == 0xFF))
   {
      update_screen();
      if (vmode != 2) return 0xFF; // ray is not in paper
      unsigned ula_t = (cpu.t+temp.border_add) & temp.border_and;
      return temp.base[vcurr->atr_offs + (ula_t - vcurr[-1].next_t)/4];
   }
   return 0xFF;
}

unsigned char in(unsigned port)
{
   brk_port_val = in1(port);
   return brk_port_val;
}

#undef in_trdos
#undef out_trdos
