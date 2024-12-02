package main

import (
	"database/sql"
	"fmt"
	"net/http"
	"os"
	"time"
	cardDelivery "transaction-service/internal/rfidcard/delivery"
	cardRepository "transaction-service/internal/rfidcard/repository/mysql"
	cardUsecase "transaction-service/internal/rfidcard/usecase"
	"transaction-service/pkg/middleware"
	notFoundDelivery "transaction-service/pkg/notfound/delivery"
	"transaction-service/tools"

	_ "github.com/go-sql-driver/mysql"
	"github.com/gorilla/mux"
	"github.com/joho/godotenv"
	"gopkg.in/yaml.v2"
)

const configPath = "config.yaml"

type Config struct {
	Host string `yaml:"HOST"`
	Port int    `yaml:"PORT"`
}

var AppConfig *Config

func main() {
	tools.InitLogger()

	err := godotenv.Load()
	if err != nil {
		tools.Logger.Fatal("error loading .env file:", err)
	}

	configFile, err := os.Open(configPath)
	if err != nil {
		tools.Logger.Fatal("error opening config file:", err)
	}
	defer configFile.Close()

	decoder := yaml.NewDecoder(configFile)
	err = decoder.Decode(&AppConfig)
	if err != nil {
		tools.Logger.Fatal("error reading config file:", err)
	}

	mysqlDSN := fmt.Sprintf("%s:%s@tcp(%s:%s)/%s?",
		os.Getenv("MYSQL_USER"),
		os.Getenv("MYSQL_PASSWORD"),
		os.Getenv("MYSQL_HOST"),
		os.Getenv("MYSQL_PORT"),
		os.Getenv("MYSQL_DATABASE"),
	)
	mysqlDSN += "&charset=utf8"
	mysqlDSN += "&interpolateParams=true"

	mysqlConnect, err := sql.Open("mysql", mysqlDSN)
	if err != nil {
		panic(err)
	}

	mysqlConnect.SetConnMaxLifetime(time.Minute * 3)
	mysqlConnect.SetMaxOpenConns(10)
	mysqlConnect.SetMaxIdleConns(10)

	defer func() {
		if err = mysqlConnect.Close(); err != nil {
			panic(err)
		}
	}()

	router := mux.NewRouter()

	cardRepo := cardRepository.NewRfidCardMySqlRepo(mysqlConnect)
	cardUC := cardUsecase.NewRfidCardUsecase(cardRepo)
	cardHandler := cardDelivery.RfidCardHandler{
		RfidCardUC: cardUC,
	}

	router.NotFoundHandler = http.HandlerFunc(notFoundDelivery.NotFoundHandler)

	router.Handle(
		"/transaction-api/card/{card_id}",
		middleware.LogRequest(
			http.HandlerFunc(cardHandler.GetCard),
		),
	).Methods("GET")

	router.Handle(
		"/transaction-api/card/{card_id}",
		middleware.LogRequest(
			http.HandlerFunc(cardHandler.HandleCardOperaiton),
		),
	).Methods("POST")

	tools.Logger.Printf("starting server at http://%s:%d", AppConfig.Host, AppConfig.Port)
	tools.Logger.Fatal(http.ListenAndServe(fmt.Sprintf("%s:%d", AppConfig.Host, AppConfig.Port), router))
}
