package delivery

import (
	"encoding/json"
	"net/http"
	"transaction-service/internal/models"
	"transaction-service/internal/rfidcard/usecase"
	"transaction-service/tools"

	"github.com/gorilla/mux"
)

type OperationType int

const (
	WriteOff OperationType = iota
	Refill
)

func (op OperationType) String() string {
	return [...]string{"WriteOff", "Refill"}[op]
}

type RfidCardHandler struct {
	RfidCardUC *usecase.RfidCardUsecase
}

func (h *RfidCardHandler) GetCard(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	cardID := vars["card_id"]

	card, err := h.RfidCardUC.GetCard(cardID)
	if err != nil {
		tools.JSONError(w, http.StatusInternalServerError, err.Error(), "RfidCardUsecase.GetCard")
		return
	}

	jsonCard, err := json.Marshal(card)
	if err != nil {
		tools.JSONError(w, http.StatusInternalServerError, err.Error(), "RfidCardHandler.GetCard")
		return
	}

	_, err = w.Write(jsonCard)
	if err != nil {
		tools.JSONError(w, http.StatusInternalServerError, err.Error(), "RfidCardHandler.GetCard")
		return
	}
}

type CardOperationRequest struct {
	OperationType OperationType `json:"operation_type"`
}

func (h *RfidCardHandler) HandleCardOperaiton(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	cardID := vars["card_id"]

	var request CardOperationRequest
	decoder := json.NewDecoder(r.Body)
	if err := decoder.Decode(&request); err != nil {
		tools.JSONError(w, http.StatusBadRequest, "Unable to parse request body", "RfidCardHandler.HandleCardOperaiton")
		return
	}

	operationHandlers := map[OperationType]func(cardID string) (*models.RfidCard, error){
		WriteOff: h.RfidCardUC.WriteOff,
		Refill:   h.RfidCardUC.Refill,
	}

	operationHandler, exists := operationHandlers[request.OperationType]
	if !exists {
		tools.JSONError(w, http.StatusBadRequest, "Invalid operation type", "RfidCardHandler.HandleCardOperaiton")
		return
	}

	updatedCard, err := operationHandler(cardID)
	if err != nil {
		tools.JSONError(w, http.StatusBadRequest, err.Error(), "RfidCardUsecase.HandleCardOperaiton")
		return
	}

	jsonCard, err := json.Marshal(updatedCard)
	if err != nil {
		tools.JSONError(w, http.StatusInternalServerError, err.Error(), "RfidCardHandler.HandleCardOperaiton")
		return
	}

	_, err = w.Write(jsonCard)
	if err != nil {
		tools.JSONError(w, http.StatusInternalServerError, err.Error(), "RfidCardHandler.HandleCardOperaiton")
		return
	}
}
