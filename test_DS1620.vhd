--  test_DS1620.vhd
-- test the unit: ds1620 which just reads the 
-- serial port of the AVR and returns the raw_data(s)

-- AVR -> FPGA -> RS-232 -> monitor
-- AVR to FPGA uses 5v->3v3 conv
-- FPGA to RS-232 uses 3v3 conv to RS-232 conv to monitor (linux box)
-- tx_uart is P77
-- rx_ds1620 is P75

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_signed.all;
--use IEEE.MATH_REAL.ALL;

library XESS;
use XESS.CommonPckg.all;
--use XESS.PulsePckg.all;
--use XESS.DelayPckg.all;
--use XESS.PulsePckg.all;

entity test_DS1620 is
	port(
		clk, reset: in std_logic;
		tx_uart: out std_logic;
		rx_ds1620 : in std_logic;
		led1: out std_logic_vector(3 downto 0)
		);
end test_DS1620;

architecture truck_arch of test_DS1620 is

	-- send_uart1
	type state_uart1 is (xmit_idle, xmit_start, xmit_check_FF,
	xmit_check_FF2, xmit_presend, xmit_send, xmit_wait, xmit_inc, xmit_delay);
	signal state_tx1_reg, state_tx1_next: state_uart1;

	type state_txdly is (idled, startd, procd1);
	signal state_txdly_reg, state_txdly_next: state_txdly;
	
	signal time_delay_reg, time_delay_next: unsigned(24 downto 0);
	signal time_delay_reg2, time_delay_next2: unsigned(24 downto 0);

	signal ds1620_done: std_logic;
	signal start_tx: std_logic;
	signal pstart_tx: std_logic;
	signal data_tx: std_logic_vector(7 downto 0);
	signal high_byte, low_byte: std_logic_vector(7 downto 0);
	signal done_tx: std_logic;
	signal raw_data1: std_logic_vector(15 downto 0);
	signal start_ds1620: std_logic;
	signal index1: std_logic_vector(1 downto 0);
	signal temp_data1, temp_data2, temp_data3, temp_data4: std_logic_vector(15 downto 0);
	constant NO_ELEMS:  integer:=  16;
	type xmit_array_type is array(0 to NO_ELEMS-1) of std_logic_vector(7 downto 0);
	signal xmit_array: xmit_array_type:=(others=>(others=>'0'));
	signal xmit_uindex: unsigned(3 downto 0);
	signal skip: std_logic;
	signal skip2: std_logic;
	signal xmit_stdlv: std_logic_vector(7 downto 0);
	signal done: std_logic;
	-- no of elements in xmit_array minus the 1st one which is 0xFF
	
begin

wrapper_DS1620_unit: entity work.poll_DS1620
	port map(clk=>clk, reset=>reset,
		rx_ds1620=>rx_ds1620,
		temp_data1=>temp_data1,
		temp_data2=>temp_data2,
		temp_data3=>temp_data3,
		temp_data4=>temp_data4,
		done=>done);

tx_uart_wrapper_unit: entity work.uartLED(str_arch)
	generic map(DVSR_M=>DVSR_MU_19200)
	port map(clk=>clk, reset=>reset,
	tx_start=>start_tx,
	w_data=>data_tx,
	done_tick=>done_tx,
	tx=>tx_uart);

