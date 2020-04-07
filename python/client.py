import httplib, subprocess

c = httplib.HTTPConnection('localhost', 8080)
c.request('POST', '/return', '{}')
doc = c.getresponse().read()
print doc
