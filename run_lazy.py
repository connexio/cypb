
import time

import ctest
from pb_types import *


msg = "CiAKBUF0YXNoGghBdGFtdXJhZCINSGV6cmV0a3VsaXlldioUChJhdGFteXJhdEBnbWFpbC5jb20q\nFAoSYXRhbXVyYWRAY29ubmV4LmlvKhQKEmF0YW11cmFkQGdtYWlsLmNvbTIOCgwrOTkzNjc2NDI2\nNDIyDgoMKzk5MzEyMjcwMjAzYh50aGlzIGlzIG5vdGUgZmllbGQgZm9yIGNvbnRhY3Q=\n".decode("base64")

msg = "Ch8KB0F0YXNoa2EaBUF0YXNoIg1IZXpyZXRrdWxpeWV2KhYKEmF0YW11cmFkQGNvbm5leC5pbxAC\nKhYKEmF0YW15cmF0QGdtYWlsLmNvbRABMgoKBjI3MDIwMxACMhIKDis5OTMgNjcgNjQyNjQyEAdS\nJAoJY29ubmV4LmlvEhBDVE8gJiBDby1Gb3VuZGVyKgUI2g8QAWIXdGVzdCBub3RlIGZyb20gc29t\nZSBndXk=\n".decode("base64")

start = time.time()
for i in range(5000):
    a = ctest.PBMsg(MSG_Contact, msg)
print "Our lazy c parser: ", time.time()-start


print a.name.display_name, " ", a.name.first, " ", a.name.last

print "Emails (first access):"
print a.email

print "Emails: "
for e in a.email:
    print " * ", e.email, e._type

print "Phones: "
for p in a.phone:
    print " * ", p.display_number, p._type

print "Note:"
print a.note

print "Job: "
for j in a.job:
    print j.company, " - ", j.position, "  [", j.startdate.year, "/",  j.startdate.month, "]"
