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
    _ "github.com/lib/pq"
	"github.com/op/go-logging"
    "gopkg.in/yaml.v2"
    "io/ioutil"
    "os"
    "os/exec"
)

type Conf struct {
	ConnStr string `yaml:"connstr"`
	PgData string `yaml:"pgdata"`
	KillQuerySQL string `yaml:"killquery"`
	PgRestart string `yaml:"pgrestart"`
	PgbRestart string `yaml:"pgbrestart"`
}
var configFile string
var c Conf
var launchChecker bool

type OpsMessage struct {
    OpsType       string
	metric		  float64
}

// XXX maybe need singleton
var db *sql.DB
var log = logging.MustGetLogger("patrol")

func initFlag() {
	flag.StringVar(&configFile, "f", "./patrol.yml", "configuration file path")
	flag.BoolVar(&launchChecker, "c", false, "launch checker for failover")
	flag.Parse()
}

func initlog() {
	var format = logging.MustStringFormatter(
		`%{color}%{time:15:04:05.000}|%{level:.4s}%{id:03x}%{color:reset}» %{shortfile} %{message}`,
	)
	backend1 := logging.NewLogBackend(os.Stderr, "", 0)
	backend2 := logging.NewLogBackend(os.Stderr, "", 0)

	backend2Formatter := logging.NewBackendFormatter(backend2, format)

	backend1Leveled := logging.AddModuleLevel(backend1)
	backend1Leveled.SetLevel(logging.ERROR, "")

	logging.SetBackend(backend1Leveled, backend2Formatter)
}

func loadconf() {
	yamlFile, err := ioutil.ReadFile(configFile)
	if err != nil {
		log.Fatalf("yamlFile.Get err #%v", err)
	}
	err = yaml.Unmarshal(yamlFile, &c)
	if err != nil {
		log.Fatalf("Unmarshal: %v", err)
	}
}

func main() {

	initlog()
	initFlag()
	loadconf()

	db, err := sql.Open("postgres", c.ConnStr)
	if err != nil {
		  log.Error(err)
	}
	err = db.Ping()
	if err != nil {
		  log.Error(err)
	}
	defer db.Close()


	if launchChecker {
		go HttpServer()

		ops_ch := make(chan OpsMessage, 100)
		go CpuChecker(ops_ch)
		go ActivityChecker(ops_ch, db)
		go CheckPGalive(ops_ch)
		go CheckPGBalive(ops_ch)

		for  {
			select {
			case msg := <-ops_ch:
				switch msg.OpsType{
				case "killquery":
					res, err := db.Exec(c.KillQuerySQL)
					if err == nil {
						rows,_ := res.RowsAffected()
						log.Infof("metric: %f, kill %d querys\n",msg.metric, rows)
					} else {
						log.Error(err)
					}
					break;
				case "pg_restart":
					command := exec.Command("/bin/sh", "-c", c.PgRestart)
					err := command.Run()
					if nil != err {
						log.Warningf("PostgerSQL restart error: %s", err);
					}
					log.Infof("PostgerSQL restart: %v", command.Args);
					break;
				case "pgb_restart":
					command := exec.Command("/bin/sh", "-c", c.PgbRestart)
					err := command.Run()
					if nil != err {
						log.Warningf("Pgbouncer restart error: %v >> %s", command.Args, err);
					}
					log.Infof("Pgbouncer restart: %v", command.Args);
					break;
				}
			}
		}
	} else {
		HttpServer()
	}

}
