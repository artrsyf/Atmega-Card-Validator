package models

import "errors"

var (
	ErrNoCard                   = errors.New("cant find such RFID card")
	ErrCardRecordAlreadyCreated = errors.New("card with such RFID is already created")

	ErrNotEnoughBalance = errors.New("not enough balance to write off")
)
