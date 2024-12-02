dofile("credentials.lua")

function startup()
    if file.open("application.lua") == nil then
    else
        file.close("application.lua")
        dofile("application.lua")
    end
end

wifi_connect_event = function(T)
end

wifi_got_ip_event = function(T)
    tmr.create():alarm(3000, tmr.ALARM_SINGLE, startup)
end

wifi_disconnect_event = function(T)
end

wifi.eventmon.register(wifi.eventmon.STA_CONNECTED, wifi_connect_event)
wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, wifi_got_ip_event)
wifi.eventmon.register(wifi.eventmon.STA_DISCONNECTED, wifi_disconnect_event)

wifi.setmode(wifi.STATION)
wifi.sta.config({ssid=SSID, pwd=PASSWORD})

uart.setup(0, 4800, 8, uart.PARITY_NONE, uart.STOPBITS_1, 0)

require("handlers")

function uart_send_string(payload)
    for c in string.gmatch(payload, ".") do
        uart.write(0, c)
        tmr.delay(100000) -- Задержка в микросекундах
    end
end

uart.on("data", "\n",
    function(data)
        local card_id = string.gsub(data, "\n", "")
        handleRfidCardInfo(card_id, function(response, error)
            if error then
                uart_send_string("Error fetching data\n")
            else
                uart_send_string("Data: " .. response .. "\n")
            end
        end)
end, 0)