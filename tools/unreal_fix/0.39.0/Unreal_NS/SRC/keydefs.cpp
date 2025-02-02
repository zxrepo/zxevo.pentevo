#include "std.h"

#include "emul.h"
#include "vars.h"
#include "emulkeys.h"
#include "snapshot.h"
#include "gui.h"
#include "savesnd.h"
#include "cheat.h"
#include "dbgreg.h"
#include "dbgcmd.h"
#include "dbgoth.h"
#include "dbgrwdlg.h"
#include "dbgbpx.h"
#include "dbgtrace.h"
#include "dbglabls.h"
#include "dbgmem.h"
#include "funcs.h"

//=============================================================================
action ac_main[] =
{
    { "main.exit", 		correct_exit 		},
    { "main.monitor", 		main_debug 		},
    { "main.fullscreen",	main_fullscr 		},
    { "main.pause", 		main_pause 		},
    { "main.selectfix", 	main_selectfix 		},
    { "main.selectsnd", 	main_selectsnd 		},
    { "main.incfix", 		main_incfix 		},
    { "main.decfix", 		main_decfix 		},
    { "main.incfix10", 		main_incfix10 		},
    { "main.decfix10", 		main_decfix10 		},
    { "main.leds", 		main_leds 		},
    { "main.maxspeed", 	  	main_maxspeed     	},
    { "main.selectfilter", 	main_selectfilter 	},
    { "main.selectdriver", 	main_selectdriver 	},
    { "main.pokedialog",	main_poke 	  	},
    { "main.starttape",		main_starttape 		},
    { "main.screenshot",	main_scrshot 		},
    { "main.savevideo",		main_savevideo 		},
    { "main.reset",		main_reset 		},
    { "main.reset128",		main_reset128 		},
    { "main.resetsys",		main_resetsys 		},
    { "main.reset48",		main_reset48 		},
    { "main.resetbasic",	main_resetbas 		},
    { "main.resetdos",		main_resetdos 		},
    { "main.resetcache",	main_resetcache		},
    { "main.nmi",		main_nmi 		},
    { "main.nmidos",		main_nmidos 		},
    { "main.nmicache",	 	main_nmicache 	 	},
    { "main.tapebrowser",	main_tapebrowser 	},
    { "main.settings",		setup_dlg 	 	},
    { "main.save",		savesnap 		},
    { "main.load",		opensnap 		},
    { "main.savesound",		savesnddialog 		},
    { "main.qsave1", 		qsave1 			},
    { "main.qsave2", 		qsave2 			},
    { "main.qsave3", 		qsave3 			},
    { "main.qload1", 		qload1 			},
    { "main.qload2", 		qload2 			},
    { "main.qload3", 		qload3 			},
    { "main.keystick", 		main_keystick 		},
    { "main.autofire", 		main_autofire 		},
    { "main.saveall", 		main_save 		},
    { "main.lockmouse", 	main_mouse 		},
    { "main.xtkbd", 		main_atmkbd 		},
    { "main.pastetext", 	main_pastetext 		},
    { "main.size1", 		main_size1 		},
    { "main.size2", 		main_size2 		},
    { "main.sizem", 		main_sizem 		},
    { "main.memsearch", 	main_cheat 	  	},
    { "main.border_none",  	main_border_none  	},
    { "main.border_small", 	main_border_small 	},
    { "main.border_full",  	main_border_full  	},
    { "main.help", 	  	main_help 	  	},
    { "main.unlock_mouse", 	main_unlock_mouse 	},

    { nullptr, nullptr },
};
//=============================================================================


//=============================================================================
action ac_main_xt[] =
{
    { "main.xtkbd", main_atmkbd },

    { nullptr, nullptr },
};
//=============================================================================


//=============================================================================
#ifdef MOD_MONITOR

