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
		rx_ds1620: in std_logic;
		raw_data: out std_logic_vector(15 downto 0);
		led1: out std_logic_vector(3 downto 0)
		);
end test_DS1620;

architecture truck_arch of test_DS1620 is

	-- send_uart1
	type state_uart1 is (idle1, start1, proc1, proc2, proc3);
	signal state_tx1_reg, state_tx1_next: state_uart1;

	signal time_delay_reg, time_delay_next: unsigned(24 downto 0);		-- send_uart1
	signal time_delay_reg2, time_delay_next2: unsigned(23 downto 0);	-- calc_proc

	signal ds1620_done: std_logic;
	signal start_tx: std_logic;
	signal data_tx: std_logic_vector(7 downto 0);
	signal high_byte, low_byte: std_logic_vector(7 downto 0);
	signal done_tx: std_logic;
	signal raw_data1: std_logic_vector(15 downto 0);
	signal start_ds1620: std_logic;
	
begin

ds1620_unit: entity work.ds1620
	port map(clk=>clk,reset=>reset,
		start=>start_ds1620,
		ds_rx=>rx_ds1620,
		raw_data=>raw_data1,
		done=>ds1620_done);

tx_uart_wrapper_unit: entity work.uartLED(str_arch)
	generic map(DVSR_M=>DVSR_MU_19200)
	port map(clk=>clk, reset=>reset,
	tx_start=>start_tx,
	w_data=>data_tx,
	done_tick=>done_tx,
	tx=>tx_uart);

-- ********************************************************************************
send_uart1: process(clk, reset, state_tx1_reg)
variable temp_uart: integer range 0 to 255:= 33;
begin
	if reset = '0' then
		state_tx1_reg <= idle1;
		data_tx <= (others=>'0');
		start_tx <= '0';
--		skip <= '0';
		time_delay_reg <= (others=>'0');
		time_delay_next <= (others=>'0');
		low_byte <= X"00";
		high_byte <= X"00";
		start_ds1620 <= '0';
		led1 <= "1111";

	else if clk'event and clk = '1' then
		case state_tx1_reg is
			when idle1 =>
				start_tx <= '0';
				start_ds1620 <= '1';
				state_tx1_next <= start1;

			when start1 =>
				start_ds1620 <= '0';
				start_tx <= '0';
				if ds1620_done = '1' then
					state_tx1_next <= proc1;
					led1 <= "1110";
				end if;

			when proc1 =>
				data_tx <= raw_data1(7 downto 0);
				start_tx <= '1';
				state_tx1_next <= proc2;

			when proc2 =>
				start_tx <= '0';
				if done_tx = '1' then
					led1 <= "0111";
					data_tx <= raw_data1(15 downto 8);
					state_tx1_next <= proc3;
				end if;	

			when proc3 =>
				start_tx <= '1';
				state_tx1_next <= idle1;
				
--			when delay1 =>
 				-- if time_delay_reg > TIME_DELAY9 then
					-- time_delay_next <= (others=>'0');
					-- start_tx <= '0';
					-- state_tx1_next <= idle1;
				-- else
					-- time_delay_next <= time_delay_reg + 1;
				-- end if;	
		end case;
		time_delay_reg <= time_delay_next;
		state_tx1_reg <= state_tx1_next;
		end if;
	end if;
end process;

end truck_arch;
