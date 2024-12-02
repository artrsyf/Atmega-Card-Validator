dofile("credentials.lua")

function startup()
    if file.open("application.lua") == nil then
        print("application.lua not found. Stopping...")
    else
        print("Starting application...")
        file.close("application.lua")
        dofile("application.lua")
    end
end

wifi_connect_event = function(T)
    print("Connected to AP: "..T.SSID)
    print("Waiting for IP address...")
end

wifi_got_ip_event = function(T)
    print("WiFi ready! IP address: "..T.IP)
    print("Starting in 3 seconds...")
    tmr.create():alarm(3000, tmr.ALARM_SINGLE, startup)
end

wifi_disconnect_event = function(T)
    print("WiFi disconnected! Reason: "..T.reason)
end

wifi.eventmon.register(wifi.eventmon.STA_CONNECTED, wifi_connect_event)
wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, wifi_got_ip_event)
wifi.eventmon.register(wifi.eventmon.STA_DISCONNECTED, wifi_disconnect_event)

wifi.setmode(wifi.STATION)
wifi.sta.config({ssid=SSID, pwd=PASSWORD})
print("Connecting to WiFi...")