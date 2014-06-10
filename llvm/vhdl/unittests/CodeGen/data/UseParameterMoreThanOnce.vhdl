-- generated by Mehari
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.math_real.ALL;
use ieee.numeric_std.all;

library work;
use work.float_helpers.all;
use work.test_helpers.all;

entity test is
   port ( 
         aclk : in std_logic;
         a_in_data : in  std_logic_vector(31 downto 0);
         a_in_valid : in std_logic;
         a_in_ready : out std_logic;
         a_out_data : out  std_logic_vector(31 downto 0);
         a_out_valid : out std_logic;
         a_out_ready : in std_logic
   );
end entity;

architecture arch of test is
   component add is
      port ( 
         aclk : in std_logic;
         a_data : in  std_logic_vector(31 downto 0);
         b_data : in  std_logic_vector(31 downto 0);
         result_data : out  std_logic_vector(31 downto 0);
         a_valid : in std_logic;
         b_valid : in std_logic;
         result_valid : out std_logic;
         a_ready : out std_logic;
         b_ready : out std_logic;
         result_ready : in std_logic
   );
   end component;

signal a_in_ready_1 : std_logic;
signal a_in_ready_2 : std_logic;
signal t0_data :  std_logic_vector(31 downto 0);
signal t0_valid : std_logic;
signal t0_ready : std_logic;
signal t0_data_1 :  std_logic_vector(31 downto 0);
signal t0_valid_1 : std_logic;
begin
   a_in_ready <= a_in_ready_1;
   a_in_ready <= a_in_ready_2;
   t0_data <= t0_data_1;
   t0_valid <= t0_valid_1;
   t0: add
      port map ( a_data => a_in_data,
                 a_ready => a_in_ready_1,
                 a_valid => a_in_valid,
                 aclk => aclk,
                 b_data => a_in_data,
                 b_ready => a_in_ready_2,
                 b_valid => a_in_valid,
                 result_data => t0_data_1,
                 result_ready => t0_ready,
                 result_valid => t0_valid_1);
   a_out_data <= t0_data;
   a_out_valid <= t0_valid;
   t0_ready <= a_out_ready;
end architecture;

