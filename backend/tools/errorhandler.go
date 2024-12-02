package tools

import (
	"encoding/json"
	"net/http"

	"github.com/sirupsen/logrus"
)

func JSONError(w http.ResponseWriter, status int, msg string, method string) {
	defer func() {
		if r := recover(); r != nil {
			Logger.WithFields(logrus.Fields{
				"method": method,
				"status": status,
				"panic":  r,
			}).Error("panic occurred")
		}
	}()

	Logger.WithFields(logrus.Fields{
		"method": method,
		"status": status,
	}).Error(msg)

	w.WriteHeader(status)

	resp, err := json.Marshal(map[string]interface{}{
		"status": status,
		"error":  msg,
	})
	if err != nil {
		return
	}

	_, err = w.Write(resp)
	if err != nil {
		return
	}
}
