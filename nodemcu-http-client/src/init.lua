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
        local cleaned_data  = string.gsub(data, "\n", "")

        local parts = {}
        for part in string.gmatch(cleaned_data, "([^#]+)") do
            table.insert(parts, part)
        end
        
        handleRfidCardInfo(card_id, function(response, error)
            if error then
                uart_send_string("Error fetching data\n")
            else
                uart_send_string("Data: " .. response .. "\n")
            end
        end)
end, 0)

uart.on("data", "\n", function(data)
    -- Убираем символ новой строки, если он присутствует
    local cleaned_data = string.gsub(data, "\n", "")
    
    -- Разделяем строку по символу #
    local parts = {}
    for part in string.gmatch(cleaned_data, "([^#]+)") do
        table.insert(parts, part)
    end
    
    -- Обработка в зависимости от типа запроса
    if parts[1] == "POST" then
        -- Если запрос POST: ожидаем номер операции и id карты
        if #parts == 3 then
            local operation = tonumber(parts[2])  -- Номер операции
            local card_id = parts[3]               -- ID карты
            if operation == 0 or operation == 1 then
                handleRfidCardOperation(card_id, operation, function(response, error)
                    if error then
                        uart_send_string("Error fetching data\n")
                    else
                        uart_send_string("Data: " .. response .. "\n")
                    end
                end)
            else
                uart_send_string("Invalid operation\n")
            end
        else
            uart_send_string("Invalid POST format\n")
        end

    elseif parts[1] == "GET" then
        -- Если запрос GET: ожидаем только id карты
        if #parts == 2 then
            local card_id = parts[2]  -- ID карты
            handleRfidCardInfo(card_id, function(response, error)
                if error then
                    uart_send_string("Error fetching data\n")
                else
                    uart_send_string("Data: " .. response .. "\n")
                end
            end)
        else
            uart_send_string("Invalid GET format\n")
        end

    else
        uart_send_string("Invalid request type: ", cleaned_data .. "\n")
    end
end, 0)