require("config")

function handleRfidCardInfo(card_id, callback)
    local http = require("http")

    local url = CONFIG.PROTOCOL .. CONFIG.HOST .. "/transaction-api/card/" .. card_id

    http.get(url, nil, function(code, data)
        if (code < 0) then
            callback(nil, "HTTP GET request failed")
        else
            callback(data, nil)
        end
    end)
end

OPERATION_TYPE = {
    WRITE_OFF = 0,
    REFILL = 1
}

function handleRfidCardOperation(card_id, operation_type)
    local http = require("http")

    local url = CONFIG.PROTOCOL .. CONFIG.HOST .. "/transaction-api/card/" .. card_id
    local headers = {
        ["Content-Type"] = "application/json"
    }
    local body = '{"operation_type": ' .. operation_type .. '}'

    http.post(url, headers, body, function(code, data)
        if (code < 0) then
--            print("HTTP POST request failed")
        else
--            print("HTTP POST response code: ", code)
--            print("HTTP POST response data: ", data)
        end
    end)
end
