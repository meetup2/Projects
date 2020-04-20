module AES 
(
input	 logic clk,
input  logic RESET,
input  logic AES_START,
output logic AES_DONE,
input  logic [127:0] AES_KEY,
input  logic [127:0] AES_MSG_ENC,
output logic [127:0] AES_MSG_DEC
);
logic ld_msg;
logic [1:0] wordSelectBits;
logic [3:0] stateSelectBits;
logic [4:0] update;
logic [31:0] invColValue;
logic [31:0] word;
logic [127:0] input0; 
logic [127:0] invRowShiftValue;
logic [127:0] invSubBytes;
logic [127:0] key; 
logic [127:0] nextState; 
logic [127:0] output0; 
logic [1407:0] KeySchedule;

KeyExpansion keyExpansion(.*, .Cipherkey(AES_KEY));
always_comb
begin
	key = KeySchedule[127:0];
	unique case(update)
		5'd0: key = KeySchedule[127:0];
		5'd1: key = KeySchedule[255:128];
		5'd2: key = KeySchedule[383:256];
		5'd3: key = KeySchedule[511:384];
		5'd4: key = KeySchedule[639:512];
		5'd5: key = KeySchedule[767:640];
		5'd6: key = KeySchedule[895:768];
		5'd7: key = KeySchedule[1023:896];
		5'd8: key = KeySchedule[1151:1024];
		5'd9: key = KeySchedule[1279:1152];
		5'd10: key = KeySchedule[1407:1280];
	endcase	
end

always_comb
begin
	word = input0[31:0];
	case(wordSelectBits)
		2'b00: word = input0[31:0];
		2'b01: word = input0[63:32];
		2'b10: word = input0[95:64];
		2'b11: word = input0[127:96];
	endcase
end

always_comb
begin
	nextState = input0;
	case(stateSelectBits)
		4'h1: nextState = output0; //operation for inv add roundkey
		4'h2: nextState = invRowShiftValue; //invrowshift
		4'h4: nextState = invSubBytes; //invsubbyte
		4'h8:
			begin
				case(wordSelectBits)
					2'b00: nextState[31:0] = invColValue;
					2'b01: nextState[63:32] = invColValue;
					2'b10: nextState[95:64] = invColValue;
					2'b11: nextState[127:96] = invColValue;
				endcase
			end
	endcase
end

InvSubBytes subby1(.clk(clk), .in(input0[7:0]), .out(invSubBytes[7:0]));
InvSubBytes subby2(.clk(clk), .in(input0[15:8]), .out(invSubBytes[15:8]));
InvSubBytes subby3(.clk(clk), .in(input0[23:16]), .out(invSubBytes[23:16]));
InvSubBytes subby4(.clk(clk), .in(input0[31:24]), .out(invSubBytes[31:24]));
InvSubBytes subby5(.clk(clk), .in(input0[39:32]), .out(invSubBytes[39:32]));
InvSubBytes subby6(.clk(clk), .in(input0[47:40]), .out(invSubBytes[47:40]));
InvSubBytes subby7(.clk(clk), .in(input0[55:48]), .out(invSubBytes[55:48]));
InvSubBytes subby8(.clk(clk), .in(input0[63:56]), .out(invSubBytes[63:56]));
InvSubBytes subby9(.clk(clk), .in(input0[71:64]), .out(invSubBytes[71:64]));
InvSubBytes subby10(.clk(clk), .in(input0[79:72]), .out(invSubBytes[79:72]));
InvSubBytes subby11(.clk(clk), .in(input0[87:80]), .out(invSubBytes[87:80]));
InvSubBytes subby12(.clk(clk), .in(input0[95:88]), .out(invSubBytes[95:88]));
InvSubBytes subby13(.clk(clk), .in(input0[103:96]), .out(invSubBytes[103:96]));
InvSubBytes subby14(.clk(clk), .in(input0[111:104]), .out(invSubBytes[111:104]));
InvSubBytes subby15(.clk(clk), .in(input0[119:112]), .out(invSubBytes[119:112]));
InvSubBytes subby16(.clk(clk), .in(input0[127:120]), .out(invSubBytes[127:120]));
InvAddRoundKey invKey(.*);
InvShiftRows invRowShift(.data_in(input0), .data_out(invRowShiftValue));
InvMixColumns mixInv(.in(word), .out(invColValue));

