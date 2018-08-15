/* =============================================================================
#     FileName: patrol.go
#         Desc:
#       Author: LiuYangming
#        Email: sdwhlym@126.com
#     HomePage: http://yummyliu.github.io
#      Version: 0.0.1
#   LastChange: 2018-08-10 11:21:02
#      History:
============================================================================= */
package main

import (
    "database/sql"
    "flag"
    "fmt"
    _ "github.com/lib/pq"
	"github.com/op/go-logging"
    "net/http"
    "os"
)

var host     = "localhost"
var	port     = 5432
var	user     = "postgres"
var	password = "123"
var	dbname   = "postgres"
var connStr  = ""
var cancelSQL = "select pg_terminate_backend(pid) from pg_stat_activity where pid <> pg_backend_pid() and usename != 'dba';"
var db *sql.DB
var log = logging.MustGetLogger("patrol")

type OpsMessage struct {
    OpsType       string
	metric		  float64
}

func initFlag() {

	flag.StringVar(&host, "dbhost", "localhost", "db host")
	flag.IntVar(&port, "dbport", 5432, "db port")
	flag.StringVar(&user, "dbuser", "dba", "db user")
	flag.StringVar(&dbname,"dbname", "postgres", "monitor db")
	// XXX change to no password in command line
	flag.StringVar(&password,"passwd", "123", "password of user")

	flag.Parse()
}
func initlog() {
	var format = logging.MustStringFormatter(
		`%{color}%{time:15:04:05.000} » %{level:.4s} %{id:03x}%{color:reset} %{message}`,
	)
	// For demo purposes, create two backend for os.Stderr.
	backend1 := logging.NewLogBackend(os.Stderr, "", 0)
	backend2 := logging.NewLogBackend(os.Stderr, "", 0)

	// For messages written to backend2 we want to add some additional
	// information to the output, including the used log level and the name of
	// the function.
	backend2Formatter := logging.NewBackendFormatter(backend2, format)

	// Only errors and more severe messages should be sent to backend1
	backend1Leveled := logging.AddModuleLevel(backend1)
	backend1Leveled.SetLevel(logging.ERROR, "")

	// Set the backends to be used.
	logging.SetBackend(backend1Leveled, backend2Formatter)
}
func httpServer() {

	server := http.Server{
		Addr: "127.0.0.1:8888",
	}
	http.HandleFunc("/dbinfo", getDbInfo)

	log.Info("http service on (8888)...")
	server.ListenAndServe()
}

func connectPG() *sql.DB {
	connStr = fmt.Sprintf("host=%s port=%v user=%s password=%s dbname=%s sslmode=disable",
							host,
							port,
							user,
							password,
							dbname)
	log.Info(connStr)
	db, err := sql.Open("postgres", connStr)
	if err != nil {
		  panic(err)
	}
	err = db.Ping()
	if err != nil {
		  panic(err)
	}

	return db
}

func main() {

	initFlag()
	initlog()
    ops_ch := make(chan OpsMessage, 100)

	db = connectPG()
	err := db.Ping()
	if err != nil {
		  panic(err)
	}
	go CpuChecker(ops_ch)
	go ActivityChecker(ops_ch)

	// start http server
	go httpServer()

	for  {
	// waiting for ops message
		select {
		case msg := <-ops_ch:
			if msg.OpsType == "killquery" {
				res, err := db.Exec(cancelSQL)

				if err == nil {
					rows,_ := res.RowsAffected()
					log.Infof("metric: %f, kill %d querys\n",msg.metric, rows)
				}
			}
		}
	}

	defer db.Close()
}
