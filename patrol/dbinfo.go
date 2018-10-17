package main

import (
	"database/sql"
	"encoding/json"
	"fmt"
	"net/http"
	"os"
	_ "strconv"
	"strings"
	"syscall"
	"time"
)

type DbInfo struct {
	Maxage uint64
	DiskUsage float32
}

func HandleError(err error) {
	if err !=nil {
		log.Error(err)
	}
}

func diskUsage(volumePath string) float32 {

	var stat syscall.Statfs_t
	syscall.Statfs(volumePath, &stat)

	msize := stat.Blocks * uint64(stat.Bsize)
	// available = free - reserved filesystem blocks(for root)
	mavail := stat.Bavail* uint64(stat.Bsize)

	return float32(msize - mavail) / float32(msize)
}

func getDbAge(mdb *sql.DB) uint64 {
	msql := `SELECT age(datfrozenxid)
				FROM pg_database
				WHERE datname <> 'template1'
				 	AND datname <> 'template0'
				 	AND datname <> 'postgres'
				  	AND datname <> 'monitordb'
				ORDER BY age(datfrozenxid) DESC LIMIT 1;`

	var age uint64
	err := mdb.QueryRow(msql).Scan(&age)
	if err != nil && err != sql.ErrNoRows {
		log.Error(err)
	}

	return age
}

func getDbDiskUsage(mdb *sql.DB) float32 {
	msql := `show data_directory;`

	var dataDir string
	err := mdb.QueryRow(msql).Scan(&dataDir)
	if err != nil && err != sql.ErrNoRows {
		log.Error(err)
	}

	return diskUsage(dataDir)
}

func getDbInfo(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")

	mdb, err := sql.Open("postgres", c.ConnStr)
	HandleError(err)
	err = mdb.Ping()
	HandleError(err)
	defer mdb.Close()

	msql := `SELECT datname
	FROM pg_database
	WHERE datname!='postgres'
		AND datname != 'template1'
	    AND datname != 'template0';`
	var dbname string
	err = mdb.QueryRow(msql).Scan(&dbname)
	if err != nil && err != sql.ErrNoRows {
		log.Error(err)
	}

	newconn := strings.Replace(c.ConnStr, "dbname=postgres", "dbname="+dbname,-1);
	log.Info(newconn)
	ndb, err := sql.Open("postgres", newconn)
	HandleError(err)
	err = ndb.Ping()
	HandleError(err)
	defer ndb.Close()

	ma := getDbAge(ndb)
	du := getDbDiskUsage(ndb)
	log.Infof("%d:%f", ma,du)
	dbinfo := &DbInfo{
		Maxage: ma,
		DiskUsage : du,
	}

	json, err := json.Marshal(dbinfo)
	HandleError(err)
	w.Write(json)
}

func HttpServer() {
	server := http.Server{
		Addr: "0.0.0.0:9999",
	}
	http.HandleFunc("/dbinfo", getDbInfo)

	log.Info("patrol service on (9999)...")
	server.ListenAndServe()
}


func MetricCollector() {
//    replication_delay_sql := `select client_addr,
//                                    CASE
//                                    WHEN pg_is_in_recovery()
//                                        THEN pg_wal_lsn_diff(pg_last_wal_replay_lsn(), replay_lsn)
//                                    ELSE
//                                        pg_wal_lsn_diff(pg_current_wal_lsn(), replay_lsn)
//                                    END AS total_lag
//                              from monitor.v_repl_stats;`
	replication_delay_sql := `select slave_ip as client_addr, total_lag from testt;`


	HOSTNAME,_ := os.Hostname()

	mdb, err := sql.Open("postgres", c.ConnStr)
	HandleError(err)
	err = mdb.Ping()
	HandleError(err)

	msql := `SELECT datname
	FROM pg_database
	WHERE datname!='postgres'
		AND datname != 'template1'
	    AND datname != 'template0';`
	var dbname string
	err = mdb.QueryRow(msql).Scan(&dbname)
	if err != nil && err != sql.ErrNoRows {
		log.Error(err)
	}
	mdb.Close()

	for  {
		time.Sleep(1 * time.Second)

		newconn := strings.Replace(c.ConnStr, "dbname=postgres", "dbname="+dbname,-1);
		log.Info(newconn)
		ndb, err := sql.Open("postgres", newconn)
		HandleError(err)
		err = ndb.Ping()
		HandleError(err)
		defer ndb.Close()

		rows, err := mdb.Query(replication_delay_sql)
		if err != nil {
		    log.Error(err)
		}
		defer rows.Close()
		for rows.Next() {
		    var slaveIp string
			var replay_lag int64
			rows.Scan(&slaveIp, &replay_lag)
			fmt.Println(slaveIp, replay_lag)

			reportServers := [2]string{"10.191.160.46","10.191.160.54"}
			for _, rs := range reportServers {
				url := fmt.Sprintf("http://%s:9091/metrics/job/pushgateway/instance/%s/datname/%s/slave/%s/role/%s",
					rs, HOSTNAME, dbname, slaveIp, c.PromRole)
				log.Info(url);
			}
		}
	}
}
