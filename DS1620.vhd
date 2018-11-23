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

entity DS1620 is
	port(
		clk, reset: in std_logic;
		start: in std_logic;
		ds_rx: in std_logic;
		raw_data: out std_logic_vector(15 downto 0);
		done: out std_logic);
end DS1620;

architecture DS1620_arch of DS1620 is

	type state_uart2 is (idle2, start2, next2, done2);
	signal state_uart_reg2, state_uart_next2: state_uart2;

	signal data_rx: std_logic_vector(7 downto 0);
	signal done_rx: std_logic;
	signal rx_uart: std_logic;
	signal start_rx: std_logic;
begin

rx_uart_wrapper_unit: entity work.uartLED2(str_arch)
	generic map(DVSR_M=>DVSR_MU_19200)
	port map(clk=>clk, reset=>reset,
	rx_start=>start_rx,
	r_data=>data_rx,
	done_tick=>done_rx,
	rx=>ds_rx);

-- ********************************************************************************
recv_uart: process(clk, reset, state_uart_reg2)
begin
	if reset = '0' then
		state_uart_reg2 <= idle2;
		state_uart_next2 <= idle2;
		start_rx <= '0';
		done <= '0';

	else if clk'event and clk = '1' then
		case state_uart_reg2 is
			when idle2 =>
				start_rx <= '0';
				done <= '0';
				if start = '1' then
					start_rx <= '1';
					state_uart_next2 <= start2;
				end if;
			
			when start2 =>
				start_rx <= '0';
				if done_rx = '1' then
					raw_data(7 downto 0) <= data_rx;
					state_uart_next2 <= next2;
				end if;	

			when next2 =>
				start_rx <= '1';
				state_uart_next2 <= done2;
			
			when done2 =>
				start_rx <= '0';
				if done_rx = '1' then
					raw_data(15 downto 8) <= data_rx;
					done <= '1';
					state_uart_next2 <= idle2;
				end if;

		end case;
		state_uart_reg2 <= state_uart_next2;
		end if;
	end if;
end process;

end DS1620_arch;