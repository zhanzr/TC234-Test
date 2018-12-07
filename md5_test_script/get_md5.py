import os
import hashlib

test_file = 'G:/vs_prj/test_text.txt'

f=open(test_file, mode='rb')
buf = f.read()
f.close()

hm5 = hashlib.md5()
hm5.update(buf)
print(hm5.hexdigest())

