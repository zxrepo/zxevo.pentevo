module pfpzu(
    input oe_n,
	input res_n,
	input [15:2] a,
	output a16,a17
	
);

	reg [1:0] plane;
	reg [1:0] newplane;

	reg enable_switch;



	assign a17 = plane[1];
	assign a16 = plane[0];
	
	
	always @*
	begin
		if( a[15:4] == 12'b100000010000 )
			enable_switch <= 1;
		else
			enable_switch <= 0;
	end


	always @(negedge oe_n)
	if (oe_n==1'b0)
	begin
		case( {a[3:2],plane[1:0]} )
		4'b0000:
			newplane <= 0;
		4'b0001:
			newplane <= 1;
		4'b0010:
			newplane <= 2;
		4'b0011:
			newplane <= 3;
		4'b0100:
			newplane <= 3;
		4'b0101:
			newplane <= 3;
		4'b0110:
			newplane <= 3;
		4'b0111:
			newplane <= 2;
		4'b1000:
			newplane <= 2;
		4'b1001:
			newplane <= 2;
		4'b1010:
			newplane <= 0;
		4'b1011:
			newplane <= 1;
		4'b1100:
			newplane <= 1;
		4'b1101:
			newplane <= 0;
		4'b1110:
			newplane <= 1;
		4'b1111:
			newplane <= 0;
		endcase
	end




	always @(posedge oe_n, negedge res_n)
	begin
		if (res_n==1'b0)
			plane <= 2'b00;
		else	
		if( enable_switch==1 )
		begin
			plane <= newplane;
		end
	end


endmodule
