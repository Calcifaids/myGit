--Set nodeMCU as remote client and join network REMEMBER TO SET BEFORE USING
ssid = "xxxx"
pass = "xxxx"
station_cfg={}
station_cfg.ssid=ssid
station_cfg.pwd=pass
wifi.sta.config(station_cfg)

-- SETUP I2C interface
id  = 0
sda = 1
scl = 2

-- initialize i2c, set pin1 as sda, set pin2 as scl
i2c.setup(id, sda, scl, i2c.SLOW)

--Write to arudino addres 0x8
address = 0x8
function read_reg(devAddress, stringVar)
    i2c.start(id)
    i2c.address(id, devAddress, i2c.TRANSMITTER)
    i2c.write(id,stringVar)
    i2c.stop(id)
end

--Create web server and listen for incoming connections
srv=net.createServer(net.TCP)
srv:listen(80,function(conn)
  conn:on("receive",function(conn,payload)
    --Print for debugging
    print(payload)
    function doStuff()
      --Remove data name to leave just the command number
      value =string.sub(buttonString[1],8)
      --More debugging
      print(buttonString[1])
      print (value)
      --Send operation via I2C
      read_reg(address, value) 
    end
    --Match for payload and send if present
    buttonString={string.match(payload,"button=%d+")}
    if (buttonString[1]~=nil)then doStuff()end

    --create buffer
    buffer = "<!DOCTYPE html><html><head><style>table{table-layout: fixed;width: 300px;}td{border: solid green;overflow: hidden;width: 100px;}button{width: 100px;}</style></head><body>"
    buffer = buffer .. "<form action=\"/\" method=\"post\">"     
    buffer = buffer .. "<table><tbody><tr>"
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"0\" type=\"submit\">Power</td>"
    buffer = buffer .. "<td></td>"
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"1\" type=\"submit\">Mute</td>"
    buffer = buffer .. "</tr><tr>"
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"2\" type=\"submit\">Menu</td>"
    buffer = buffer .. "<td></td>"
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"3\" type=\"submit\">Source</td>"
    buffer = buffer .. "</tr><tr>"
    buffer = buffer .. "<td></td>"     
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"5\" type=\"submit\">UP</td>"
    buffer = buffer .. "<td></td>"
    buffer = buffer .. "</tr><tr>"
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"6\" type=\"submit\">LEFT</td>"
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"4\" type=\"submit\">OK</td>"
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"7\" type=\"submit\">RIGHT</td>"
    buffer = buffer .. "</tr><tr>"
    buffer = buffer .. "<td></td>"
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"8\" type=\"submit\">DOWN</td>"
    buffer = buffer .. "<td></td>" 
    buffer = buffer .. "</tr><tr>" 
    buffer = buffer .. "<td></td>"
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"9\" type=\"submit\">Exit</td>"
    buffer = buffer .. "<td></td>"
    buffer = buffer .. "</tr><tr>" 
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"10\" type=\"submit\">Vol +</td>"
    buffer = buffer .. "<td></td>" 
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"11\" type=\"submit\">Ch +</td>"
    buffer = buffer .. "</tr><tr>" 
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"12\" type=\"submit\">Vol -</td>"
    buffer = buffer .. "<td></td>" 
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"13\" type=\"submit\">Ch -</td>"     
    buffer = buffer .. "</tr><tr>"  
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"14\" type=\"submit\">1</td>"  
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"15\" type=\"submit\">2</td>"         
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"16\" type=\"submit\">3</td>"   
    buffer = buffer .. "</tr><tr>"  
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"17\" type=\"submit\">4</td>"  
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"18\" type=\"submit\">5</td>"         
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"19\" type=\"submit\">6</td>" 
    buffer = buffer .. "</tr><tr>"  
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"20\" type=\"submit\">7</td>"  
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"21\" type=\"submit\">8</td>"         
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"22\" type=\"submit\">9</td>" 
    buffer = buffer .. "</tr><tr>" 
    buffer = buffer .. "<td></td>" 
    buffer = buffer .. "<td><button type=\"submit\" name=\"button\" value=\"23\" type=\"submit\">0</td>" 
    buffer = buffer .. "<td></td>" 
    buffer = buffer .. "</tr></tbody></table>"
    buffer = buffer .. "</body></html>"
    
    conn:send(buffer)
    collectgarbage()
  end)
  conn:on("sent",function(conn) conn:close() end)
end)

