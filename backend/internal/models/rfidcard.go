package models

type RfidCard struct {
	CardId  string  `json:"card_id"`
	Balance float64 `json:"balance"`
}
