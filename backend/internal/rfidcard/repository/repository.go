package repository

import "transaction-service/internal/models"

//go:generate mockgen -source=repository.go -destination=mock_repository/rfidcard_mock.go -package=mock_repository MockRfidCardRepository
type RfidCardRepo interface {
	GetCardByID(cardID string) (*models.RfidCard, error)
	Create(cardID string, initialBalance float64) (*models.RfidCard, error)
	Update(cardID string, newBalance float64) (*models.RfidCard, error)
}
