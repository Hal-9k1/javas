define cppskipstl
  set logging redirect on
  set logging file /dev/null
  set logging overwrite on
  set logging enabled on
  python

def get_sources():
  return [source.strip() for line in gdb.execute('info sources', to_string=True).splitlines()
    for source in line.split(',') if line.startswith('/')]
def skip_dir(sources, dir):
  for source in sources:
    if source.startswith(dir):
      gdb.execute('skip file ' + source)
if 'c++' in gdb.execute('show language', to_string=True):
  sources = get_sources()
  skip_dir(sources, '/usr')
  skip_dir(sources, '/mnt')

  end
  set logging enabled off
  set logging redirect off
end

define hookpost-run
  cppskipstl
end
define hookpost-start
  cppskipstl
end
# skip line
define skipl
  tbreak +1
  jump +1
end
