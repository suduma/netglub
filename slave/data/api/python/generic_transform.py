import re,sys,codecs

# This module contains 5 functions:
#  get_from_args() : read input from command line args, and returns (type, values, params).
#  get_from_stdin() : read an "Input" line from stdin, and returns (type, values, params).
#  write_results(type, values) : writes a "Result" line to stdout.
#  write_progress(val, str) : writes a "Progress" line to stdout of 'val' percent,
#                             and message 'str'.
#  write_error(str) : writes a "Error" line to stdout with message 'str'.

# communication with generic plugin is forced to utf8

sys.stdout = codecs.getwriter('utf8')(sys.stdout)

def get_from_args():
  args = sys.argv[1:]
  if len(args) < 1:
    return None
  e_type = unicode(args[0],'utf8')
  is_param = False
  e_values = {}
  params = {}
  i = 1
  while (i+1) < len(args):
    key = unicode(args[i],'utf8')
    val = unicode(args[i+1],'utf8')
    if len(key) == 0 and not is_param:
      is_param = True
      i+=1
      continue
    if is_param:
      params[key] = val
    else:
      e_values[key] = val
    i+=2
  return (e_type,e_values,params)

def get_from_stdin():
  line = unicode(raw_input(),'utf8')
  if not line.lower().startswith("input"):
    return None

  m = re.findall(r'"(.+?)(?<!\\)"', line)
  m = map(lambda x:x.replace(r'\"','"'),m)
  e_type = m[0]
  e_values = {}
  params = {}

  #find first hash
  mh = re.search(r'"(?:[^"]|\\")*?(?<!\\)"\s*:\s*"(?:.*?)(?<!\\)"(?:\s*,\s*"(?:.*?)(?<!\\)"\s*:\s*"(?:.*?)(?<!\\)")*',line)
  if mh is None:
    return (e_type,{},{})
  
  v_str = mh.group(0)
  # count items in it
  v_len = len(re.findall(r'(?<!\\)"', v_str)) / 2
  
  m = m[1:]
  for i in range(v_len/2):
    e_values[m[2*i]] = m[2*i+1]
  for i in range(v_len/2, len(m)/2):
    params[m[2*i]] = m[2*i+1]
  return (e_type,e_values,params)

def write_result(e_type, e_values):
  print 'Result "%s"' % _escape_s(e_type),
  keys = e_values.keys()
  if len(keys) == 0:
    return
  i = keys[0]
  print '"%s":"%s"' % (_escape_s(i), _escape_s(e_values[i])),
  for i in keys[1:]:
    print ',"%s":"%s"' % (_escape_s(i), _escape_s(e_values[i])),
  print

def write_progress(p_val, p_str):
  print 'Progress %f "%s"' % (p_val,_escape_s(p_str))

def write_error(e_str):
  print 'Error "%s"' % _escape_s(e_str)

def _escape_s(s):
  return s.replace('"',r'\"')
