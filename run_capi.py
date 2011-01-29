
import time
import ctest
from pb_types import *

msg = "Ch8KB0F0YXNoa2EaBUF0YXNoIg1IZXpyZXRrdWxpeWV2KhYKEmF0YW11cmFkQGNvbm5leC5pbxAC\nKhYKEmF0YW15cmF0QGdtYWlsLmNvbRABMgoKBjI3MDIwMxACMhIKDis5OTMgNjcgNjQyNjQyEAdS\nJAoJY29ubmV4LmlvEhBDVE8gJiBDby1Gb3VuZGVyKgUI2g8QAWIXdGVzdCBub3RlIGZyb20gc29t\nZSBndXk=\n".decode("base64")

start = time.time()
for i in range(5000):
    c = ctest.decode(MSG_Contact, msg)
print "Our pure c parser: ", time.time()-start

print "Parse done!"
print c["name"]["display_name"], " ", c["name"]["first"], " ", c["name"]["last"]

print "Emails: "
for e in c["email"]:
    print " * ", e["email"]
print "Phones: "
for p in c["phone"]:
    print " * ", p["display_number"]

print "Note: ", c["note"]

print c

