
import time
import ctest

msg = "CiAKBUF0YXNoGghBdGFtdXJhZCINSGV6cmV0a3VsaXlldioUChJhdGFteXJhdEBnbWFpbC5jb20q\nFAoSYXRhbXVyYWRAY29ubmV4LmlvKhQKEmF0YW11cmFkQGdtYWlsLmNvbTIOCgwrOTkzNjc2NDI2\nNDIyDgoMKzk5MzEyMjcwMjAzYh50aGlzIGlzIG5vdGUgZmllbGQgZm9yIGNvbnRhY3Q=\n".decode("base64")


start = time.time()
for i in range(5000):
    c = ctest.decode(32, msg)
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

