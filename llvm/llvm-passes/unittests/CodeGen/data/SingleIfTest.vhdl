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
         reset : in std_logic;
         a_in_data : in  std_logic_vector(31 downto 0);
         a_in_valid : in std_logic;
         a_in_ready : out std_logic;
         b_in_data : in  std_logic_vector(31 downto 0);
         b_in_valid : in std_logic;
         b_in_ready : out std_logic;
         b_out_data : out  std_logic_vector(31 downto 0);
         b_out_valid : out std_logic;
         b_out_ready : in std_logic
   );
end entity;

architecture arch of test is
   component icmp_ne is
      port ( 
         aclk : in std_logic;
         a_data : in  std_logic_vector(31 downto 0);
         b_data : in  std_logic_vector(31 downto 0);
         result_data : out  std_logic_vector(0 downto 0);
         a_valid : in std_logic;
         b_valid : in std_logic;
         result_valid : out std_logic;
         a_ready : out std_logic;
         b_ready : out std_logic;
         result_ready : in std_logic
   );
   end component;

signal a_in_ready_1 : std_logic;
signal t0_data :  std_logic_vector(0 downto 0);
signal t0_valid : std_logic;
signal t0_ready : std_logic;
signal t0_data_1 :  std_logic_vector(0 downto 0);
signal t0_valid_1 : std_logic;
signal t1_data :  std_logic_vector(31 downto 0);
signal t1_valid : std_logic;
signal t1_ready : std_logic;
signal t2_data :  std_logic_vector(0 downto 0);
signal t2_valid : std_logic;
signal t2_ready : std_logic;
signal t3_data :  std_logic_vector(31 downto 0);
signal t3_valid : std_logic;
signal t3_ready : std_logic;
signal t4_data :  std_logic_vector(31 downto 0);
signal t4_valid : std_logic;
signal t4_ready : std_logic;
begin
   t0_data <= t0_data_1;
   t0_valid <= t0_valid_1;
   t0: icmp_ne
      port map ( a_data => a_in_data,
                 a_ready => a_in_ready_1,
                 a_valid => a_in_valid,
                 aclk => aclk,
                 b_data => std_logic_vector(to_unsigned(0, 32)),
                 b_valid => '1',
                 result_data => t0_data_1,
                 result_ready => t0_ready,
                 result_valid => t0_valid_1);
   t1_data <= std_logic_vector(to_unsigned(2, 32));
   t1_valid <= '1';
   remember_t2 : process(aclk)
   begin
      if reset = '1' then
         t2_valid <= '0';
         t2_data <= (others => '0');
      elsif rising_edge(aclk) and t0_valid = '1' then
         t2_valid <= t0_valid;
         t2_data <= t0_data;
      end if;
   end process;
   t3_valid <= '1';
   t3_data <= std_logic_vector(to_unsigned(2, 32));
   remember_t4 : process(aclk)
   begin
      if reset = '1' then
         t4_valid <= '0';
         t4_data <= (others => '0');
      elsif rising_edge(aclk) and b_in_valid = '1' then
         t4_valid <= b_in_valid;
         t4_data <= b_in_data;
      end if;
   end process;
   b_out_valid <= t3_valid WHEN t2_valid = '1' and t2_data(0) = '1' ELSE
      t4_valid WHEN t2_valid = '1' and t2_data(0) = '0' ELSE
      '0';
   b_out_data <= t3_data WHEN t2_valid = '1' and t2_data(0) = '1' ELSE
      t4_data WHEN t2_valid = '1' and t2_data(0) = '0' ELSE
      (others => 'X');
   a_in_ready <= a_in_ready_1;
   b_in_ready <= '1';
   t0_ready <= '1';
end architecture;
