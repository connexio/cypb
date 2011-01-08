
import time
import connexiopb
from connexio_pb2 import *

msg = "CiAKBUF0YXNoGghBdGFtdXJhZCINSGV6cmV0a3VsaXlldioUChJhdGFteXJhdEBnbWFpbC5jb20q\nFAoSYXRhbXVyYWRAY29ubmV4LmlvKhQKEmF0YW11cmFkQGdtYWlsLmNvbTIOCgwrOTkzNjc2NDI2\nNDIyDgoMKzk5MzEyMjcwMjAzYh50aGlzIGlzIG5vdGUgZmllbGQgZm9yIGNvbnRhY3Q=\n".decode("base64")

start = time.time()
for i in range(5000):
    c = Contact()
    c.ParseFromString(msg)
end = time.time()

print end-start
print c

