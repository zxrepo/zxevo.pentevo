// PentEvo project (c) NedoPC 2008-2010

// ATM-like memory pager (it pages specific 16kb region)
//  with additions to support 4m addressable memory
//  and pent1m mode.
//
// contain ports 3FF7,7FF7,BFF7,FFF7 as well as 37F7, 77F7, B7F7, F7F7

`include "../include/tune.v"

module atm_pager(

	input  wire rst_n,

	input  wire fclk,
	input  wire zpos,
	input  wire zneg,


	input  wire [15:0] za, // Z80 address bus

	input  wire [ 7:0] zd, // Z80 data bus - for latching port data

	input  wire        mreq_n,rd_n,m1_n, // to track DOS turn on/turn off


	input  wire        pager_off, // PEN as in ATM2: turns off memory paging, service ROM is everywhere


	input  wire        pent1m_ROM,    // for memory maps switching: d4 of 7ffd
	input  wire [ 5:0] pent1m_page,   // 1 megabyte from pentagon1024 addressing, from 7FFD port
	input  wire        pent1m_ram0_0, // RAM0 to the window 0 from pentagon1024 mode
	input  wire        pent1m_1m_on,  // 1 meg addressing of pent1m mode on

	input  wire        in_nmi, // when we are in nmi, in 0000-3FFF must be last (FFth)
	                           // RAM page. analoguous to pent1m_ram0_0
				   // but has higher priority

	input  wire        atmF7_wr, // write strobe for the xxF7 ATM port


	input  wire        dos, // indicates state of computer: also determines ROM mapping


	output wire        dos_turn_on,  // turns on or off DOS signal
	output wire        dos_turn_off, //

	output wire        zclk_stall, // stall Z80 clock during DOS turning on

	output reg  [ 7:0] page,
	output reg         romnram,

	// output for xxBE port read
	output wire	[ 7:0] rd_page0,
	output wire	[ 7:0] rd_page1,
	output wire [ 1:0] rd_dos7ffd,
	output wire [ 1:0] rd_ramnrom
);
	parameter ADDR = 2'b00;


	reg [ 7:0] pages [0:1]; // 2 pages for each map - switched by pent1m_ROM

	reg [ 1:0] ramnrom; // ram(=1) or rom(=0)
	reg [ 1:0] dos_7ffd; // =1 7ffd bits (ram) or DOS enter mode (rom) for given page

	reg mreq_n_reg, rd_n_reg, m1_n_reg;

	wire dos_exec_stb, ram_exec_stb;


	reg [2:0] stall_count;



	// output data for port xxBE
	assign rd_page0 = pages[0];
	assign rd_page1 = pages[1];
	//
	assign rd_dos7ffd = dos_7ffd;
	assign rd_ramnrom = ramnrom;



	// paging function, does not set pages, ramnrom, dos_7ffd
	//
	always @(posedge fclk)
	begin
		if( pager_off )
		begin // atm no pager mode - each window has same ROM
			romnram <= 1'b1;
			page    <= 8'hFF;
		end
		else // pager on
		begin
			if( (ADDR==2'b00) && (pent1m_ram0_0 || in_nmi) ) // pent ram0 OR nmi
			begin
				if( in_nmi )
				begin
					romnram <= 1'b0;
					page    <= 8'hFF;
				end
				else // if( pent1m_ram0_0 )
				begin
					romnram <= 1'b0;
					page    <= 8'd0;
				end
			end
			else
			begin
				romnram <= ~ramnrom[ pent1m_ROM ];

				if( dos_7ffd[ pent1m_ROM ] ) // 7ffd memmap switching
				begin
					if( ramnrom[ pent1m_ROM ] )
					begin // ram map
						if( pent1m_1m_on )
						begin // map whole Mb from 7ffd to atm pages
							page <= { pages[ pent1m_ROM ][7:6], pent1m_page[5:0] };
						end
						else //128k like in atm2
						begin
							page <= { pages[ pent1m_ROM ][7:3], pent1m_page[2:0] };
						end
					end
					else // rom map with dos
					begin
						page <= { pages[ pent1m_ROM ][7:1], dos };
					end
				end
				else // no 7ffd impact
				begin
					page <= pages[ pent1m_ROM ];
				end
			end
		end
	end




	// port reading: sets pages, ramnrom, dos_7ffd
	//
	always @(posedge fclk) if( atmF7_wr )
	begin
		if( za[15:14]==ADDR )
		begin
			if( za[11] ) // xff7 ports - 1 meg
			begin
				pages   [ pent1m_ROM ] <= ~{ 2'b11, zd[5:0] };
				ramnrom [ pent1m_ROM ] <= zd[6];
				dos_7ffd[ pent1m_ROM ] <= zd[7];
			end
			else // x7f7 ports - 4 meg ram
			begin
				pages   [ pent1m_ROM ] <= ~zd;
				ramnrom [ pent1m_ROM ] <= 1'b1; // RAM on
				// dos_7ffd - UNCHANGED!!! (possibility to use 7ffd 1m and 128k addressing in the whole 4m!)
			end
		end
	end


	// DOS turn on/turn off
	//

`ifdef SIMULATE
	initial
	begin
		m1_n_reg   = 1'b1;
		mreq_n_reg = 1'b1;
		rd_n_reg   = 1'b1;

		stall_count = 3'b000;
	end
`endif

	always @(posedge fclk) if( zpos )
	begin
		m1_n_reg <= m1_n;
	end

	always @(posedge fclk) if( zneg )
	begin
		mreq_n_reg <= mreq_n;
	end



	assign dos_exec_stb = zneg && (za[15:14]==ADDR) &&
	                      (!m1_n_reg) && (!mreq_n) && mreq_n_reg &&
	                      (za[13:8]==6'h3D) &&
	                      dos_7ffd[1'b1] && (!ramnrom[1'b1]) && pent1m_ROM;

	assign ram_exec_stb = zneg && (za[15:14]==ADDR) &&
	                      (!m1_n_reg) && (!mreq_n) && mreq_n_reg &&
	                      ramnrom[pent1m_ROM];

	assign dos_turn_on  = dos_exec_stb;
	assign dos_turn_off = ram_exec_stb;


	// stall Z80 for some time when dos turning on to allow ROM chip to supply new data
	// this can be important at 7 or even 14 mhz. minimum stall time is
	// 3 clocks @ 28 MHz
	always @(posedge fclk)
	begin
		// ������������ � ��� ��� ���������� �� �������� z80 �� ����, ���
		// z80 ������� ������. �.�. � ��� �������� ��� ������ ����� ������.
		// 3.5��� - 140 ���, 7��� - 70 ���, 14��� - 35 ���.
		// ��� ����� 120�� �� 14��� ���� ��� 3 �������� ��������, ��� �������
		// �������, �������� � ����� �������� �� ����� ������� ������� 3 �����
		// 28 ���.
		if( dos_turn_on )
		begin
			stall_count[2] <= 1'b1; // count: 000(stop) -> 101 -> 110 -> 111 -> 000(stop)
			stall_count[0] <= 1'b1;
		end
		else if( stall_count[2] )
		begin
			stall_count[2:0] <= stall_count[2:0] + 3'd1;
		end

	end

	assign zclk_stall = dos_turn_on | stall_count[2];



endmodule

