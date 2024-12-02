package middleware

import (
	"net/http"
	"transaction-service/tools"
)

func LogRequest(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		clientIP := r.RemoteAddr
		userAgent := r.UserAgent()
		requestedURL := r.URL.String()

		tools.Logger.Printf("Client IP: %s, User-Agent: %s, Requested URL: %s\n", clientIP, userAgent, requestedURL)

		next.ServeHTTP(w, r)
	})
}