/*
//=============================================================================
// ���������� ������� ������� (�������� � ����� ���� ���������)
#define ac_mon						\
   { "mon.exit",	correct_exit		},	\
   { "mon.emul",	mon_emul		},	\
   { "mon.gs",		mon_gsdialog		},	\
   { "mon.saveblock",	mon_save		},	\
   { "mon.loadblock",	mon_load		},	\
   { "mon.fillblock",	mon_fill		},	\
   { "mon.bpdialog",	mon_bpdialog		},	\
   { "mon.pokedialog",	main_poke		},	\
   { "mon.tapebrowser",	main_tapebrowser	},	\
   { "mon.reset",	main_reset		},	\
   { "mon.reset128",	main_reset128		},	\
   { "mon.resetsys",	main_resetsys		},	\
   { "mon.reset48",	main_reset48		},	\
   { "mon.resetbasic",	main_resetbas		},	\
   { "mon.resetdos",	main_resetdos		},	\
   { "mon.resetcache",	main_resetcache		},	\
   { "mon.nmi",		main_nmi		},	\
   { "mon.nmidos",	main_nmidos		},	\
   { "mon.nmicache",	main_nmicache		},	\
   { "mon.settings",	mon_setup_dlg		},	\
   { "mon.save",	savesnap		},	\
   { "mon.load",	opensnap		},	\
   { "mon.savesound",	savesnddialog		},	\
   { "mon.qsave1",	qsave1			},	\
   { "mon.qsave2",	qsave2			},	\
   { "mon.qsave3",	qsave3			},	\
   { "mon.qload1",	qload1			},	\
   { "mon.qload2",	qload2			},	\
   { "mon.qload3",	qload3			},	\
   { "mon.screen",	mon_scr0		},	\
   { "mon.altscreen",	mon_scr1		},	\
   { "mon.rayscreen",	mon_scray		},	\
   { "mon.step",	mon_step		},	\
   { "mon.stepover",	mon_stepover		},	\
   { "mon.exitsub",	mon_exitsub		},	\
   { "mon.setbank",	editbank		},	\
   { "mon.sethimem",	editextbank		},	\
   { "mon.dump",	mon_dump		},	\
   { "mon.switchdump",	mon_switch_dump		},	\
   { "mon.next",	mon_nxt			},	\
   { "mon.prev",	mon_prv			},	\
   { "mon.rip",		mon_tool		},	\
   { "mon.osw",		mon_watchdialog		},	\
   { "mon.help",	mon_help		},	\
   { "mon.scrshot",	mon_scrshot		},	\
   { "mon.setwatch",	mon_setwatch		},	\
   { "mon.switchay",	mon_switchay		},	\
   { "mon.memsearch",	main_cheat		},	\
   { "mon.labels",	mon_show_labels		},	\
   { "mon.cpu",		mon_switch_cpu		},
//=============================================================================
*/


// ���������� ������ � ��������			// [NS] ������ ��������� �������
//						// �� ������ ����� ���� �� ������ ���������� ��������
action ac_mon[] =
{	
    { "mon.exit",		correct_exit		},
    { "mon.emul",		mon_emul		},
    { "mon.gs",			mon_gsdialog		},
    { "mon.saveblock",		mon_save		},
    { "mon.loadblock",		mon_load		},
    { "mon.fillblock",		mon_fill		},
    { "mon.bpdialog",		mon_bpdialog		},
    { "mon.pokedialog",		main_poke		},
    { "mon.tapebrowser",	main_tapebrowser	},
    { "mon.reset",		main_reset		},
    { "mon.reset128",		main_reset128		},
    { "mon.resetsys",		main_resetsys		},
    { "mon.reset48",		main_reset48		},
    { "mon.resetbasic",		main_resetbas		},
    { "mon.resetdos",		main_resetdos		},
    { "mon.resetcache",		main_resetcache		},
    { "mon.nmi",		main_nmi		},
    { "mon.nmidos",		main_nmidos		},
    { "mon.nmicache",		main_nmicache		},
    { "mon.settings",		mon_setup_dlg		},
    { "mon.save",		savesnap		},
    { "mon.load",		opensnap		},
    { "mon.savesound",		savesnddialog		},
    { "mon.qsave1",		qsave1			},
    { "mon.qsave2",		qsave2			},
    { "mon.qsave3",		qsave3			},
    { "mon.qload1",		qload1			},
    { "mon.qload2",		qload2			},
    { "mon.qload3",		qload3			},
    { "mon.screen",		mon_scr0		},
    { "mon.altscreen",		mon_scr1		},
    { "mon.rayscreen",		mon_scray		},
    { "mon.step",		mon_step		},
    { "mon.step_x2",		mon_step_x2		},	// [NS]
    { "mon.step_x4",		mon_step_x4		},	// [NS]
    { "mon.step_x8",		mon_step_x8		},	// [NS]
    { "mon.step_x16",		mon_step_x16		},	// [NS]
    { "mon.step_x32",		mon_step_x32		},	// [NS]
    { "mon.step_x64",		mon_step_x64		},	// [NS]
    { "mon.step_x128",		mon_step_x128		},	// [NS]
    { "mon.step_x256",		mon_step_x256		},	// [NS]
    { "mon.stepover",		mon_stepover		},
    { "mon.stepover_jump",	mon_stepover_jump	},	// [NS]
    { "mon.exitsub",		mon_exitsub		},
    { "mon.setbank",		editbank		},
    { "mon.sethimem",		editextbank		},
    { "mon.dump",		mon_dump		},
    { "mon.switchdump",		mon_switch_dump		},
    { "mon.next",		mon_nxt			},
    { "mon.prev",		mon_prv			},
    { "mon.rip",		mon_tool		},
    { "mon.osw",		mon_watchdialog		},
    { "mon.help",		mon_help		},
    { "mon.scrshot",		mon_scrshot		},
    { "mon.setwatch",		mon_setwatch		},
    { "mon.switchay",		mon_switchay		},
    { "mon.memsearch",		main_cheat		},
    { "mon.labels",		mon_show_labels		},
    { "mon.cpu",		mon_switch_cpu		},
    { nullptr, nullptr }
};
   
