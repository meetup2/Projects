module InvAddRoundKey 
(
input logic[127:0] input0,
input	logic[127:0] key,
output logic[127:0] output0
);
							  
assign output0 = input0 ^ key;

endmodule