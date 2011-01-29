
import time
import ctest


msg = "CiAKBUF0YXNoGghBdGFtdXJhZCINSGV6cmV0a3VsaXlldioUChJhdGFteXJhdEBnbWFpbC5jb20q\nFAoSYXRhbXVyYWRAY29ubmV4LmlvKhQKEmF0YW11cmFkQGdtYWlsLmNvbTIOCgwrOTkzNjc2NDI2\nNDIyDgoMKzk5MzEyMjcwMjAzYh50aGlzIGlzIG5vdGUgZmllbGQgZm9yIGNvbnRhY3Q=\n".decode("base64")

start = time.time()
for i in range(5000):
    a = ctest.PBMsg(1, msg)
print "Our lazy c parser: ", time.time()-start


print a.name.display_name, " ", a.name.first, " ", a.name.last

print "Emails (first access):"
print a.emails

print "Emails: "
for e in a.emails:
    print " * ", e.email

print "Phones: "
for p in a.phones:
    print " * ", p.phone

print "Note:"
print a.note