decrypt_state_machine decrypt_state_machine(.*, .aes_done(AES_DONE));

always_ff @ (posedge clk)
begin
	if(RESET)
	begin
		ld_msg <= 1'b1;
		AES_MSG_DEC <= 128'b0;
		input0 <= 128'b0;
	end
	else if(AES_START && ld_msg)
	begin
		input0 <= AES_MSG_ENC;		
		ld_msg <= 1'b0;
	end
	else if(AES_START && !ld_msg)
	begin
		AES_MSG_DEC <= nextState;
		input0 <= nextState;
		ld_msg <= ld_msg;
	end
	else if (!AES_START)
	begin
		ld_msg <= 1'b1;
	end
end


endmodule

module decrypt_state_machine
(
input logic clk, RESET, AES_START,
output logic aes_done,
output logic [1:0] wordSelectBits,
output logic [3:0] stateSelectBits,
output logic [4:0] update
);

enum logic [4:0] 
{
Hold, FirstKey, InvShiftRows, last_RoundKey, InvSubBytes, last_InvShiftRows, last_InvSubBytes, RoundKey, InvMixCol0, InvMixCol1, InvMixCol2, InvMixCol3, Finish //mix col needs 4 states, in and out only 32 bits
} State, Next_State;
logic [4:0] next_update;

always_ff @ (posedge clk)
begin
	if(RESET)
		begin
			update <= 5'd0;
			State <= Hold;
		end
	else
		begin
			update <= next_update;
			State <= Next_State;
		end
end

always_comb
begin
	Next_State = State;

	unique case(State)
		Hold:
			begin
				if(AES_START)
					Next_State = FirstKey;
				else
					Next_State = Hold;
			end
		
		FirstKey:
			Next_State = InvShiftRows;

		InvShiftRows:
			Next_State = InvSubBytes;

		InvSubBytes:
			Next_State = RoundKey;
		
		RoundKey:
			Next_State = InvMixCol0;

		InvMixCol0:
			Next_State = InvMixCol1;
		InvMixCol1:
			Next_State = InvMixCol2;
		InvMixCol2:
			Next_State = InvMixCol3;
		InvMixCol3:
		if(update != 10)
			Next_State = InvShiftRows;
		else 
			Next_State = last_InvShiftRows; //triggers last cycle where we don't wanna invmixcol
		last_InvShiftRows:
			Next_State = last_InvSubBytes;
		last_InvSubBytes:
			Next_State = last_RoundKey;
		last_RoundKey:
			Next_State = Finish;
		Finish:
			begin
				if(!AES_START) //can't perform another decryption until aes_start is flipped off and on again
					Next_State = Hold;
				else
					Next_State = Finish;
			end
	endcase
end

always_comb
begin

	next_update = update;
	stateSelectBits = 4'h0;
	wordSelectBits = 2'b00;
	aes_done = 1'b0;

	case(State)

		Hold:
			begin
				if(AES_START)
					begin
					next_update = 5'd0;
					end
			end
		InvShiftRows:
			begin
				stateSelectBits = 4'h2;
			end
		InvSubBytes:
			begin
				stateSelectBits = 4'h4;
			end
		last_InvShiftRows:
			begin
				stateSelectBits = 4'h2;
			end
		last_InvSubBytes:
			begin
				stateSelectBits = 4'h4;
			end
		InvMixCol0:
			begin
				stateSelectBits = 4'h8;
				wordSelectBits = 2'b00;
			end
		InvMixCol1:
			begin
				stateSelectBits = 4'h8;
				wordSelectBits = 2'b01;
			end
		InvMixCol2:
			begin
				stateSelectBits = 4'h8;
				wordSelectBits = 2'b10;
			end
		InvMixCol3:
			begin
				stateSelectBits = 4'h8;
				wordSelectBits = 2'b11;
			end
		FirstKey:
			begin
				stateSelectBits = 4'h1;
				next_update = 5'd1;
			end

		RoundKey:
			begin
				next_update = update + 5'd1;
				stateSelectBits = 4'h1;
			end
		last_RoundKey:
			begin
				stateSelectBits = 4'h1;
			end
		Finish:
			aes_done = 1'b1;
	endcase		
end

endmodule