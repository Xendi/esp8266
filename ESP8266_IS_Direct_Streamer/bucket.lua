-- Data Bucket Creator for initialstate.com API
-- David Reeves 2015 (MIT licence)
bucketKey = "YOUR_BUCKET_KEY_HERE"
bucketName = "YOUR_BUCKET_NAME_HERE"
streamKey = "YOUR_IS_STREAM_KEY_HERE"

conn=net.createConnection(net.TCP, 0)
conn:connect(80,"insecure-groker.initialstate.com")
conn:on("receive", function(conn, incoming) print(incoming) conn:close() a=node.heap() print(a) end )

a=node.heap() print (a)

local payload = '{\"bucketName\":\"'..bucketName..'\",\"bucketKey\":\"'..bucketKey..'\"}'

local headers = {}
headers["Host"] = "insecure-groker.initialstate.com"
headers["User-Agent"] = "nodemcu"
headers["Accept-Version"] = "~0"
headers["X-IS-AccessKey"] = streamKey
headers["Content-Type"] = "application/json"
headers["Content-Length"] = string.len(payload)

local toSend = "POST /api/buckets HTTP/1.1\r\n"
for k,v in pairs(headers) do
  toSend = toSend..k..": "..v.."\r\n"
  end
toSend = toSend.."\r\n"..payload.."\r\n"
print(toSend)

tmr.stop(3)
tmr.alarm( 3 , 5000 , 0 , function() conn:send(toSend) end)
