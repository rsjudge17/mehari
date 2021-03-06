--------------------------------------------------------------------------------
--                          ParameterAssignmentTest
-- This wrapper has been generated by PivPav (using FloPoCo)
-- and is distributed under the terms of the GNU Lesser General Public Licence.
-- Authors: 
--------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use ieee.numeric_std.all;

entity icmp_sgt is
  port (
    aclk : in std_logic;
    a_data : in std_logic_vector(31 downto 0);
    a_valid : in std_logic;
    a_ready : out std_logic;
    b_data : in std_logic_vector(31 downto 0);
    b_valid : in std_logic;
    b_ready : out std_logic;
    result_data : out std_logic_vector(0 downto 0);
    result_valid : out std_logic;
    result_ready : in std_logic
  );
end entity;

architecture behavioural of icmp_sgt is
begin
  icmp_sgt : process(aclk)
    variable result : boolean;
  begin
    if rising_edge(aclk) then
      if a_valid = '1' and b_valid = '1' then
        result := to_integer(signed(a_data)) > to_integer(signed(b_data));
        if result then
          result_data <= "1";
        else
          result_data <= "0";
        end if;
        result_valid <= '1';
      else
        result_data <= (others => '0');
        result_valid <= '0';
      end if;
    end if;
  end process;

  a_ready <= result_ready;
  b_ready <= result_ready;
end architecture;
