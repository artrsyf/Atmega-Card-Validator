FROM golang:alpine AS builder
LABEL stage=gobuilder

ENV CGO_ENABLED 0
ENV GOOS linux

RUN apk update --no-cache && apk add --no-cache tzdata

WORKDIR /build

COPY ./go.mod .
COPY ./go.sum .

RUN go mod download

COPY . .

RUN go build -ldflags="-s -w" -o /app/transaction-service ./cmd/transaction-service/

FROM alpine

RUN apk update --no-cache

WORKDIR /app/transaction-service/bin/transaction-service/

COPY --from=builder /build/cmd/transaction-service/.env /app/transaction-service/bin/transaction-service/.env
COPY --from=builder /build/cmd/transaction-service/config.yaml /app/transaction-service/bin/transaction-service/config.yaml
COPY --from=builder /app/transaction-service /app/transaction-service/bin/transaction-service/transaction-service

EXPOSE 8080

CMD ["./transaction-service"]