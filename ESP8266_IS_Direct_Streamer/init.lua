-- init script for esp2866/nodemcu
SSID="WIFI_SSID" APPWD="YOUR_WIFI_PASS" CMDFILE="stream.lua"
wT=0
NWT=200

function launch()
  print("UP")
  a=node.heap()
  print(a)
--  dofile(CMDFILE)
end

function checkWIFI() 
  if ( wT > NWT ) then
    print("NC")  
  else ipAddr = wifi.sta.getip()
    if ( ( ipAddr ~= nil ) and  ( ipAddr ~= "0.0.0.0" ) )then
      tmr.alarm( 1 , 500 , 0 , launch )
    else
      tmr.alarm( 0 , 2500 , 0 , checkWIFI)
      print(wT)
      wT = wT + 1    end   end   
end

ipAddr = wifi.sta.getip()
if ( ( ipAddr == nil ) or  ( ipAddr == "0.0.0.0" ) ) then
  print("TX...")
  wifi.setmode( wifi.STATION )
  wifi.sta.config( SSID , APPWD)
  print("waiting")
  tmr.alarm( 2 , 2500 , 0 , checkWIFI )  -- Call checkWIFI 2.5S in the future.
else launch()
end
