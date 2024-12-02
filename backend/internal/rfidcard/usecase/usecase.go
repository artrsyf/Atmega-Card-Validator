package usecase

import (
	"transaction-service/internal/models"
	"transaction-service/internal/rfidcard/repository"
)

const (
	TravelCost    = 50
	RefillBalance = 400
)

type RfidCardUsecaseI interface {
	GetCard(cardID string) (*models.RfidCard, error)
	WriteOff(cardID string) (*models.RfidCard, error)
	Refill(cardID string) (*models.RfidCard, error)
}

type RfidCardUsecase struct {
	CardRepo repository.RfidCardRepo
}

func NewRfidCardUsecase(repo repository.RfidCardRepo) *RfidCardUsecase {
	return &RfidCardUsecase{
		CardRepo: repo,
	}
}

func (uc *RfidCardUsecase) GetCard(cardID string) (*models.RfidCard, error) {
	card, err := uc.CardRepo.GetCardByID(cardID)
	if err != nil {
		return nil, err
	}

	return card, err
}

func (uc *RfidCardUsecase) WriteOff(cardID string) (*models.RfidCard, error) {
	card, err := uc.CardRepo.GetCardByID(cardID)
	if err != nil {
		return nil, err
	}

	if card.Balance < TravelCost {
		return nil, models.ErrNotEnoughBalance
	}

	updatedCard, err := uc.CardRepo.Update(card.CardId, card.Balance-TravelCost)
	if err != nil {
		return nil, err
	}

	return updatedCard, nil
}

func (uc *RfidCardUsecase) Refill(cardID string) (*models.RfidCard, error) {
	card, err := uc.CardRepo.GetCardByID(cardID)
	if err == models.ErrNoCard {
		newCard, err := uc.CardRepo.Create(cardID, RefillBalance)
		if err != nil {
			return nil, err
		}

		return newCard, nil
	} else if err != nil {
		return nil, err
	}

	updatedCard, err := uc.CardRepo.Update(cardID, card.Balance+RefillBalance)
	if err != nil {
		return nil, err
	}

	return updatedCard, nil
}
