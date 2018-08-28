package main

import (
	"database/sql"
	"encoding/json"
	"net/http"
	_ "os"
	_ "strconv"
	"strings"
	"syscall"
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
	mfree := stat.Bfree * uint64(stat.Bsize)

	return float32(msize - mfree) / float32(msize)
}

func getDbAge(mdb *sql.DB) uint64 {
	msql := `SELECT age(relfrozenxid)
	FROM pg_authid t1
	JOIN pg_class t2 ON t1.oid=t2.relowner
	JOIN pg_namespace t3 ON t2.relnamespace=t3.oid
	WHERE t2.relkind IN ($$t$$,$$r$$)
	ORDER BY age(relfrozenxid) DESC LIMIT 1;`

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

