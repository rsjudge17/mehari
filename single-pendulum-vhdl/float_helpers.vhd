LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;
use ieee.MATH_REAL.all;

LIBRARY work;
use work.double_type.all;
use work.float_pkg_c_min.to_float;

package float_helpers is
  -- Those functions only work for "normal" real values and even for them
  -- they may produce different results due to round-off errors. Please
  -- use work.float_pkg_c_min.to_float !
  function double_to_std_logic_vector(x : real) return double;


  function to_float (arg : REAL) return double;
  function to_real  (arg : double) return REAL;

  type fixedpoint is array(2 downto -45) of std_logic;
  function to_fixedpoint(value : real) return fixedpoint;
end package;

package body float_helpers is

   function double_sign(x : real) return std_logic is
   begin
      if x >= 0.0 then
         return '0';
      else
         return '1';
      end if;
   end function;

   function double_exponent(x : real) return integer is
      variable e_min : integer;
      variable e_max : integer;
      variable e     : integer;
   begin
      e_min := -1022;
      e_max :=  1024;

      while e_min < e_max loop
        -- we add (...) mod 2 to make it round towards positive infinity instead of zero
         e := (e_min + e_max + (e_min + e_max) mod 2) / 2;
         if x / 2**real(e) >= 1.0 then
            e_min := e;
         else
            e_max := e-1;
         end if;

         --DEBUG
         report "double_exponent(" & real'image(x) & "): e_min=" & integer'image(e_min) & ", e_max=" & integer'image(e_max);
      end loop;

      report "double_exponent(" & real'image(x) & "): after loop: e_min=" & integer'image(e_min) & ", e_max=" & integer'image(e_max);
      assert e_min = e_max;

      return e_min;
   end function;

   -- uses code from http://www.eda.org/fphdl/float_pkg_c.vhdl
   function double_fraction(x : real; e : integer) return double_fraction_t is
      variable frac : real;
      variable fract : double_fraction_t;
   begin
      frac := abs(x / 2**real(e)) - 1.0;
      assert not (frac >= 0.0 and frac < 1.0)
        report "Exponent is wrong for double_fraction(x=" & real'image(x) &
          ", e=" & integer'image(e) & "): frac=" & real'image(frac);
      --return std_logic_vector(to_unsigned(natural(abs(round(x * 2**real(e + 52)))), 53));
      for i in 0 to 51 loop
         report "bit " & integer'image(i) & ": " & real'image(frac) & " >= " & real'image(2.0 ** (-1 - i)) & " -> " & boolean'image(frac >= 2.0 ** (-1 - i));
         if frac >= 2.0 ** (-1 - i) then
            fract (51 - i) := '1';
            frac := frac - 2.0 ** (-1 - i);
         else
            fract (51 - i) := '0';
         end if;
      end loop;
      return fract;
   end function;

   function double_to_std_logic_vector_mine(x : real) return double is
      variable e : integer;
      variable e_binary : std_logic_vector(10 downto 0);
   begin
      e := double_exponent(abs(x));

      --e_binary := std_logic_vector(to_signed(e, 11));
      --e_binary(e_binary'left) := not e_binary(e_binary'left);
      e_binary := std_logic_vector(to_unsigned(e+1023, 11));

      return double_sign(x) & e_binary & double_fraction(x, e);
   end function;

  function double_to_std_logic_vector(x : real) return double is
  begin
    return work.float_pkg_c_min.to_float(x);
  end function;

  function to_float (arg : REAL) return double is
  begin
    return work.float_pkg_c_min.to_float(arg);
  end function;

  function to_real  (arg : double) return REAL is
  begin
    return work.float_pkg_c_min.to_real(arg);
  end function;


  function two_complement(value : std_logic_vector) return std_logic_vector is
    variable tmp : std_logic_vector(natural range value'range);
  begin
    tmp := not(value);

    -- add one
    for i in tmp'low to tmp'high loop
      if tmp(i) = '1' then
        tmp(i) := '0';
      else
        tmp(i) := '1';
        exit;
      end if;
    end loop;

    return tmp;
  end function;

  function to_fixedpoint(value : real) return fixedpoint is
    variable tmp : real;
    variable fix : fixedpoint;
  begin
    assert value < 2.0**(fixedpoint'high-1+1)
      report "value to big for fixedpoint: " & real'image(value) & " >= " & real'image(2.0**(fixedpoint'high-1));

    --tmp := value * (2.0**(48.0-3.0));
    --return std_logic_vector(to_signed(integer(tmp), 48));

    tmp := abs(value);

    fix(fixedpoint'high) := '0';
    for i in fixedpoint'high-1 downto fixedpoint'low loop
      --report "tmp=" & real'image(tmp) & ", 2.0**i=" & real'image(2.0**i) & ", i=" & integer'image(i);
      if tmp >= 2.0**i then
        tmp := tmp - 2.0**i;
        fix(i) := '1';
      else
        fix(i) := '0';
      end if;
    end loop;

    if value < 0.0 then
      fix := fixedpoint(two_complement(std_logic_vector(fix)));
    end if;

    return fix;
  end function;
end package body float_helpers;
