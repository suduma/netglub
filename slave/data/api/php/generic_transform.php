<?php

# This module contains 4 functions:
#  get_from_args() : read input from command line args, and returns (type, values, params).
#  write_results(type, values) : writes a "Result" line to stdout.
#  write_progress(val, str) : writes a "Progress" line to stdout of 'val' percent,
#                             and message 'str'.
#  write_error(str) : writes a "Error" line to stdout with message 'str'.

function get_from_args()
{
  global $argv;
  if( sizeof($argv) < 2)
    return NULL;
  $e_type = $argv[1];
  $e_values = array();
  $params = array();
  $is_param = false;
  $i = 2;
  while(($i+1) < sizeof($argv))
  {
    $key = $argv[$i];
    $val = $argv[$i+1];
    if(strlen($key) == 0 && ! $is_param)
    {
      $i++;
      $is_param = true;
      continue;
    }
    if($is_param)
      $params[$key] = $val;
    else
      $e_values[$key] = $val;
    $i+=2;
  }
  
  return array($e_type,$e_values,$params);
}

function write_result($e_type, $e_values)
{
  printf('Result "%s" ', _escape_s($e_type));
  $keys = array_keys($e_values);
  if(sizeof($keys) == 0)
    return;
  for($i = 0; $i < sizeof($keys); $i++)
  {
    if($i == 0)
      printf('"%s":"%s"', _escape_s($keys[0]), _escape_s($e_values[$keys[0]]));
    else
      printf(',"%s":"%s"', _escape_s($keys[$i]), _escape_s($e_values[$keys[$i]]));
  }
  printf("\n");
}

function write_progress($p_val, $p_str)
{
  printf("Progress %f \"%s\"\n", $p_val, _escape_s($p_str));
}

function write_error($e_str)
{
  printf("Error \"%s\"\n", _escape_s($e_str));
}

function _escape_s($s)
{
  return addcslashes($s,"\n\r\"");
}


?>
