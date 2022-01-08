#===----------------------------------------------------------------------===//
#
#                               Ranked
#
#===----------------------------------------------------------------------===//
#
#  server-test.py - 01/08/2022
#
#  Copyright (C) 2022. rollrat. All Rights Reserved.
#
#===----------------------------------------------------------------------===//

import requests
import time

host = '127.0.0.1'
port = 6372
url = 'http://' + host + ':' + str(port)

def post(data):
    res = requests.post(url, data=data)
    return str(res.content)

#datas = '''zadd aa 1 a'''

datas = '''
zadd aa 1 a
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrby table 1 mem
zincrbyp table 1 mem 1
zincrbyp table 1 mem 1
zincrbyp table 1 mem 1
zincrbyp table 1 mem 1
zget table mem
'''

print(post(datas))

time.sleep(3)

expect = '''
zget table mem
'''

print(post(expect))