//=============================================================================
action ac_regs[] =
{
    { "reg.left",	rleft		},
    { "reg.right",	rright		},
    { "reg.up",		rup		},
    { "reg.down",	rdown		},
    { "reg.edit",	renter		},
    { "reg.a",		ra		},
    { "reg.f",		rf		},
    { "reg.bc",		rbc		},
    { "reg.de",		rde		},
    { "reg.hl",		rhl		},
    { "reg.pc",		rpc		},
    { "reg.sp",		rsp		},
    { "reg.ix",		rix		},
    { "reg.iy",		riy		},
    { "reg.i",		ri		},
    { "reg.r",		rr		},
    { "reg.im",		rm		},
    { "reg.iff1",	r_1		},
    { "reg.iff2",	r_2		},
    { "reg.SF",		rSF		},
    { "reg.ZF",		rZF		},
    { "reg.F5",		rF5		},
    { "reg.HF",		rHF		},
    { "reg.F3",		rF3		},
    { "reg.PF",		rPF		},
    { "reg.NF",		rNF		},
    { "reg.CF",		rCF		},
    { "reg.codejump",	rcodejump	},
    { "reg.datajump",	rdatajump	},
   //ac_mon					[NS] ������ ��������� �������

   { nullptr, nullptr },
};
//=============================================================================


//=============================================================================
action ac_trace[] =
{
    { "cpu.findpc", 	cfindpc 	},
    { "cpu.here", 	chere 		},
    { "cpu.findtext", 	cfindtext 	},
    { "cpu.findcode", 	cfindcode 	},
    { "cpu.goto", 	cgoto 		},
    { "cpu.bpx", 	cbpx 		},
    { "cpu.asm", 	center 		},
    { "cpu.setpc", 	csetpc 		},
    { "cpu.up", 	cup 		},
    { "cpu.down", 	cdown 		},
    { "cpu.left", 	cleft 		},
    { "cpu.right", 	cright 		},
    { "cpu.pgdn", 	cpgdn 		},
    { "cpu.pgup", 	cpgup 		},
    { "cpu.save1", 	csave1 		},
    { "cpu.save2", 	csave2 		},
    { "cpu.save3", 	csave3 		},
    { "cpu.save4", 	csave4 		},
    { "cpu.save5", 	csave5 		},
    { "cpu.save6", 	csave6 		},
    { "cpu.save7", 	csave7 		},
    { "cpu.save8", 	csave8 		},
    { "cpu.rest1", 	crest1 		},
    { "cpu.rest2", 	crest2 		},
    { "cpu.rest3", 	crest3 		},
    { "cpu.rest4", 	crest4 		},
    { "cpu.rest5", 	crest5 		},
    { "cpu.rest6", 	crest6 		},
    { "cpu.rest7", 	crest7 		},
    { "cpu.rest8", 	crest8 		},
    { "cpu.back",  	pop_pos 	},
    { "cpu.context", 	cjump 		},
    { "cpu.datajump", 	cdjump 		},
    { "cpu.labels", 	cfliplabels 	},
    { "cpu.importl", 	c_lbl_import 	},
    //ac_mon					[NS] ������ ��������� �������
    { nullptr, nullptr },
};
//=============================================================================


//=============================================================================
action ac_mem[] =
{
    { "mem.left",	mleft },
    { "mem.right",	mright },
    { "mem.up", 	mup },
    { "mem.down", 	mdown },
    { "mem.pgup", 	mpgup },
    { "mem.pgdn", 	mpgdn },
    { "mem.switch", 	mswitch },
    { "mem.stline", 	mstl },
    { "mem.endline", 	mendl },
    { "mem.findtext", 	mtext },
    { "mem.findcode", 	mcode },
    { "mem.goto", 	mgoto },
    { "mem.mem", 	mmodemem },
    { "mem.diskphys", 	mmodephys },
    { "mem.disklog", 	mmodelog },
    { "mem.diskgo", 	mdiskgo },
    { "mem.pc", 	mpc },
    { "mem.sp", 	msp },
    { "mem.bc", 	mbc },
    { "mem.de", 	mde },
    { "mem.hl", 	mhl },
    { "mem.ix", 	mix },
    { "mem.iy", 	miy },
    { "mem.addr_edit", 	menter },	// [NS]
    //ac_mon				// [NS] ������ ��������� �������
    
    { nullptr, nullptr },
};
//=============================================================================
#endif // MOD_MONITOR
//=============================================================================