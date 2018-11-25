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

entity poll_DS1620 is
	port(
		clk, reset: in std_logic;
		rx_ds1620: in std_logic;
		temp_data1: out std_logic_vector(15 downto 0);
		temp_data2: out std_logic_vector(15 downto 0);
		temp_data3: out std_logic_vector(15 downto 0);
		temp_data4: out std_logic_vector(15 downto 0);
		done: out std_logic
		);
end poll_DS1620;

architecture truck_arch of poll_DS1620 is

	-- send_uart1
	type state_uart1 is (idle1, start1, proc1, proc2);
	signal state_tx1_reg, state_tx1_next: state_uart1;

	signal ds1620_done: std_logic;
	signal pstart_tx: std_logic;
	signal high_byte, low_byte: std_logic_vector(7 downto 0);
	signal raw_data1: std_logic_vector(15 downto 0);
	signal start_ds1620: std_logic;
	signal index1: std_logic_vector(1 downto 0);
	
begin

ds1620_unit: entity work.ds1620
	port map(clk=>clk,reset=>reset,
		start=>start_ds1620,
		ds_rx=>rx_ds1620,
		index=>index1,
		raw_data=>raw_data1,
		done=>ds1620_done);

-- ********************************************************************************
send_uart1: process(clk, reset, state_tx1_reg)
begin
	if reset = '0' then
		state_tx1_reg <= idle1;
		state_tx1_next <= idle1;
--		skip <= '0';
		low_byte <= X"00";
		high_byte <= X"00";
		start_ds1620 <= '0';
		pstart_tx <= '0';
		temp_data1 <= (others=>'0');
		temp_data2 <= (others=>'0');
		temp_data3 <= (others=>'0');
		temp_data4 <= (others=>'0');
		done <= '0';

	else if clk'event and clk = '1' then
		case state_tx1_reg is
			when idle1 =>
				done <= '0';
				start_ds1620 <= '1';
				state_tx1_next <= start1;

			when start1 =>
				start_ds1620 <= '0';
				if ds1620_done = '1' then
					state_tx1_next <= proc1;
				end if;

			when proc1 =>
				state_tx1_next <= proc2;

			when proc2 =>
				case index1 is
					when "00" => temp_data1 <= raw_data1;
					when "01" => temp_data2 <= raw_data1;
					when "10" => temp_data3 <= raw_data1;
					when "11" => temp_data4 <= raw_data1;
					when others =>
				end case;
				done <= '1';
				state_tx1_next <= idle1;

		end case;
		state_tx1_reg <= state_tx1_next;
		end if;
	end if;
end process;

end truck_arch;
