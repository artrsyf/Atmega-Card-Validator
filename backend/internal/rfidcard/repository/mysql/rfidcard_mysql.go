package mysql

import (
	"database/sql"
	"transaction-service/internal/models"
)

type RfidCardMySqlRepository struct {
	DB *sql.DB
}

func NewRfidCardMySqlRepo(db *sql.DB) *RfidCardMySqlRepository {
	return &RfidCardMySqlRepository{
		DB: db,
	}
}

func (repo *RfidCardMySqlRepository) GetCardByID(cardID string) (*models.RfidCard, error) {
	card := &models.RfidCard{}

	err := repo.DB.
		QueryRow("SELECT card_id, balance FROM rfid_card WHERE card_id = ?", cardID).
		Scan(&card.CardId, &card.Balance)

	if err == sql.ErrNoRows {
		return nil, models.ErrNoCard
	} else if err != nil {
		return nil, err
	}

	return card, nil
}

func (repo *RfidCardMySqlRepository) Create(cardID string, initialBalance float64) (*models.RfidCard, error) {
	err := repo.DB.
		QueryRow("SELECT 1 FROM rfid_card WHERE card_id = ?", cardID).Scan(new(int))

	if err == nil {
		return nil, models.ErrCardRecordAlreadyCreated
	}

	if err != sql.ErrNoRows {
		return nil, err
	}

	repo.DB.Exec(
		"INSERT INTO rfid_card (`card_id`, `balance`) VALUES (?, ?)",
		cardID,
		initialBalance,
	)

	card := &models.RfidCard{}
	err = repo.DB.
		QueryRow("SELECT card_id, balance FROM rfid_card WHERE card_id = ?", cardID).
		Scan(&card.CardId, &card.Balance)
	if err != nil {
		return nil, err
	}

	return card, nil
}

func (repo *RfidCardMySqlRepository) Update(cardID string, newBalance float64) (*models.RfidCard, error) {
	_, err := repo.DB.Exec(
		"UPDATE rfid_card SET balance = ? WHERE card_id = ?",
		newBalance,
		cardID,
	)

	if err != nil {
		return nil, err
	}

	card, err := repo.GetCardByID(cardID)
	if err != nil {
		return nil, err
	}

	return card, nil
}
