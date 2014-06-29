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
         a_in_data : in  std_logic_vector(63 downto 0);
         a_in_valid : in std_logic;
         a_in_ready : out std_logic;
         return_data : out  std_logic_vector(63 downto 0);
         return_valid : out std_logic;
         return_ready : in std_logic
   );
end entity;

architecture arch of test is
   component float_cmp is
      port ( 
         aclk : in std_logic;
         s_axis_a_tdata : in  std_logic_vector(63 downto 0);
         s_axis_b_tdata : in  std_logic_vector(63 downto 0);
         m_axis_result_tdata : out  std_logic_vector(7 downto 0);
         s_axis_a_tvalid : in std_logic;
         s_axis_b_tvalid : in std_logic;
         m_axis_result_tvalid : out std_logic;
         s_axis_a_tready : out std_logic;
         s_axis_b_tready : out std_logic;
         m_axis_result_tready : in std_logic;
         s_axis_operation_tdata : in  std_logic_vector(7 downto 0);
         s_axis_operation_tvalid : in std_logic;
         s_axis_operation_tready : out std_logic
   );
   end component;

signal a_in_ready_1 : std_logic;
signal t1_data :  std_logic_vector(7 downto 0);
signal t1_valid : std_logic;
signal t1_ready : std_logic;
signal t1_data_1 :  std_logic_vector(7 downto 0);
signal t1_valid_1 : std_logic;
signal t0_data :  std_logic_vector(0 downto 0);
signal t0_valid : std_logic;
signal t0_ready : std_logic;
signal a_in_ready_2 : std_logic;
signal t4_data :  std_logic_vector(7 downto 0);
signal t4_valid : std_logic;
signal t4_ready : std_logic;
signal t4_data_1 :  std_logic_vector(7 downto 0);
signal t4_valid_1 : std_logic;
signal t3_data :  std_logic_vector(0 downto 0);
signal t3_valid : std_logic;
signal t3_ready : std_logic;
signal t7_data :  std_logic_vector(0 downto 0);
signal t7_valid : std_logic;
signal t7_ready : std_logic;
signal t8_data :  std_logic_vector(63 downto 0);
signal t8_valid : std_logic;
signal t8_ready : std_logic;
signal t9_data :  std_logic_vector(63 downto 0);
signal t9_valid : std_logic;
signal t9_ready : std_logic;
signal t6_data :  std_logic_vector(63 downto 0);
signal t6_valid : std_logic;
signal t6_ready : std_logic;
signal t10_data :  std_logic_vector(0 downto 0);
signal t10_valid : std_logic;
signal t10_ready : std_logic;
signal t11_data :  std_logic_vector(63 downto 0);
signal t11_valid : std_logic;
signal t11_ready : std_logic;
signal t5_data :  std_logic_vector(63 downto 0);
signal t5_valid : std_logic;
signal t5_ready : std_logic;
signal t12_data :  std_logic_vector(63 downto 0);
signal t12_valid : std_logic;
signal t12_ready : std_logic;
signal t13_data :  std_logic_vector(0 downto 0);
signal t13_valid : std_logic;
signal t13_ready : std_logic;
signal t14_data :  std_logic_vector(63 downto 0);
signal t14_valid : std_logic;
signal t14_ready : std_logic;
signal t15_data :  std_logic_vector(63 downto 0);
signal t15_valid : std_logic;
signal t15_ready : std_logic;
signal t2_data :  std_logic_vector(63 downto 0);
signal t2_valid : std_logic;
signal t2_ready : std_logic;
begin
   t1_data <= t1_data_1;
   t1_valid <= t1_valid_1;
   t0_data(0) <= t1_data(0);
   t0_valid <= t1_valid;
   t0: float_cmp
      port map ( aclk => aclk,
                 m_axis_result_tdata => t1_data_1,
                 m_axis_result_tready => t1_ready,
                 m_axis_result_tvalid => t1_valid_1,
                 s_axis_a_tdata => a_in_data,
                 s_axis_a_tready => a_in_ready_1,
                 s_axis_a_tvalid => a_in_valid,
                 s_axis_b_tdata => to_float(0.000000),
                 s_axis_b_tvalid => '1',
                 s_axis_operation_tdata => "00110100",
                 s_axis_operation_tvalid => '1');
   t4_data <= t4_data_1;
   t4_valid <= t4_valid_1;
   t3_data(0) <= t4_data(0);
   t3_valid <= t4_valid;
   t3: float_cmp
      port map ( aclk => aclk,
                 m_axis_result_tdata => t4_data_1,
                 m_axis_result_tready => t4_ready,
                 m_axis_result_tvalid => t4_valid_1,
                 s_axis_a_tdata => a_in_data,
                 s_axis_a_tready => a_in_ready_2,
                 s_axis_a_tvalid => a_in_valid,
                 s_axis_b_tdata => to_float(0.000000),
                 s_axis_b_tvalid => '1',
                 s_axis_operation_tdata => "00010100",
                 s_axis_operation_tvalid => '1');
   remember_t7 : process(aclk)
   begin
      if reset = '1' then
         t7_valid <= '0';
         t7_data <= (others => '0');
      elsif rising_edge(aclk) and t3_valid = '1' then
         t7_valid <= t3_valid;
         t7_data <= t3_data;
      end if;
   end process;
   t8_valid <= '1';
   t8_data <= to_float(0.000000);
   t9_valid <= '1';
   t9_data <= to_float(-1.000000);
   t6_valid <= t8_valid WHEN t7_valid = '1' and t7_data(0) = '1' ELSE
      t9_valid WHEN t7_valid = '1' and t7_data(0) = '0' ELSE
      '0';
   t6_data <= t8_data WHEN t7_valid = '1' and t7_data(0) = '1' ELSE
      t9_data WHEN t7_valid = '1' and t7_data(0) = '0' ELSE
      (others => 'X');
   remember_t10 : process(aclk)
   begin
      if reset = '1' then
         t10_valid <= '0';
         t10_data <= (others => '0');
      elsif rising_edge(aclk) and t0_valid = '1' then
         t10_valid <= t0_valid;
         t10_data <= t0_data;
      end if;
   end process;
   remember_t11 : process(aclk)
   begin
      if reset = '1' then
         t11_valid <= '0';
         t11_data <= (others => '0');
      elsif rising_edge(aclk) and t5_valid = '1' then
         t11_valid <= t5_valid;
         t11_data <= t5_data;
      end if;
   end process;
   remember_t12 : process(aclk)
   begin
      if reset = '1' then
         t12_valid <= '0';
         t12_data <= (others => '0');
      elsif rising_edge(aclk) and t6_valid = '1' then
         t12_valid <= t6_valid;
         t12_data <= t6_data;
      end if;
   end process;
   t5_valid <= t11_valid WHEN t10_valid = '1' and t10_data(0) = '1' ELSE
      t12_valid WHEN t10_valid = '1' and t10_data(0) = '0' ELSE
      '0';
   t5_data <= t11_data WHEN t10_valid = '1' and t10_data(0) = '1' ELSE
      t12_data WHEN t10_valid = '1' and t10_data(0) = '0' ELSE
      (others => 'X');
   remember_t13 : process(aclk)
   begin
      if reset = '1' then
         t13_valid <= '0';
         t13_data <= (others => '0');
      elsif rising_edge(aclk) and t0_valid = '1' then
         t13_valid <= t0_valid;
         t13_data <= t0_data;
      end if;
   end process;
   t14_valid <= '1';
   t14_data <= to_float(1.000000);
   remember_t15 : process(aclk)
   begin
      if reset = '1' then
         t15_valid <= '0';
         t15_data <= (others => '0');
      elsif rising_edge(aclk) and t6_valid = '1' then
         t15_valid <= t6_valid;
         t15_data <= t6_data;
      end if;
   end process;
   t2_valid <= t14_valid WHEN t13_valid = '1' and t13_data(0) = '1' ELSE
      t15_valid WHEN t13_valid = '1' and t13_data(0) = '0' ELSE
      '0';
   t2_data <= t14_data WHEN t13_valid = '1' and t13_data(0) = '1' ELSE
      t15_data WHEN t13_valid = '1' and t13_data(0) = '0' ELSE
      (others => 'X');
   return_data <= t2_data;
   return_valid <= t2_valid;
   a_in_ready <= a_in_ready_1 and a_in_ready_2;
   t0_ready <= '1';
   t1_ready <= t0_ready;
   t2_ready <= return_ready;
   t3_ready <= '1';
   t4_ready <= t3_ready;
   t5_ready <= '1';
   t6_ready <= '1';
end architecture;

