USE transaction_service_mysql;

CREATE TABLE IF NOT EXISTS rfid_card (
    card_id VARCHAR(255) PRIMARY KEY,
    balance DECIMAL(10, 2) NOT NULL UNIQUE
);