-- ESP8266 data streamer for initialstate.com
-- David Reeves 2015 (CC-SA-NC) 
bucketKey = "esp8266_17"
streamKey = "dBV3uHArQF92BneXRlEykNlTZS279PVQ"

pin = 4
ow.setup(pin)

conn=net.createConnection(net.TCP, 0)
conn:connect(80,"insecure-groker.initialstate.com")
conn:on("receive", function(conn, incoming) print(incoming) end )

function postData(key, value)

  local stream = {}
  stream["key"] = key
  stream["value"] = value
  local payload = cjson.encode(stream)

  local headers = {}
  headers["Host"] = "insecure-groker.initialstate.com"
  headers["User-Agent"] = "nodemcu"
  headers["Accept-Version"] = "~0"
  headers["X-IS-AccessKey"] = streamKey
  headers["X-IS-BucketKey"] = bucketKey
  headers["Content-Type"] = "application/json"
  headers["Content-Length"] = string.len(payload)

  local toSend = "POST /api/events HTTP/1.1\r\n"
  for k,v in pairs(headers) do
    toSend = toSend..k..": "..v.."\r\n"
    end
  toSend = toSend.."\r\n"..payload.."\r\n"
  print(toSend)
  conn:send(toSend)
end

function genData()
  signalKey = "tempC" 
  signalValue = readTemp() 
  postData(signalKey, signalValue) 
  a=node.heap()
  print(a)
end  

function readTemp()
  count=0
  repeat
    count = count + 1
    addr = ow.reset_search(pin)
    addr = ow.search(pin)
    tmr.wdclr()
  until((addr ~= nil) or (count > 100))
  if (addr == nil) then
    print("No device")
  else
    print(addr:byte(1,8))
    crc = ow.crc8(string.sub(addr,1,7))
    if (crc == addr:byte(8)) then
      if ((addr:byte(1) == 0x10) or (addr:byte(1) == 0x28)) then
          ow.reset(pin)
          ow.select(pin, addr)
          ow.write(pin, 0x44, 1)
          tmr.delay(1000000)
          present = ow.reset(pin)
          ow.select(pin, addr)
          ow.write(pin,0xBE,1)
          print("P="..present)  
          data = nil
          data = string.char(ow.read(pin))
          for i = 1, 8 do
            data = data .. string.char(ow.read(pin))
          end
          print(data:byte(1,9))
          crc = ow.crc8(string.sub(data,1,8))
          if (crc == data:byte(9)) then
             t = (data:byte(1) + data:byte(2) * 256) * 625
             t1 = t / 10000
             t2 = t % 10000
             temp = t1
          end                   
          tmr.wdclr()
          return temp
      else
        return false
      end
    else
      return false
    end
  end
end

tmr.stop(4)
tmr.alarm( 4 , 15000, 1 , genData )  