-- ********************************************************************************
send_uart1: process(clk, reset, state_tx1_reg)
begin
	if reset = '0' then
		state_tx1_reg <= xmit_idle;
		state_tx1_next <= xmit_idle;
		data_tx <= (others=>'0');
		time_delay_reg <= (others=>'0');
		time_delay_next <= (others=>'0');
		xmit_array <= (others=>(others=>'0'));
		xmit_array(0) <= X"FF";
		xmit_uindex <= "0000";
		pstart_tx <= '0';
		led1 <= "1111";
		skip <= '1';
		skip2 <= '1';
		xmit_stdlv <= (others=>'0');

	else if clk'event and clk = '1' then
		case state_tx1_reg is
			when xmit_idle =>
				pstart_tx <= '0';
				-- never goes faster than this delay
 				if time_delay_reg > TIME_DELAY5 then
					time_delay_next <= (others=>'0');
					state_tx1_next <= xmit_start;
				else
					time_delay_next <= time_delay_reg + 1;
				end if;	

			when xmit_start =>
				xmit_uindex <= "0001";
				led1 <= "1110";
				xmit_array(0) <= X"FF";
				xmit_array(1) <= temp_data1(7 downto 0);
				xmit_array(2) <= temp_data1(15 downto 8);
				xmit_array(3) <= temp_data2(7 downto 0);
				xmit_array(4) <= temp_data2(15 downto 8);
				xmit_array(5) <= temp_data3(7 downto 0);
				xmit_array(6) <= temp_data3(15 downto 8);
				xmit_array(7) <= temp_data4(7 downto 0);
				xmit_array(8) <= temp_data4(15 downto 8);
				-- xmit_array(0) <= X"FF";
				-- xmit_array(1) <= X"AA";
				-- xmit_array(2) <= X"55";
				-- xmit_array(3) <= X"FF";
				-- xmit_array(4) <= X"FF";
				-- xmit_array(5) <= X"22";
				-- xmit_array(6) <= X"FF";
				-- xmit_array(7) <= X"34";
				-- xmit_array(8) <= X"45";
				xmit_array(9) <= X"01";
				xmit_array(10) <= X"02";
				xmit_array(11) <= X"03";
				xmit_array(12) <= X"04";
				xmit_array(13) <= X"05";
				xmit_array(14) <= X"06";
				xmit_array(15) <= X"07";
				state_tx1_next <= xmit_check_FF;
--				state_tx1_next <= xmit_presend;

			when xmit_check_FF =>
				led1 <= "1101";
				xmit_stdlv <= xmit_array(conv_integer(xmit_uindex));
				state_tx1_next <= xmit_check_FF2;
				
			when xmit_check_FF2 =>
				led1 <= "1011";
				skip <= not skip;
				if skip = '1' then
					if xmit_stdlv = X"FF" then
						xmit_array(conv_integer(xmit_uindex)) <= X"FE";
					end if;
					if xmit_uindex < NO_ELEMS - 1 then
						xmit_uindex <= xmit_uindex + 1;
						state_tx1_next <= xmit_check_FF;
					else
--						xmit_uindex <= "00001";
						state_tx1_next <= xmit_presend;
					end if;
				end if;

			when xmit_presend =>
				led1 <= "1110";
				xmit_uindex <= "0000";
				state_tx1_next <= xmit_send;
				
			when xmit_send =>
				led1 <= "1101";
				data_tx <= xmit_array(conv_integer(xmit_uindex));
				pstart_tx <= '1';
				state_tx1_next <= xmit_wait;
					
			when xmit_wait =>
				led1 <= "1011";
				if done_tx = '1' then
					pstart_tx <= '0';
					state_tx1_next <= xmit_inc;
				end if;

			when xmit_inc =>	
				skip2 <= not skip2;
				if skip2 = '1' then
					if xmit_uindex < NO_ELEMS - 1 then
						xmit_uindex <= xmit_uindex + 1;
						state_tx1_next <= xmit_send;
					else
						state_tx1_next <= xmit_delay;
					end if;
				end if;
				
			when xmit_delay =>
				led1 <= "0111";
--				if time_delay_reg > TIME_DELAY9/1000 then
				if time_delay_reg > TIME_DELAY5 then
					time_delay_next <= (others=>'0');
					state_tx1_next <= xmit_start;
--					state_tx1_next <= xmit_presend;
				else
					time_delay_next <= time_delay_reg + 1;
				end if;	
		end case;
		time_delay_reg <= time_delay_next;
		state_tx1_reg <= state_tx1_next;
		end if;
	end if;
end process;

-- ********************************************************************************
tx_delay: process(clk, reset, state_txdly_reg)
variable temp_uart: integer range 0 to 255:= 33;
begin
	if reset = '0' then
		time_delay_reg2 <= (others=>'0');
		time_delay_next2 <= (others=>'0');
		state_txdly_reg <= idled;
		state_txdly_next <= idled;
		start_tx <= '0';

	else if clk'event and clk = '1' then
		case state_txdly_reg is
			when idled =>
				if pstart_tx = '1' then
					state_txdly_next <= startd;
				end if;

			when startd =>
 				if time_delay_reg2 > TIME_DELAY8c then
					time_delay_next2 <= (others=>'0');
					state_txdly_next <= procd1;
					start_tx <= '1';
				else
					time_delay_next2 <= time_delay_reg2 + 1;
				end if;	

			when procd1 =>
				start_tx <= '0';
				state_txdly_next <= idled;

		end case;
		time_delay_reg2 <= time_delay_next2;
		state_txdly_reg <= state_txdly_next;
		end if;
	end if;
end process;

end truck_arch;
