package delivery

import (
	"net/http"
	"transaction-service/tools"
)

func NotFoundHandler(w http.ResponseWriter, r *http.Request) {
	clientIP := r.RemoteAddr
	userAgent := r.UserAgent()
	requestedURL := r.URL.String()

	tools.Logger.Warnf("404 Not Found - Client IP: %s, User-Agent: %s, Requested URL: %s\n", clientIP, userAgent, requestedURL)

	w.WriteHeader(http.StatusNotFound)
	_, _ = w.Write([]byte("404 Not Found"))
}
