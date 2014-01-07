----------------------------------------------------------------------------------
-- autogenerated from {{source_files}}
----------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use ieee.math_real.all;

library work;
use work.double_type.all;
use work.float_helpers.all;
use work.test_helpers.all;


entity test_{{uut_name}} is
end test_{{uut_name}};

architecture behavior of test_{{uut_name}} is 

  -- Component Declaration for the Unit Under Test (UUT)

  COMPONENT {{uut_name}}
  PORT(
       aclk : IN  std_logic;
       {{input_a_name}}_tdata  : IN  double;
       {{input_a_name}}_tvalid : IN  std_logic;
       {{input_a_name}}_tready : OUT std_logic;
       {{input_b_name}}_tdata  : IN  double;
       {{input_b_name}}_tvalid : IN  std_logic;
       {{input_b_name}}_tready : OUT std_logic;
       {{result_name}}_tdata  : OUT double;
       {{result_name}}_tvalid : OUT std_logic;
       {{result_name}}_tready : IN  std_logic
      );
  END COMPONENT;

  -- Clock
  signal aclk : std_logic := '0';
  constant aclk_period : time := 10 ns;

  -- Inputs
  signal input_a_tdata  : double    := (others => '0');
  signal input_a_tvalid : std_logic := '0';
  signal input_b_tdata  : double    := (others => '0');
  signal input_b_tvalid : std_logic := '0';
  signal result_tready: std_logic := '0';

  -- Outputs
  signal input_a_tready  : std_logic;
  signal input_b_tready  : std_logic;
  signal result_tdata  : double;
  signal result_tvalid : std_logic;

BEGIN
  -- Instantiate the Unit Under Test (UUT)
  uut: {{uut_name}} PORT MAP (
    aclk => aclk,
    {{input_a_name}}_tdata  => input_a_tdata,
    {{input_a_name}}_tvalid => input_a_tvalid,
    {{input_a_name}}_tready => input_a_tready,
    {{input_b_name}}_tdata  => input_b_tdata,
    {{input_b_name}}_tvalid => input_b_tvalid,
    {{input_b_name}}_tready => input_b_tready,
    {{result_name}}_tdata   => result_tdata,
    {{result_name}}_tvalid  => result_tvalid,
    {{result_name}}_tready  => result_tready
  );

  -- Clock process definitions
  aclk_process : process
  begin
    aclk <= '0';
    wait for aclk_period/2;
    aclk <= '1';
    wait for aclk_period/2;
  end process;

  stimulus_process : process
    constant max_clock_cycles : natural := {{max_clock_cycles}};

    procedure test(input_a_value : in real;
                   input_b_value : in real;
                   epsilon       : in real := 1.0e-10) is
    begin
      wait until input_a_tready = '1' and input_b_tready = '1' and rising_edge(aclk) for 10*aclk_period;
      assert input_a_tready = '1' and input_b_tready = '1'
        report "uut is not ready for data: input_tready = "
          & std_logic'image(input_a_tready) & " and " & std_logic'image(input_b_tready);

      wait until falling_edge(aclk);
      report "testing with " & real'image(input_a_value) & " and " & real'image(input_b_value);
      input_a_tdata <= to_float(input_a_value);
      input_a_tvalid <= '1';
      input_b_tdata <= to_float(input_b_value);
      input_b_tvalid <= '1';

      if max_clock_cycles > 0 then
        wait for aclk_period;

        input_a_tvalid <= '0';
        input_a_tdata <= (others => '0');
        input_b_tvalid <= '0';
        input_b_tdata <= (others => '0');

        wait until result_tvalid = '1' and rising_edge(aclk)
          for max_clock_cycles*aclk_period - aclk_period/2;
      else
        wait for aclk_period/2;
      end if;
      assert result_tvalid = '1'
        report "result was not ready in time: result_tvalid = " & std_logic'image(result_tvalid);

      if result_tvalid = '1' then
        assertAlmostEqual(to_real(result_tdata), {{expected_value_calculation}}, epsilon);
      end if;
    end procedure;
  begin
    wait for 100 ns;

    result_tready <= '1';

    wait for aclk_period*10;

{{#TESTS}}
    {{TEST:x-fix-indent=    }}
{{/TESTS}}

    -- The user might want to see what happens after the tests, so we make
    -- sure that the waveform file contains a few more samples.
    wait for 1 ns;

    endOfSimulation(0);

    wait;
  end process;

  debug_timing_process : process(aclk)
    variable input_a_was_valid : std_logic := '0';
    variable input_b_was_valid : std_logic := '0';
    variable output_was_valid  : std_logic := '0';
  begin
    if rising_edge(aclk) then
      if input_a_tvalid = '1' and input_a_was_valid = '0' then
        report "data on input a is valid";
      end if;

      if input_b_tvalid = '1' and input_b_was_valid = '0' then
        report "data on input b is valid";
      end if;

      if result_tvalid = '1' and output_was_valid = '0' then
        report "output data is valid";
      end if;

      input_a_was_valid := input_a_tvalid;
      input_b_was_valid := input_b_tvalid;
      output_was_valid  := result_tvalid;
    end if;
  end process;
end;
